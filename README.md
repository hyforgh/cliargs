# cliargs (Command Line Interface Arguments Parser)

`cliargs` 是一个轻量级、功能强大的 C++ 命令行参数解析库。它仅包含一个头文件（Header-only），支持标准的 GNU 命令行参数语法风格。

本项目的设计灵感致敬了 **getopts** (GNU C/Linux Shell)、**cxxopts** (C++) 和 **argparse** (Python)，但在容器支持上更加强大，同时支持自定义数据类型。

---

## 🚀 1. 快速开始

### 基本特性

支持标准的 GNU 风格参数语法：

*   `--long_option`
*   `--long_option=value`
*   `-s` (短选项)
*   `-abc` (组合短选项)

### 代码示例

这是一个简单的入门示例，展示了如何定义参数、解析以及获取结果。

```cpp
#include "cliargs.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    // 1. 创建解析器实例
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");

    // 2. 定义参数
    parser.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // 布尔型参数
        ('i', "int", "An integer", cliargs::value<int>()->default_value(-1))
        ('s', "string", "A string", cliargs::value<std::string>()->default_value("/dev/mem"), "str")
        ('v', "vector", "An int vector", cliargs::value<std::vector<float>>());

    // 3. 解析参数
    auto result = parser.parse(argc, argv);

    // 4. 处理帮助或错误
    if (result.error() || result["help"].as<bool>()) {
        result.print_help();
        return result.error() ? -1 : 0;
    }

    // 5. 使用结果
    auto int_value = result["int"].as<int>();
    std::cout << "   int: " << cliargs::to_string(int_value) << std::endl;

    auto &str_value = result["string"].as<std::string>();
    std::cout << "string: " << cliargs::to_string(str_value) << std::endl;

    auto &vec_value = result["vector"].as<std::vector<float>>();
    std::cout << "vector: " << cliargs::to_string(vec_value) << std::endl;

    return 0;
}
```

运行效果：

* 查看帮助信息

```bash
./simple -h

One line description of MyProgram
Usage: MyProgram  ...
    -h, --help   Print this message and exit
    -i, --int    An integer
                      value: 'int'[optional]; default:-1
    -s, --string A string
                      value: 'string'[optional]; default:"/dev/mem"
        --str    same as '-s, --string'
    -v, --vector An int vector
                      value: 'vector<float>'[0~N]
```

* 解析参数

```bash
./simple -i=32 -s hello -v 1.5 2.5 3.5

   int: 32
string: "hello"
vector: [1.5, 2.5, 3.5]
```

## 🛠️ 2. 高级特性

## 2.1 丰富的类型支持

|layout|C++ Type|usage|
|------|--------|-----|
|scalar|`char`, `short`, `int`, `long`, `long long`<br>`unsigned ...`<br>`float`, `double`, `bool`<br>`char *`, `const char *`, `std::string`|`--arg_name arg_value`<br>`--arg_name=arg_value`<br>`--arg_name`|
|vector|`std::vector<scalar>`|`--arg_name v1 v2 v3 ...`<br>`--arg_name v1 v2 --arg_name v3 ...`|
|matrix|`std::vector<std::vector<scalar>>`|`--arg_name v00 v01 ... --arg_name v10 v11 ...`|
|map   |`std::map<scalar, scalar>`<br>`std::unordered_map<...>`|`--arg_name key1 v1 --arg_name key2 v2`|
|      |`std::map<scalar, std::vector<scalar>>`<br>`std::unordered_map<...>`|`--arg_name key1 k1v1 k1v2 ... --arg_name key2 k2v1 k2v2 ...`
|tuple |`std::tuple<scalar...>`|`--arg_name v1 v2 ...`|
|      |`std::vector<std::tuple<scalar...>>`|`--arg_name v00 v01 ... --arg_name v10 v11 ...`
|      |`std::map<scalar, std::tuple<scalar...>>`<br>`std::unordered_map<...>`|`--arg_name key1 k1v1 k1v2 ... --arg_name key2 k2v1 k2v2 ...`

> 注意：容器最多支持两层嵌套，例如：`std::map<std::string, std::vector<int>>`, `std::vector<std::vector<int>>`

## 2.2 自定义结构体支持

你可以轻松地将自定义结构体集成到参数解析中。

* 定义结构体

