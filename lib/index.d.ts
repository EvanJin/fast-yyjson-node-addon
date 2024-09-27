/**
 * 解析 JSON 字符串，超出安全整数范围的 bigint 会被转换为字符串
 * @param json - 要解析的 JSON 字符串
 * @returns 解析后的 JavaScript 对象
 */
export function parse(json: string): any;

/**
 * 序列化 JavaScript 对象为 JSON 字符串，bigint 类型会被转换为字符串
 * @param value - 要序列化的 JavaScript 对象
 * @returns 序列化后的 JSON 字符串
 */
export function stringify(value: any): string;
