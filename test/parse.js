const yyjson = require("../src");

const jsonStr =
  '[{"name":"John","age":30,"bigIntValue":9223372036854775807}, {"name":"John","age":30,"bigIntValue":9223372036854775807}]';
const parsed = yyjson.parse(jsonStr);
console.log(parsed);
