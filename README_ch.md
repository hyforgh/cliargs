## 简介
cliargs (**Command Line Interface Arguments** Parser)
这是一个 C++ 程序命令行参数解析器。支持标准的 GNU 命令行参数语法风格；此程序库仅包含一个头文件（header-only）。

功能类似于（致敬）：**getopts** (GNU C / Linux Shell)、**cxxpots** (C++)、**argparse** (Python)

### 1. 基本特性
#### 1.1 支持标准的 GNU 命令行参数语法
命令行参数可以这样指定：

    --long1
    --long2=value
    --long2 value
    -a
    -ab
    -abc value

其中， `long2` 和 `c` 需要参数值，`long1`、`a` 和 `b` 不需要参数值。

#### 1.2 快速上手
[examples/simple.cpp](examples/simple.cpp)
```cpp
#include "cliargs.hpp"

int main(int argc, char *argv[]) {
    //  Create a 'cliargs::Parser' instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");
    // Define arguments
    parser.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ('i', "int", "An interger", cliargs::value<int>()->default_value(-1))
        ('s', "string", "A string", cliargs::value<std::string>()->default_value("/dev/mem"))
        ('v', "vector", "An int vector", cliargs::value<std::vector<float>>())
        ;
    // Parse
    if (parser.error() || result["help"].as<bool>()) {
        parser.print_help();
        return parser.error() ? -1 : 0;
    }
    // Use result
    std::cout << "   int: " << cliargs::to_string(result["int"].as<int>()) << std::endl;
    std::cout << "string: " << cliargs::to_string(result["string"].as<std::string>()) << std::endl;
    std::cout << "vector: " << cliargs::to_string(result["vector"].as<std::vector<float>>()) << std::endl;
    return 0;
}
```
```sh
./simple -h

One line description of MyProgram
Usage: MyProgram  ...
    -h, --help   Print this message and exit
    -i, --int    An interger
                      value: 'int'[optional]; default:-1
    -s, --string A string
                      value: 'string'[optional]; default:"/dev/mem"
    -v, --vector An int vector
                      value: 'vector<float>'[0~N]
```
```sh
./simple -i=32 -s hello -v 1.5 2.5 3.5

   int: 32
string: "hello"
vector: [1.5, 2.5, 3.5]
```

### 2. 高级特性
#### 2.1 支持标准 C++ 类型和部分 STL 容器
|layout|c++ type|usage|
|------|--------|-----|
|scalar|`char`, `short`, `int`, `long`, `long long`<br>`unsigned ...`<br>`float`, `double`, `bool`<br>`char *`, `const char *`, `std::string`|`--arg_name arg_value`<br>`--arg_name=arg_value`<br>`--arg_name`|
|vector|`std::vector<scalar>`|`--arg_name v1 v2 v3 ...`<br>`--arg_name v1 v2 --arg_name v3 ...`|
|matrix|`std::vector<std::vector<scalar>>`|`--arg_name v00 v01 ... --arg_name v10 v11 ...`|
|map   |`std::map<scalar, scalar>`<br>`std::unordered_map<...>`|`--arg_name key1 v1 --arg_name key2 v2`|
|      |`std::map<scalar, std::vector<scalar>>`<br>`std::unordered_map<...>`|`--arg_name key1 k1v1 k1v2 ... --arg_name key2 k2v1 k2v2 ...`
|tuple |`std::tuple<scalar...>`|`--arg_name v1 v2 ...`|
|      |`std::vector<std::tuple<scalar...>>`|`--arg_name v00 v01 ... --arg_name v10 v11 ...`
|      |`std::map<scalar, std::tuple<scalar...>>`<br>`std::unordered_map<...>`|`--arg_name key1 k1v1 k1v2 ... --arg_name key2 k2v1 k2v2 ...`

