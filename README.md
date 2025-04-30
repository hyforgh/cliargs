## Introduction
cliargs (**Command Line Interface Arguments** Parser)
This is a C++ program command line argument parser. Supports standard GNU command line argument syntax style; this library contains only one header file (header-only).

Functions similar to (tribute to): **getopts** (GNU C / Linux Shell), **cxxpots** (C++), **argparse** (Python)

### 1. Basic features
#### 1.1 Support standard GNU command line argument syntax
Command line arguments can be specified as follows:

    --long1
    --long2=value
    --long2 value
    -a
    -ab
    -abc value

Among them, `long2` and `c` require parameter values, while `long1`, `a` and `b` do not.

#### 1.2 Get started quickly
[examples/simple.cpp](examples/simple.cpp)
```cpp
#include "cliargs.hpp"

int main(int argc, char *argv[]) {
    // Create a 'cliargs::Parser' instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");
    // Define arguments
    parser.set_width(120).add_args()
    ('h', "help", "Print this message and exit") // a bool argument
    ('i', "int", "An interger", cliargs::value<int>()->default_value(-1))
    ('s', "string", "A string", cliargs::value<std::string>()->default_value("/dev/mem"))
    ('v', "vector", "An int vector", cliargs::value<std::vector<float>>())
    ;
    //Parse
    auto result = parser.parse(argc, argv);
    if (parser.error() || result["help"].as<bool>()) {
        parser.print_help();
        return parser.error() ? -1 : 0;
    }
    //Use result
    std::cout << " int: " << cliargs::to_string(result["int"].as<int>()) << std::endl;
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

### 2. Advanced features
#### 2.1 Support standard C++ types and some STL containers
|layout|c++ type|usage|
|------|--------|-----|
|scalar|`char`, `short`, `int`, `long`, `long long`<br>`unsigned ...`<br>`float`, `double`, `bool`<br>`char *`, `const char *`, `std::string`|`--arg_name arg_value`<br>`--arg_name=arg_value`<br>`--arg_name`|
|vector|`std::vector<scalar>`|`--arg_name v1 v2 v3 ...`<br>`--arg_name v1 v2 --arg_name v3 ...`|
|matrix|`std::vector<std::vector<scalar>>`|`--arg_name v00 v01 ... --arg_name v10 v11 ...`|
|map |`std::map<scalar, scalar>`<br>`std::unordered_map<...>`|`--arg_name key1 v1 --arg_name key2 v2`|
| |`std::map<scalar, std::vector<scalar>>`<br>`std::unordered_map<...>`|`--arg_name key1 k1v1 k1v2 ... --arg_name key2 k2v1 k2v2 ...`
|tuple |`std::tuple<scalar...>`|`--arg_name v1 v2 ...`|
| |`std::vector<std::tuple<scalar...>>`|`--arg_name v00 v01 ... --arg_name v10 v11 ...`
| |`std::map<scalar, std::tuple<scalar...>>`<br>`std::unordered_map<...>`|`--arg_name key1 k1v1 k1v2 ... --arg_name key2 k2v1 k2v2 ...`

#### 2.2 Support for custom structure types and their derivative types after combining with STL containers
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
##### 2.2.1 Overload the parsing function
Choose one of the following functions. If both parsing functions exist, `__parse_by_parser` will be called.
###### 2.2.1.1 Overload `__parse_by_parser`
After overloading this function, the program user needs to use **string array** to assign values ​​to the structure. [Complete example](examples/my_struct_parse_by_parser.cpp)
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

> Note: `cliargs` will automatically infer the type name of `MyStruct`
assert(cliargs::type_traits<MyStruct>::name() == "{string, float[, long]}");
assert(cliargs::type_traits<std::vector<MyStruct>>::name() == "vector<{string, float[, long]}>");

###### 2.2.1.2 Overload `__parse_by_format`
After overloading this function, the program user needs to use **a single structured string** to assign a value to the structure. [Complete example](examples/my_struct_parse_by_format.cpp)
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
> Note: `cliargs` will automatically deduce the type name of `MyStruct`:
> assert(cliargs::type_traits<MyStruct>::name() == "{string, float[, long]}");
> assert(cliargs::type_traits<std::vector<MyStruct>>::name() == "vector<{string, float[, long]}>");

##### 2.2.2 Combining structures with STL containers
|layout|c++ type|__parse_by|usage|
|------|--------|------------|-----|
|single|`MyStruct`|`parser`|`--arg_name name gain [size]`
| | |`format`|`--arg_name name,gain[,size]`<br>`--arg_name=name,gain[,size]`
|vector|`std::vector<MyStruct>`|`parser`|`--arg_name name1 gain1 [size1] --arg_name name2 gain2 [size2] ...`|
| | |`format`|`--arg_name name,gain[,size] --arg_name=name,gain[,size] ...`
|map |`std::map<scalar, MyStruct>`|`parser`|`--arg_name key1 name1 gain1 [size1] --arg_name key2 name2 gain2 [size2] ...`|
| | |`format`|`--arg_name key1 name,gain[,size] --arg_name key 2 name,gain[,size] ...`


#### 2.3 Supports generalized enumeration types
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
        ->ranges({{10, 20}, {30, 50}}) ->examine([](int &v) ->bool { return v % 2; }, "an odd number")
        // the value can only be one of {1, 3, 5} or in range [10, 20] or [30, 50] and be an odd number
    );
```

