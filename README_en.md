# Introduction

cliargs (Command Line Interface Arguments Parser)

This is a command-line argument parser for C++ programs. It supports standard GNU command-line syntax; this library is header-only.

It is functionally similar to (and pays homage to): getopts (GNU C / Linux Shell), cxxopts (C++), and argparse (Python). However, it offers even more powerful features.

# 1. Basic Features

## 1.1 Support for Standard GNU Command-Line Syntax

Command-line arguments can be specified as follows:

```sh
--long1
--long2=value
--long2 value
-a
-ab
-abc value
```

In this example, long2 and c require argument values, while long1, a, and b do not.

## 1.2 Quick Start

examples/simple.cpp

```cpp
#include "cliargs.hpp"

int main(int argc, char *argv[]) {
    //  Create a 'cliargs::Parser' instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");
    // Define arguments
    parser.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ('i', "int", "An interger", cliargs::value<int>()->default_value(-1))
        ('s', "string", "A string", cliargs::value<std::string>()->default_value("/dev/mem"), "str")
        ('v', "vector", "An int vector", cliargs::value<std::vector<float>>())
        ;
    // Parse
    if (result.error() || result["help"].as<bool>()) {
        result.print_help();
        return result.error() ? -1 : 0;
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
        --str    same as '-s, --string'
    -v, --vector An int vector
                      value: 'vector<float>'[0~N]
```
```sh
./simple -i=32 -s hello -v 1.5 2.5 3.5

   int: 32
string: "hello"
vector: [1.5, 2.5, 3.5]
```

# 2. Advanced Features

## 2.1 Support for Standard C++ Types and Partial STL Containers

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

## 2.2 Support for Custom Struct Types and Derived Types Combined with STL Containers

```c++
struct MyStruct {
    std::string name;
    float gain;
    long size;
};
```

### 2.2.1 Overloading Parse Functions `cliargs_parse_custom`

After overloading this function, program users need to use a string array to assign values to the struct. [Full Example](examples/custom.cpp)

```c++
void cliargs_parse_custom(MyStruct &obj, cliargs::ArgParser &parser) {
    parser.domain_begin("MyStruct"); // Tell ArgParser the struct's name
    if (parser.assign(obj.name, "name")) { // MyStruct::name requires a string value
        parser.check(!obj.name.empty(), "invalid name: empty");
    }
    parser.assign(obj.gain, "gain"); // MyStruct::gain requires a uint64 value
    parser.set_optional(); // The following member is optional
    parser.assign(obj.size, "size", (long)0); // Specify a default value for the optional member
    parser.domain_end();
}
```
```bash
--my_struct data.bin 32 64
--my_struct data.bin 32
```

> Note: cliargs will automatically deduce the type name of MyStruct:
> `assert(cliargs::type_traits<MyStruct>::name() == "{string, float[, long]}");`
> `assert(cliargs::type_traits<std::vector<MyStruct>>::name() == "vector<{string, float[, long]}>");`

### 2.2.2 Combining Structs with STL Containers

|layout|c++ type|__parse_by|usage|
|------|--------|------------|-----|
|single|`MyStruct`|`parser`|`--arg_name name gain [size]`
|      |          |`format`|`--arg_name name,gain[,size]`<br>`--arg_name=name,gain[,size]`
|vector|`std::vector<MyStruct>`|`parser`|`--arg_name name1 gain1 [size1] --arg_name name2 gain2 [size2] ...`|
|      |                       |`format`|`--arg_name name,gain[,size] --arg_name=name,gain[,size] ...`
|map   |`std::map<scalar, MyStruct>`|`parser`|`--arg_name key1 name1 gain1 [size1] --arg_name key2 name2 gain2 [size2] ...`|
|      |                            |`format`|`--arg_name key1 name,gain[,size] --arg_name key 2 name,gain[,size] ...`

## 2.3 Support for Generalized Enum Types