#### 2.2 支持自定义结构体类型及其与 STL 容器结合后的衍生类型
```c++
struct MyStruct {
    std::string name;
    float gain;
    long size;
};
// overload oerator << for printing default-value in help
std::ostream &operator << (std::ostream &os, const MyStruct &obj) {
    os << "{.name=" << obj.name << ", .gain=" << obj.gain << ", .size=" << obj.size << "}";
    return os;
}
```
##### 2.2.1 重载解析函数
以下函数二选一即可，如果两个解析函数都存在，则 `__parse_by_parser` 将被调用。
###### 2.2.1.1 重载 `__parse_by_parser`
重载此函数后，程序用户需要使用**字符串数组**给结构体赋值。[完整示例](examples/my_struct_parse_by_parser.cpp)
```c++
void __parse_by_parser(MyStruct &obj, cliargs::ArgParser &parser, const std::string &name) {
    parser.domain_begin(name.empty() ? "MyStruct" : name); // tell ArgParser the struct's name
    if (parser.assign(obj.name, "name")) { // MyStruct::name required a string value
        parser.check(!obj.name.empty(), "invalid name: empty");
    }
    parser.assign(obj.gain, "gain"); // MyStruct::gain required an uint64 value
    parser.set_optional(); // the followwing member is optional
    parser.assign(obj.size, "size", (long)0); // specify a default value for optional member
    parser.domain_end();
}
```
```bash
--my_struct data.bin 32 64
--my_struct data.bin 32
```

> 说明：`cliargs` 将自动推导 `MyStruct` 的类型名称
    assert(cliargs::type_traits<MyStruct>::name() == "{string, float[, long]}");
    assert(cliargs::type_traits<std::vector<MyStruct>>::name() == "vector<{string, float[, long]}>");

###### 2.2.1.2 重载 `__parse_by_format`
重载此函数后，程序用户需要使用**单个结构化的字符串**给结构体赋值。[完整示例](examples/my_struct_parse_by_format.cpp)
```c++
const char *__parse_by_format(MyStruct &obj, char *psz
        , std::string name, std::list<std::string> &err_list
        , void *context, char *parent) {
    const char *type_name = "\"string,float[,long]\"";
    if (!psz || !psz[0]) {
        return type_name;
    }
    // try to split the string like "data.bin,32,64" or "data.bin,32"
    ...
    // asign struct's member
    // push messages into err_list if something is bad
    ...
    return type_name; // return the type name
}
```
```bash
--my_struct "data.bin,32,64"
--my_struct "data.bin,32"
```
> 说明：`cliargs` 将自动推导 `MyStruct` 的类型名称
assert(cliargs::type_traits<MyStruct>::name() == "{\"string,float[,long]\"}");
assert(cliargs::type_traits<std::vector<MyStruct>>::name() == "vector<{\"string,float[,long]\"}>");

##### 2.2.2 结构体与 STL 容器结合
|layout|c++ type|__parse_by|usage|
|------|--------|------------|-----|
|single|`MyStruct`|`parser`|`--arg_name name gain [size]`
|      |          |`format`|`--arg_name name,gain[,size]`<br>`--arg_name=name,gain[,size]`
|vector|`std::vector<MyStruct>`|`parser`|`--arg_name name1 gain1 [size1] --arg_name name2 gain2 [size2] ...`|
|      |                       |`format`|`--arg_name name,gain[,size] --arg_name=name,gain[,size] ...`
|map   |`std::map<scalar, MyStruct>`|`parser`|`--arg_name key1 name1 gain1 [size1] --arg_name key2 name2 gain2 [size2] ...`|
|      |                            |`format`|`--arg_name key1 name,gain[,size] --arg_name key 2 name,gain[,size] ...`


#### 2.3 支持广义的枚举类型
```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ('s', "string", "A string argument which only can be one of {'file', 'directory'})",
        cliargs::value<std::string>()
        ->choices({"dump", "load"})
        ->regex("\\d+", "a integer")
        // the value can only be one of {"dump", "load"} or an interger
    )
    ('i', "interger", "An interger argument which only can be one of {1, 3, 5}",
        cliargs::value<std::vector<int>>()
        ->choices({1, 3, 5})
        ->ranges({{10, 20}, {30, 50}})
        ->examine([](int &v) ->bool { return v % 2; }, "an odd number")
        // the value can only be one of {1, 3, 5} or in range [10, 20] or [30, 50] and be an odd number
    );
```

