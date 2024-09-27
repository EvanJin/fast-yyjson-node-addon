{
  "targets": [
    {
      "target_name": "yyjson",
      "sources": [ "src/addon.cc", "src/yyjson.c" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags": [
        "-O3",
        "-stdlib=libc++",          # 使用 libc++ 标准库
        "-mmacosx-version-min=10.9" # 设置最低支持的 macOS 版本
      ],
      "cflags_cc": [
        "-std=c++17",
        "-stdlib=libc++",
        "-mmacosx-version-min=10.9"
      ],
      "xcode_settings": {
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.9"
      },
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "libraries": [],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      }
    }
  ]
}