```c++
struct MyStruct {
    std::string name;
    float gain;
    long size;
};
```

* 重载解析函数

通过重载 `cliargs_parse_custom`，你可以控制结构体的解析逻辑。重载此函数后，程序用户需要使用**字符串数组**给结构体赋值。[完整示例](examples/custom.cpp)

```c++
void cliargs_parse_custom(MyStruct &obj, cliargs::ArgParser &parser) {
    parser.domain_begin("MyStruct"); // 设置结构体名称
    // 必填字段
    if (parser.assign(obj.name, "name")) { // MyStruct::name required a string value
        parser.check(!obj.name.empty(), "invalid name: empty");
    }
    parser.assign(obj.gain, "gain"); // MyStruct::gain required an uint64 value

    // 可选字段
    parser.set_optional();
    parser.assign(obj.size, "size", (long)0); // specify a default value for optional member
    parser.domain_end();
}
```

* 用法示例

```bash
./custom --my_struct data.bin 32 64
./custom --my_struct data.bin 32
```

> 说明：`cliargs` 将自动推导类型名称：
> `assert(cliargs::type_traits<MyStruct>::name() == "MyStruct{string, float [, long]}");`
> `assert(cliargs::type_traits<std::vector<MyStruct>>::name() == "vector<MyStruct{string, float [, long]}>");`

* 结构体与容器结合

|layout|c++ type|usage|
|------|--------|-----|
|single|`MyStruct`|`--arg_name name gain [size]`
|vector|`std::vector<MyStruct>`|`--arg_name name1 gain1 [size1]`<br>`--arg_name name2 gain2 [size2]`<br>`...`|
|map   |`std::map<scalar, MyStruct>`|`--arg_name key1 name1 gain1 [size1]`<br>`--arg_name key2 name2 gain2 [size2]`<br>`...`|


## 2.3 参数约束与校验

cliargs 提供了强大的约束接口，支持 `choices`（枚举）、`ranges`（范围）、`regex`（正则）以及自定义 `examine` 回调。

* 约束逻辑公式：

`(choices || ranges || regex) && examine`


* 代码示例

```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    // 字符串约束：必须是 "dump" / "load" 或匹配正则 "\d+"
    ('s', "string", "A string argument which only can be one of {'file', 'directory'})",
        cliargs::value<std::string>()
        ->choices({"dump", "load"})
        ->regex("\\d+", "a integer")
    )

    // 整数约束：必须是 1/3/5，或在 [10,20]/[30,50] 范围内，且必须是奇数
    ('i', "integer", "An integer argument which only can be one of {1, 3, 5}",
        cliargs::value<std::vector<int>>()
        ->choices({1, 3, 5})
        ->ranges({{10, 20}, {30, 50}})
        ->examine([](int &v) ->bool { return v % 2; }, "an odd number")
    );
```

### 2.4 默认值与隐含值

* **默认值 (default_value)**：当用户未指定参数时使用的值。
* **隐含值 (implicit_value)**：当用户指定了参数名但未提供值（或提供的值不足）时填充的值。

示例场景：

|case|user action|result|
|----|-----------|------|
|`cliargs::value<int>()`<br>`->default_value(1)`<br>`->implicit_value(5)`|（未指定）|1
|    |`--arg_name`  |5
|    |`--arg_name 6`|6
|`cliargs::value<vector<int>>()`<br>`->data_count(1, 3)`<br>`->implicit_value({0, 1, 2, 3})`|（未指定）|报错（此参数要求最少一个值）
|    |`--arg_name`        |报错（此参数要求最少一个值）
|    |`--arg_name 1`      |`[1, 1, 2]`
|    |`--arg_name 1 2`    |`[1, 2, 2]`
|    |`--arg_name 1 2 3`  |`[1, 2, 3]`
|    |`--arg_name 1 2 3 4`|报错（此参数要求最多 3 个值）或者 `4` 被位置参数吃掉

## 2.5 容器尺寸限制

你可以限制容器（Vector, Matrix, Tuple）的尺寸范围。