##### 2.3.1 Combination rules of various constraints
The parser will judge whether the parameters specified by the user meet the requirements according to this order and formula: 
`(choices || ranges || regex) && examine`

##### 2.3.2 examine
Applicable to all data types (including `tuple` and `struct`). Program developers can register custom callback functions through this interface to check (or modify) a single value. If the callback function returns `false`, it is considered that the parameter value provided by the program user does not meet the requirements.
```c++
examine(std::function<bool(T &value)> func, std::string desc = "");
examine(std::function<bool(T &value, void *context)> func, std::string desc = "");
examine(std::function<bool(T &value, void *context, void *data)> func, std::string desc = "");
```

##### 2.3.3 choices
Only applicable to single-value types (integers, floating-point numbers, and strings) and their derived composite types. Other types do not have this interface. Program developers can use this interface to set a set of determined enumeration values.
```c++
choices(std::unordered_set<T> value_set, std::string desc = "");
```

##### 2.3.4 ranges
Only applicable to numeric types (integers and floating-point numbers). Other types do not have this interface. Program developers can use this interface to set the value range(s) of the value. This interface can be called multiple times to specify multiple ranges.
```c++
range(T min_value, T max_value, std::string desc = "");
ranges(std::vector<std::pair<T, T>> pairs, std::string desc = "");
```

##### 2.3.5 regex
Only applicable to string type and its derived composite types. Other types do not have this interface. Program developers can use this interface to set a regular expression.
```c++
regex(std::string regex_string, std::string desc = "");
```

#### 2.4 Support default values ​​and implicit values
##### 2.4.1 Default value
```c++
default_value(T &&data);
```
For command line parameters that provide default values, when the user does not set them, the default parameter value will be used.

##### 2.4.2 Implicit value
```c++
implicit_value(typename T::value_type value);
```
For command line parameters that provide implicit values, if the number of parameter values ​​provided by the user is less than the number of parameter values ​​in the implicit value, the parser will automatically use the latter part of the implicit value to fill the command line parameters until the upper limit of the number of parameter values ​​of the command line parameters is reached.
|case|user action|result|
|----|-----------|------|
|`cliargs::value<int>()`<br>`->default_value(1)`<br>`->implicit_value(5)`|not specified |1
| |`--arg_name` |5
| |`--arg_name 6`|6
|`cliargs::value<vector<int>>()`<br>`->data_count(1, 3)`<br>`->implicit_value({0, 1, 2, 3})`|not specified|error
| |`--arg_name`|error
| |`--arg_name 1`|`[1, 1, 2]`
| |`--arg_name 1 2`|`[1, 2, 2]`
| |`--arg_name 1 2 3`|`[1, 2, 3]`
| |`--arg_name 1 2 3 4`|error or `4` is accepted by positional argument

#### 2.5 Support limiting the size range of containers
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

## 3. Summary of command line parameter attribute interfaces
The attribute interface includes "[common attribute interface](#common attribute interface)", "[metadata type specific attribute interface](#metadata type specific attribute interface)" and "[container type specific attribute interface](#container type specific attribute interface)". The final attribute interface of the combined type is the **union of the three**.