```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ('s', "string", "A string argument which can only be one of {'file', 'directory'}",
        cliargs::value<std::string>()
        ->choices({"dump", "load"})
        ->regex("\\d+", "an integer")
        // The value can only be one of {"dump", "load"} or an integer
    )
    ('i', "integer", "An integer argument which can only be one of {1, 3, 5}",
        cliargs::value<std::vector<int>>()
        ->choices({1, 3, 5})
        ->ranges({{10, 20}, {30, 50}})
        ->examine([](int &v) ->bool { return v % 2; }, "an odd number")
        // The value can only be one of {1, 3, 5} or within the range [10, 20] or [30, 50], and must be an odd number
    );
```

### 2.3.1 Combination Rules for Various Constraints

The parser will evaluate whether the user-specified arguments meet the requirements in the following order and formula:

`(choices || ranges || regex) && examine`

### 2.3.2 examine

Applicable to all data types (including tuple and struct). Developers can use this interface to register custom callback functions to check (or modify) individual values. If the callback function returns false, the argument value provided by the user is considered invalid.

```c++
examine(std::function<bool(T &value)> func, std::string desc = "");
examine(std::function<bool(T &value, void *context)> func, std::string desc = "");
examine(std::function<bool(T &value, void *context, void *data)> func, std::string desc = "");
```

### 2.3.3 choices

Applicable only to single-value types (integers, floating-point numbers, and strings) and their derived composite types. This interface is not available for other types. Developers can use this interface to set a set of definite enumerated values.

```c++
choices(std::vector<T> value_set, std::string desc = "");
```

### 2.3.4 ranges

Applicable only to numeric types (integers and floating-point numbers). This interface is not available for other types. Developers can use this interface to set value ranges. This interface can be called multiple times, and multiple ranges can be specified per call.

```c++
range(T min_value, T max_value, std::string desc = "");
ranges(std::vector<std::pair<T, T>> pairs, std::string desc = "");
```

### 2.3.5 regex

Applicable only to string types and their derived composite types. This interface is not available for other types. Developers can use this interface to set a regular expression.

```c++
regex(std::string regex_string, std::string desc = "");
```

## 2.4 Support for Default Values and Implicit Values

### 2.4.1 Default Values

```c++
default_value(T &&data);
```

For command-line arguments with a provided default value, the default value will be used if the user does not specify one.

### 2.4.2 Implicit Values

```c++
implicit_value(typename T::value_type value);
```

For command-line arguments with a provided implicit value, if the number of argument values provided by the user is less than the number of implicit values, the parser will automatically use the remaining implicit values to fill the command-line argument, up to the maximum limit of argument values.

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

## 2.5 Support for Restricting Container Size Ranges

```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ('v', "vector", "A size-limited vector",
        cliargs::value<std::vector<float>>()
        ->data_count(3, 5) // The vector can only accept 3 to 5 elements
        ->implicit_value({0, 0, 0, 44, 55})
    )
    ('m', "matrix", "A size-limited matrix",
        cliargs::value<std::vector<std::vector<int>>>()
        ->data_count(3, 5) // The matrix's height is 3 to 5
        ->line_width(2, 2) // The matrix's width is 2
    )
    ('t', "tuple", "A tuple with a default tail",
        cliargs::value<std::tuple<std::string, int, float>>()
        ->line_width(2) // tuple[0] and tuple[1] are required, and tuple[2] is optional
        ->implicit_value({"", 0, 1.0f})
    );
```

## 2.6 Support for Argument Nesting

### 2.6.1 Default GNU-Style Argument Nesting

All strings located after `--`, where the first string is not the name of a command-line argument with the stop_at_eof() attribute, will be placed into the Tail object returned by `cliargs::Result::tail()`.

```c++
cliargs::Parser parser("MyProgram", "One line description of MyProgram");
...
auto result = parser.parse(argc, argv);
...
auto &tail = result.tail();
```

### 2.6.2 Extended Nested Arguments

Setting the stop_at_eof attribute for a command-line argument whose data type is `std::vector<char *>` or `std::vector<std::string>` will allow it to consume all subsequent strings until it encounters the terminator (`--`). If you need to pass the terminator (`--`) as an actual input value, simply prefix it with a backslash (`\\`; note that in both shell and C++, you need to type two backslashes).

# 3. Summary of Command-Line Argument Attribute Interfaces

Attribute interfaces include the "General Attribute Interfaces", "Meta-Type-Specific Attribute Interfaces", and "Container-Type-Specific Attribute Interfaces". The attribute interface for a final composite type is the union of the three.