```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    // Vector: 限制元素个数为 3~5 个
    ('v', "vector", "A size limited vector",
        cliargs::value<std::vector<float>>()
        ->data_count(3, 5)
    )

    // Matrix: 限制行数 3~5，列数固定为 2
    ('m', "matrix", "A size limited matrix",
        cliargs::value<std::vector<std::vector<int>>>()
        ->data_count(3, 5)
        ->line_width(2, 2)
    )

    // Tuple: 前两个元素必填，第三个可选
    ('t', "tuple", "A tuple with default tail",
        cliargs::value<std::tuple<std::string, int, float>>()
        ->line_width(2)
        ->implicit_value({"", 0, 1.0f})
    );
```

## 2.6 参数嵌套与透传

支持类似 `--` 的参数透传功能，常用于将参数传递给子进程。

* **GNU 风格：** `--` 之后的所有内容将被视为 `tail` 参数。
* **Stop-at-EOF：** 对于 `std::vector<std::string>` 或 `std::vector<char *>` 或 `std::vector<const char *>` 类型的参数，设置 `stop_at_eof()` 属性后，它会吞掉后续所有参数直到遇到 `--`。

```c++
cliargs::Parser parser("MyProgram", "One line description of MyProgram");
...
auto result = parser.parse(argc, argv);
...
auto &tail = result.tail();
```

# 🛠️ 3. 编译期配置 (Configuration Macros)

为了适应不同的编译环境或禁用特定功能，cliargs 提供了以下宏定义。

**注意：这些宏必须在 `#include "cliargs.hpp"` 之前定义。**

|宏定义               |描述|
|--------------------|---|
|CLIARGS_NO_RTTI	 |禁用 RTTI (Run-Time Type Information)。如果你的项目编译时使用了 -fno-rtti，请定义此宏。
|CLIARGS_NO_EXCEPTION|禁用异常支持。定义后，库将使用 std::abort() 或错误码代替异常抛出。
|CLIARGS_NO_WARNING  |禁用编译期警告。用于屏蔽关于自定义类型 `type_name` 的提示。
|CLIARGS_NO_REGEX    |禁用正则表达式支持。主要用于规避旧版本 GCC (如 4.8/4.9) 中 std::regex 的已知 Bug。

关于 `CLIARGS_NO_REGEX` 的备选方案：
如果你禁用了正则支持，可以使用 `ArgAttr::examine()` 配合第三方正则库（如 Boost.Regex）来实现类似的校验功能。


# 📚 4. 接口参考

## 4.1 通用属性接口

适用于所有类型的参数。

|attribute            |description|
|---------------------|-----------|
|`required()`	      |标记参数为必填
|`positional()`       |标记为位置参数（无需参数名）
|`default_value(T)`   |设置默认值：用户不指定此参数时的返回值
|`implicit_value(...)`|设置隐含值：用户只指定参数名称，不指定参数值时的返回值
|`examine(Func)`      |设置自定义校验回调
|`hide()`	          |在帮助信息中隐藏该参数
|`concise_help()`     |帮助信息中不显示自动生成的内容（如数据类型、默认值值、隐含值和约束）

### 4.1.1 隐含值（implicit_value）的数据类型

```c++
cliargs::value<T1>()->implicit_value(T2);
```

|T1                        |T2|
|--------------------------|--------------|
|`scalar`                  |`scalar`
|`vector<scalar>`          |`vector<scalar>`
|`vector<vector<scalar>>`  |`vector<scalar>`
|`map<key, scalar>`        |`scalar`
|`map<key, vector<scalar>>`|`vector<scalar>`
|`tuple<scalar...>`        |`tuple<scalar...>`
|`struct`                  |`struct`

### 4.1.2 examine

适用于所有数据类型（包括 `tuple` 和 `struct`）。程序开发者可以通过此接口注册自定义的回调函数，以检查（或修改）单个数值。如果回调函数返回 `false`，则认为程序用户提供的参数值不符合要求。

```c++
examine(std::function<bool(T &value)> func, std::string desc = "");
examine(std::function<bool(T &value, void *context)> func, std::string desc = "");
```

> 参数说明：
> `context`: 通过 `value<T>()->context(void *ctx)` 设置的用户数据指针
> `desc`: 将会显示在帮助信息中

## 4.2 基础类型特有属性