##### 2.3.1 各种约束的结合规则
解析器将按此顺序和公式判断用户指定的参数是否符合要求：
`(choices || ranges || regex) && examine`

##### 2.3.2 examine
适用于所有数据类型（包括 `tuple` 和 `struct`）。程序开发者可以通过此接口注册自定义的回调函数，以检查（或修改）单个数值。如果回调函数返回 `false`，则认为程序用户提供的参数值不符合要求。
```c++
examine(std::function<bool(T &value)> func, std::string desc = "");
examine(std::function<bool(T &value, void *context)> func, std::string desc = "");
examine(std::function<bool(T &value, void *context, void *data)> func, std::string desc = "");
```

##### 2.3.3 choices
仅适用于单值类型（整数、浮点数和字符串）及其衍生复合类型，其它类型无此接口。程序开发者可以通过此接口设置由确定的枚举值组成的集合。
```c++
choices(std::unordered_set<T> value_set, std::string desc = "");
```

##### 2.3.4 ranges
仅适用于数值类型（整数和浮点数）。其它类型无此接口。程序开发者可以通过此接口设置数值的取值范围，此接口可以调用多次每次可以指定多个范围
```c++
ranges(T min_value, T max_value, std::string desc = "");
ranges(std::vector<std::pair<T, T>> pairs, std::string desc = "");
```

##### 2.3.5 regex
仅适用于字符串类型及其衍生复合类型，其它类型无此接口。程序开发者可以通过此接口设置一个正则表达式。
```c++
regex(std::string regex_string, std::string desc = "");
```

#### 2.4 支持默认值和隐含值
##### 2.4.1 默认值
```c++
default_value(T &&data);
```
对于提供了默认值的命令行参数，当用户没有设置时，将使用默认参数值。

##### 2.4.2 隐含值
```c++
implicit_value(typename T::value_type value);
```
对于提供了隐含值的命令行参数，如果用户提供的参数值的个数小于隐含值中参数值的个数，则解析器将自动使用隐含值的后部分参数值填充命令行参数，直到达到命令行参数的参数值个数的上限。
|case|user action|result|
|----|-----------|------|
|`cliargs::value<int>()`<br>`->default_value(1)`<br>`->implicit_value(5)`|not specified |1
|    |`--arg_name`  |5
|    |`--arg_name 6`|6
|`cliargs::value<vector<int>>()`<br>`->data_count(1, 3)`<br>`->implicit_value({0, 1, 2, 3})`|not specified|error
|    |`--arg_name`|error
|    |`--arg_name 1`|`[1, 1, 2]`
|    |`--arg_name 1 2`|`[1, 2, 2]`
|    |`--arg_name 1 2 3`|`[1, 2, 3]`
|    |`--arg_name 1 2 3 4`|error or `4` is accepted by positional argument

#### 2.5 支持限制容器的尺寸范围
```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ('v', "vector", "A size limited vector",
        cliargs::value<std::vector<float>>()
        ->data_count(3, 5) // the vector can only accepts 3 ~ 5 datas
        ->implicit_value({0, 0, 0, 44, 55})
    )
    ('m', "matrix", "A size limited matrix",
        cliargs::value<std::vector<std::vector<int>>>()
        ->data_count(3, 5) // the matrix's height is 3 ~ 5
        ->line_width(2, 2) // the matrix's width is 2
    )
    ('t', "tuple", "A tuple with default tail",
        cliargs::value<std::tuple<std::string, int, float>>()
        ->line_width(2) // tuple[0] and tuple[1] is required and tuple[2] is optional
        ->implicit_value({"", 0, 1.0f})
    );
```