> For detailed examples of combined usage, please refer to the unit tests.

## 3.1 General Attribute Interfaces

|Attribute    |Description|
|-------------|-----------|
|required     |The argument must be set.
|positional   |A positional argument; the argument name can be omitted when setting it.
|default_value|Sets the default value of the argument.
|examine      |Sets a hook function to check or transform the argument value.
|hide         |Hides this argument's information from the help message.
|concise_help |Suppresses automatically generated content in the help message.

## 3.2 Meta-Type-Specific Attribute Interfaces

|Meta Type|Attributes|
|---------|----------|
|numerical|choices, ranges, range
|string   |choices, regex, sensitive_mode, stop_at_eof
|tuple    |sensitive_mode, stop_at_eof
|struct   |sensitive_mode, stop_at_eof

## 3.3 Container-Type-Specific Attribute Interfaces

|Type  |Attribute|
|------|---------|
|scalar|implicit_value
|vector|implicit_value, data_count
|matrix|implicit_value, data_count, line_width
|map   |implicit_value, line_width
|tuple |implicit_value, line_width

## 3.4 Data Types of default_value and implicit_value

* The data type of `default_value` is identical to the data type of the command-line argument.
* The data type of `implicit_value` is as follows:

|argument                  |implicit_value|
|--------------------------|--------------|
|`scalar`                  |`scalar`
|`vector<scalar>`          |`vector<scalar>`
|`vector<vector<scalar>>`  |`vector<scalar>`
|`map<key, scalar>`        |`scalar`
|`map<key, vector<scalar>>`|`vector<scalar>`
|`tuple<scalar...>`        |`tuple<scalar...>`

## 3.5 Attributes of cliargs::Parser

|Attribute     |Description|
|--------------|-----------|
|allow_unknown |Does not report an error when the user specifies an undefined command-line argument name.
|set_width     |Sets the maximum number of characters displayed per line in the help message.
|concise_help  |Uses concise help messages.
|sensitive_mode|Enables "Sensitive Mode".

## 3.6 cliargs::add_args()

|method|
|------|
|`(char flag, std::string name, std::string desc, std::string alias="")`
|`(std::string name, std::string desc, std::string alias="")`
|`(char flag, std::string name, std::string desc, std::shared_ptr<ArgAttr<T>> attr, std::string alias="")`
|`(std::string name, std::string desc, std::shared_ptr<ArgAttr<T>> attr, std::string alias="")`

## 3.7 cliargs::Result::tail()

All strings located after --, provided that the first string is not the name of a command-line argument with the stop_at_eof() attribute, will be placed into the Tail object returned by cliargs::Result::tail().

```c++
struct cliargs::Result::Tail {
    int argc;
    char **argv;
};
```

# 4. Miscellaneous

## 4.1 Reverse Boolean Arguments

```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ('h', "help", "Normal boolean argument")
    ('r', "reverse_bool", "Reverse boolean argument",
        cliargs::value()->implicit_value(false)
    );
```

The attribute for a normal boolean argument is: cliargs::value()->implicit_value(true).

## 4.2 Use Pointer Types with Caution

```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    ("ptr", "char *", cliargs::value<char *>())
    ("const_ptr", "const char *", cliargs::AsArgs<const char *>());
```

* If you declare your data type as a pointer type (char * or const char *), combine pointer types with STL containers, or embed pointers within a tuple, the pointer will directly point to the string passed by the program user to the main function.
* If a pointer type is embedded within a struct and assigned using cliargs_parse_custom, the pointer will point to the string passed by the program user to the main function.

## 4.3 About GNU Mode

In the default mode:

1. When a command-line argument is of the string type, cliargs will treat any string starting with a hyphen (-) (except for negative numbers) as a command-line argument name.
2. If you need to input a string starting with a hyphen (-) as a string-type argument value, please add a backslash (\\; note that in both shell and C++, you need to type two backslashes) before the hyphen. cliargs will automatically strip the leading backslash (\) from the argument value.

In GNU mode:

cliargs will forcibly treat one or more tokens following an argument name as its argument value, regardless of whether those strings start with a hyphen (-).