|meta type|attributes |description|
|---------|-----------|-----------|
|numeric  |`choices(vector<T>)`        |设置枚举值
|         |`ranges(vector<pair<T, T>>)`|设置多个范围
|         |`range(T, T)`               |设置单个范围
|string   |`choices(vector<string>)`   |设置枚举值
|         |`regex(string)`             |设置正则表达式
|         |`stop_at_eof()`|吞掉后续所有参数（包括以 `-` 或 `--` 开头的字符串）直到遇到 `--`<br>或用 `--` 提前结束此参数解析
|tuple    |`stop_at_eof()`|最后一个成员是 `string` 类型时生效，效果同上
|struct   |`stop_at_eof()`|最后一个成员是 `string` 类型时生效，效果同上

### 4.2.1 choices

仅适用于单值类型（整数、浮点数和字符串）及其衍生复合类型，其它类型无此接口。程序开发者可以通过此接口设置由确定的枚举值组成的集合。

```c++
choices(std::vector<T> value_set, std::string desc = "");
```

### 4.2.2 ranges

仅适用于数值类型（整数和浮点数）。其它类型无此接口。程序开发者可以通过此接口设置数值的取值范围，此接口可以调用多次每次可以指定多个范围

```c++
range(T min_value, T max_value, std::string desc = "");
ranges(std::vector<std::pair<T, T>> pairs, std::string desc = "");
```

### 4.2.3 regex

仅适用于字符串类型及其衍生复合类型，其它类型无此接口。程序开发者可以通过此接口设置一个正则表达式。

> 注意：旧版本 GCC (如 4.8/4.9) 中 std::regex 的已知 Bug，请慎用。

```c++
regex(std::string regex_string, std::string desc = "");
```

## 4.3 容器类型特有属性接口
|      |attribute |description|
|------|----------|-----------|
|vector|`data_count(int at_least, int at_most = -1)`|设置 vector 长度的下限和上限。（at_most = -1 表示无限大）
|matrix|`data_count(int at_least, int at_most = -1)`|设置 matrix 的行数下限和上限
|      |`line_width(int at_least, int at_most = -1)`|设置 matrix 的列数下限和上限
|map   |`line_width(int at_least, int at_most = -1)`|只对 map<key, vector> 有用设置 vector 的长度的下限和上限
|tuple |`line_width(int at_least)`|设置 tuple 的最小参数个数

## 4.4 Parser 配置

### 4.4.1 Parser 属性配置

|attribute|description|
|---------|-----------|
|`allow_unknown` |当用户指定了未定义的命令行参数名称时不报错
|`set_width`     |设置帮助信息中每行显示的最大字符数
|`concise_help`  |帮助信息中不显示自动生成的内容（如数据类型、默认值值、隐含值和约束）
|`gnu_mode`      |开启[“GNU 模式”](#53-gnu-模式说明)

### 4.4.2 add_args

|method|
|------|
|`(char flag, std::string name, std::string desc, std::string alias="")`
|`(std::string name, std::string desc, std::string alias="")`
|`(char flag, std::string name, std::string desc, std::shared_ptr<ArgAttr<T>> attr, std::string alias="")`
|`(std::string name, std::string desc, std::shared_ptr<ArgAttr<T>> attr, std::string alias="")`

## 4.5 Result 接口

### 4.5.1 error

判断参数定义或者解析过程中是否有错误

### 4.5.2 print_help

打印帮助信息和错误信息

### 4.5.3 as<T>

获取参数。函数返回数据类型为 T

### 4.5.4 tail

位于 `--` 后，且第一个字符串不是带有 `stop_at_eof()` 属性的命令行参数名的所有字符串将被放入 `cliargs::Result::tail()` 返回的 `Tail` 对象中

```c++
struct cliargs::Result::Tail {
    int argc;
    char **argv;
};
```

# 💡 5. 其他注意事项

## 5.1 反向布尔参数

默认布尔参数隐式值为 `true`。你可以通过设置 `implicit_value(false)` 来创建反向开关。

```cpp
parser.add_args()
    ('r', "reverse_bool", "Disable feature", cliargs::value<bool>()->implicit_value(false));
```

## 5.2 指针类型慎用
虽然支持 `char *` 或 `const char *`，但解析器会直接指向 `argv` 中的原始字符串内存。请勿修改这些内存，且在 `main` 函数结束后它们将失效。

## 5.3 GNU 模式说明

* **默认模式：**以 - 开头的字符串会被视为新参数。如果需要输入以 - 开头的字符串值，请使用转义符 \（如 --name \-abc）。
* **GNU 模式：**开启后，解析器会强制将参数名后的内容视为值，即使它以 - 开头。
