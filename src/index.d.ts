declare module "yyjson" {
  interface Yyjson {
    parse(json: string): any;
    stringify(value: any): string;
  }

  const yyjson: Yyjson;
  export = yyjson;
}