## 3. 命令行参数属性接口汇总
属性接口包括“[通用属性接口](#通用属性接口)”、“[元数据类型特有属性接口](#元数据类型特有属性接口)”和“[容器类型特有属性接口](#容器类型特有属性接口)”。最终的组合类型的属性接口是**三者的并集**。

> 详细组合用法示例请参阅单元测试

#### 3.1 通用属性接口
|attribue     |
|-------------|
|required     |
|positional   |
|default_value|
|examine      |
|hide         |

#### 3.2 元数据类型特有属性接口
|meta type|attributes|
|---------|----------|
|numerical|choices, ranges
|string   |choices, regex
|struct   |

#### 3.3 容器类型特有属性接口
|      |attribute|
|------|---------|
|scalar|implicit_value
|vector|implicit_value, data_count
|matrix|implicit_value, data_count, line_width
|map   |implicit_value, line_width
|tuple |implicit_value, line_width

#### 3.4 `default_value` 和 `implicit_value` 的数据类型
`default_value` 的数据类型和命令行参数的数据类型一样

`implicit_value` 的数据类型如下表：

|argument                  |implicit_value|
|--------------------------|--------------|
|`scalar`                  |`scalar`
|`vector<scalar>`          |`vector<scalar>`
|`vector<vector<scalar>>`  |`vector<scalar>`
|`map<key, scalar>`        |`scalar`
|`map<key, vector<scalar>>`|`vector<scalar>`
|`tuple<scalar...>`        |`tuple<scalar...>`

#### 3.5 cliargs::Parser 的属性
|attribute|
|---------|
|`allow_unknow`
|`set_width`
|`concise_help`

### 4. 其它
#### 4.1 反向布尔参数
```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ('h', "help", "Normal boolean arguemnt")
    ('r', "reverse_bool", "Reverse boolean argument",
        cliargs::value()->implicit_value(false)
    );
```
普通布尔参数的属性为： `cliargs::value()->implicit_value(true)`

#### 4.2 请审慎地使用指针类型
```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ("ptr", "char *", cliargs::value<char *>())
    ("const_ptr", "const char *", cliargs::AsArgs<const char *>());
```
* 如果将自己的数据类型声明为指针类型（`char *` 或 `const char *`）、或将指针类型与 STL 容器结合、或将指针嵌入到 `tuple` 中，则指针将直接指向程序用户传递给 `main` 函数的字符串
* 如果将指针类型嵌入到结构体中，且采用 `__parse_by_parser` 为结构体成员赋值，则指针类型将指向程序用户传递给 `main` 函数的字符串
* 如果将指针类型嵌入到结构体中，且采用 `__parse_by_format` 为结构体成员赋值，请**程序开发者自行保证指针的有效性**

#### 4.3 字符串序列化工具
```c++
template <typename T>
std::string cliargs::to_string(const T &value
    , const std::string &delimiter = ","
    , const std::string &gap = " "
    );
```
可将组合数据类型转换成字符串。
* 示例：
```c++
std::map<std::string, std::tuple<int, std::vector<float>>> data = {
    {"key1": {1, {2.5, 3.5}}},
    {"key2": {2, {4.5, 5.5}}},
};
std::cout << cliargs::to_string(data) << std::endl;
```
* 输出：
```log
{"key1": (1, [2.5, 3.5]), "key2": (2, [4.5, 5.5])}
```

#### 4.4 获取类型名称的工具
```c++
template <typename T>
const std::string &cliargs::type_traits<T>::name();
```
可获得组合数据类型的字符串名称。对于自定义结构体类型，需要先重载 `__parse_by_parser` 或 `__parse_by_format`。
* 示例：
```c++
typedef std::map<std::string, std::tuple<int, std::vector<float>>> MyType;
std::cout << cliargs::type_traits<MyType>::name() << std::endl;
```
* 输出：
```log
map<string, tuple<int, vector<float>>>
```