> For detailed examples of combined usage, please refer to the unit test

#### 3.1 General attribute interface
|attribue     |
|-------------|
|required     |
|positional   |
|default_value|
|examine      |
|hide         |

#### 3.2 Metadata type-specific attribute interface
|meta type|attributes|
|---------|----------|
|numerical|choices, ranges, range
|string |choices, regex
|struct |

#### 3.3 Container type-specific attribute interface
| |attribute|
|------|---------|
|scalar|implicit_value
|vector|implicit_value, data_count
|matrix|implicit_value, data_count, line_width
|map |implicit_value, line_width
|tuple |implicit_value, line_width

#### 3.4 `default_value` and `implicit_value` data types
`default_value` The data type is the same as the data type of the command line parameter

The data type of `implicit_value` is as follows:

|argument |implicit_value|
|--------------------------|--------------|
|`scalar` |`scalar`
|`vector<scalar>` |`vector<scalar>`
|`vector<vector<scalar>>` |`vector<scalar>`
|`map<key, scalar>` |`scalar`
|`map<key, vector<scalar>>`|`vector<scalar>`
|`tuple<scalar...>` |`tuple<scalar...>`

#### 3.5 `cliargs::Parser`'s attributes
|attribute|description|
|---------|-----------|
|`allow_unknown`|if user specifies an undefined argument name, no error will occur
|`set_width`    |set the line width for help messages
|`concise_help` |use concise help messages
|`sensitive_mode`|enable [sensitive mode](#sensitive mode)

### 4. Others
#### 4.1 Reverse Boolean Argument
```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ('h', "help", "Normal boolean arguemnt")
    ('r', "reverse_bool", "Reverse boolean argument",
        cliargs::value()->implicit_value(false)
    );
```
The attribute of a normal boolean parameter is: `cliargs::value()->implicit_value(true)`

#### 4.2 Please use pointer types with caution
```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ("ptr", "char *", cliargs::value<char *>())
    ("const_ptr", "const char *", cliargs::AsArgs<const char *>());
```
* If you declare your own data type as a pointer type (`char *` or `const char *`), or combine a pointer type with an STL container, or embed a pointer in a `tuple`, the pointer will directly point to the string passed to the `main` function by the program user
* If the pointer type is embedded in the structure and `__parse_by_parser` is used to assign values ​​to the structure members, the pointer type will point to the string passed to the `main` function by the program user
* If the pointer type is embedded in the structure and `__parse_by_format` is used to assign values ​​to the structure members, **the program developer must ensure the validity of the pointer**

#### 4.3 String Serialization Tool
```c++
template <typename T>
std::string cliargs::to_string(const T &value
    , const std::string &delimiter = ",", const std::string &gap = " ");
```
Composite data types can be converted into strings.
* Example:
```c++
std::map<std::string, std::tuple<int, std::vector<float>>> data = {
    {"key1": {1, {2.5, 3.5}}},
    {"key2": {2, {4.5, 5.5}}},
};
std::cout << cliargs::to_string(data) << std::endl;
```
* Output:
```log
{"key1": (1, [2.5, 3.5]), "key2": (2, [4.5, 5.5])}
```

#### 4.4 Tool to get type name
```c++
template <typename T>
const std::string &cliargs::type_traits<T>::name();
```
You can get the string name of the composite data type. For custom structure types, you need to overload `__parse_by_parser` or `__parse_by_format` first.
* Example:
```c++
typedef std::map<std::string, std::tuple<int, std::vector<float>>> MyType;
std::cout << cliargs::type_traits<MyType>::name() << std::endl;
```
* Output:
```log
map<string, tuple<int, vector<float>>>
```

#### 4.5 sensitive mode
In this mode:
1. When the command parameter type is a string, `cliprgs` will treat the string (excluding negative numbers) starting with a minus sign (`-`) as the command line parameter name
2. If you need to input a string starting with a minus sign (`-`) as a string type command-line parameter value, please add a back slash before the minus sign. `cliargs` will automatically remove a back slash at the beginning of the command line parameter value
3. **In this mode, it may violate GNU conventions in some cases**
