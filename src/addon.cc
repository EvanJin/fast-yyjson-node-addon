#include <napi.h>
#include "yyjson.h"
#include <limits>
#include <string>

// 将 yyjson_val 转换为 Napi::Value 的辅助函数
Napi::Value YyjsonToNapi(const Napi::Env &env, yyjson_val *val)
{
    if (yyjson_is_null(val))
    {
        return env.Null();
    }
    else if (yyjson_is_bool(val))
    {
        return Napi::Boolean::New(env, yyjson_get_bool(val));
    }
    else if (yyjson_is_str(val))
    {
        return Napi::String::New(env, yyjson_get_str(val));
    }
    else if (yyjson_is_int(val))
    {
        int64_t intVal = yyjson_get_sint(val);
        // 检查是否超出 JavaScript 的安全整数范围
        if (intVal > static_cast<int32_t>(std::numeric_limits<int32_t>::max()) ||
            intVal < static_cast<int32_t>(std::numeric_limits<int32_t>::min()))
        {
            return Napi::String::New(env, std::to_string(intVal));
        }
        return Napi::Number::New(env, static_cast<double>(intVal));
    }
    else if (yyjson_is_uint(val))
    {
        uint64_t uintVal = yyjson_get_uint(val);
        // 检查是否超出 JavaScript 的安全整数范围
        if (uintVal > static_cast<uint64_t>(std::numeric_limits<double>::max()))
        {
            return Napi::String::New(env, std::to_string(uintVal));
        }
        return Napi::Number::New(env, static_cast<double>(uintVal));
    }
    else if (yyjson_is_real(val))
    {
        return Napi::Number::New(env, yyjson_get_real(val));
    }
    else if (yyjson_is_arr(val))
    {
        Napi::Array arr = Napi::Array::New(env);
        size_t idx, max;
        yyjson_val *elem;
        yyjson_arr_foreach(val, idx, max, elem)
        {
            arr[idx] = YyjsonToNapi(env, elem);
        }
        return arr;
    }
    else if (yyjson_is_obj(val))
    {
        Napi::Object obj = Napi::Object::New(env);
        size_t idx, max;
        yyjson_val *key;
        yyjson_val *value;
        yyjson_obj_foreach(val, idx, max, key, value)
        {
            std::string keyStr(yyjson_get_str(key));
            obj.Set(keyStr, YyjsonToNapi(env, value));
        }
        return obj;
    }
    return env.Null();
}

// 将 Napi::Value 转换为 yyjson_val 的辅助函数
yyjson_mut_val *NapiToYyjson(Napi::Env env, yyjson_mut_doc *doc, Napi::Value value)
{
    if (value.IsNull() || value.IsUndefined())
    {
        return yyjson_mut_null(doc);
    }
    else if (value.IsBoolean())
    {
        return yyjson_mut_bool(doc, value.As<Napi::Boolean>().Value());
    }
    else if (value.IsString())
    {
        std::string str = value.As<Napi::String>().Utf8Value();
        return yyjson_mut_str(doc, str.c_str());
    }
    else if (value.IsNumber())
    {
        return yyjson_mut_real(doc, value.As<Napi::Number>().DoubleValue());
    }
    else if (value.IsBigInt())
    {
        bool lossless;
        int64_t intVal = value.As<Napi::BigInt>().Int64Value(&lossless);
        if (lossless)
        {
            return yyjson_mut_sint(doc, intVal);
        }
        else
        {
            // 处理超出范围的 BigInt
            // 这里可以选择将其转换为字符串
            std::string str = value.As<Napi::BigInt>().ToString();
            return yyjson_mut_str(doc, str.c_str());
        }
    }
    else if (value.IsObject())
    {
        Napi::Object obj = value.As<Napi::Object>();
        yyjson_mut_val *yyobj = yyjson_mut_obj(doc);
        Napi::Array keys = obj.GetPropertyNames();
        for (uint32_t i = 0; i < keys.Length(); ++i)
        {
            Napi::Value key = keys[i];
            if (key.IsString())
            {
                std::string keyStr = key.As<Napi::String>().Utf8Value();
                Napi::Value val = obj.Get(key);
                yyjson_mut_val *yykey = yyjson_mut_str(doc, keyStr.c_str());
                yyjson_mut_val *yyval = NapiToYyjson(env, doc, val);
                yyjson_mut_obj_add(yyobj, yykey, yyval);
            }
        }
        return yyobj;
    }
    else if (value.IsArray())
    {
        Napi::Array arr = value.As<Napi::Array>();
        yyjson_mut_val *yyarr = yyjson_mut_arr(doc);
        for (uint32_t i = 0; i < arr.Length(); ++i)
        {
            yyjson_mut_val *elem = NapiToYyjson(env, doc, arr[i]);
            yyjson_mut_arr_add_val(yyarr, elem);
        }
        return yyarr;
    }
    return yyjson_mut_null(doc);
}

// 将 JSON 字符串解析成 JavaScript 对象
Napi::Value Parse(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "Expected a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string jsonStr = info[0].As<Napi::String>().Utf8Value();
    yyjson_doc *doc = yyjson_read(jsonStr.c_str(), jsonStr.length(), 0);
    if (!doc)
    {
        Napi::Error::New(env, "Invalid JSON").ThrowAsJavaScriptException();
        return env.Null();
    }

    yyjson_val *root = yyjson_doc_get_root(doc);
    Napi::Value result = YyjsonToNapi(env, root);
    yyjson_doc_free(doc);
    return result;
}

// 将 JavaScript 对象序列化为 JSON 字符串
Napi::Value Stringify(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "Expected an object").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object jsObj = info[0].As<Napi::Object>();
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = NapiToYyjson(env, doc, jsObj);
    yyjson_mut_doc_set_root(doc, root);

    const char *jsonStr = yyjson_mut_write(doc, 0, NULL);
    Napi::String result = Napi::String::New(env, jsonStr);
    yyjson_mut_doc_free(doc);
    return result;
}

// 初始化模块
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "parse"), Napi::Function::New(env, Parse));
    exports.Set(Napi::String::New(env, "stringify"), Napi::Function::New(env, Stringify));
    return exports;
}

NODE_API_MODULE(yyjson, Init)