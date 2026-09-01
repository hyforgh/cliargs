# cliargs (Command Line Interface Arguments Parser)

`cliargs` is a lightweight and powerful C++ command-line argument parsing library. It is a header-only library that supports standard GNU command-line argument syntax.

This project is inspired by and pays homage to **getopts** (GNU C/Linux Shell), **cxxopts** (C++), and **argparse** (Python). However, it offers more powerful container support and also supports custom data types.

---

## 🚀 1. Quick Start

### Basic Features

Supports standard GNU-style argument syntax:

*   `--long_option`
*   `--long_option=value`
*   `-s` (short option)
*   `-abc` (combined short options)

### Code Example

This is a simple getting-started example demonstrating how to define arguments, parse them, and retrieve the results.

```cpp
#include "cliargs.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    // 1. Create a parser instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");

    // 2. Define arguments
    parser.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // boolean argument
        ('i', "int", "An integer", cliargs::value<int>()->default_value(-1))
        ('s', "string", "A string", cliargs::value<std::string>()->default_value("/dev/mem"), "str")
        ('v', "vector", "An int vector", cliargs::value<std::vector<float>>());

    // 3. Parse arguments
    auto result = parser.parse(argc, argv);

    // 4. Handle help or errors
    if (result.error() || result["help"].as<bool>()) {
        result.print_help();
        return result.error() ? -1 : 0;
    }

    // 5. Use the results
    auto int_value = result["int"].as<int>();
    std::cout << "   int: " << cliargs::to_string(int_value) << std::endl;

    auto &str_value = result["string"].as<std::string>();
    std::cout << "string: " << cliargs::to_string(str_value) << std::endl;

    auto &vec_value = result["vector"].as<std::vector<float>>();
    std::cout << "vector: " << cliargs::to_string(vec_value) << std::endl;

    return 0;
}

```

Output Examples:

*   View help information

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

*   Parse arguments

```bash
./simple -i=32 -s hello -v 1.5 2.5 3.5

   int: 32
string: "hello"
vector: [1.5, 2.5, 3.5]

```

## 🛠️ 2. Advanced Features

## 2.1 Rich Type Support

| Layout | C++ Type | Usage |
|--------|----------|-------|
| scalar | `char`, `short`, `int`, `long`, `long long`<br>`unsigned ...`<br>`float`, `double`, `bool`<br>`char *`, `const char *`, `std::string` | `--arg_name arg_value`<br>`--arg_name=arg_value`<br>`--arg_name` |
| vector | `std::vector<scalar>` | `--arg_name v1 v2 v3 ...`<br>`--arg_name v1 v2 --arg_name v3 ...` |
| matrix | `std::vector<std::vector<scalar>>` | `--arg_name v00 v01 ... --arg_name v10 v11 ...` |
| map    | `std::map<scalar, scalar>`<br>`std::unordered_map<...>` | `--arg_name key1 v1 --arg_name key2 v2` |
|        | `std::map<scalar, std::vector<scalar>>`<br>`std::unordered_map<...>` | `--arg_name key1 k1v1 k1v2 ... --arg_name key2 k2v1 k2v2 ...` |
| tuple  | `std::tuple<scalar...>` | `--arg_name v1 v2 ...` |
|        | `std::vector<std::tuple<scalar...>>` | `--arg_name v00 v01 ... --arg_name v10 v11 ...` |
|        | `std::map<scalar, std::tuple<scalar...>>`<br>`std::unordered_map<...>` | `--arg_name key1 k1v1 k1v2 ... --arg_name key2 k2v1 k2v2 ...` |

> Note: Containers support up to two levels of nesting, e.g., `std::map<std::string, std::vector<int>>`, `std::vector<std::vector<int>>`.

## 2.2 Custom Struct Support

You can easily integrate custom structs into argument parsing.

*   Define the struct

```c++
struct MyStruct {
    std::string name;
    float gain;
    long size;
};

```

*   Overload the parsing function

By overloading `cliargs_parse_custom`, you can control the parsing logic of the struct. After overloading this function, the program user needs to use a **string array** to assign values to the struct. [Full example](examples/custom.cpp)

```c++
void cliargs_parse_custom(MyStruct &obj, cliargs::ArgParser &parser) {
    parser.domain_begin("MyStruct"); // Set the struct name
    // Required fields
    if (parser.assign(obj.name, "name")) { // MyStruct::name requires a string value
        parser.check(!obj.name.empty(), "invalid name: empty");
    }
    parser.assign(obj.gain, "gain"); // MyStruct::gain requires an uint64 value

    // Optional fields
    parser.set_optional();
    parser.assign(obj.size, "size", (long)0); // Specify a default value for the optional member
    parser.domain_end();
}

```

*   Usage Example

```bash
./custom --my_struct data.bin 32 64
./custom --my_struct data.bin 32

```

> Note: `cliargs` will automatically deduce the type name:
> `assert(cliargs::type_traits<MyStruct>::name() == "MyStruct{string, float [, long]}");`
> `assert(cliargs::type_traits<std::vector<MyStruct>>::name() == "vector<MyStruct{string, float [, long]}>");`

*   Combining Structs with Containers

| Layout | C++ Type | Usage |
|--------|----------|-------|
| single | `MyStruct` | `--arg_name name gain [size]` |
| vector | `std::vector<MyStruct>` | `--arg_name name1 gain1 [size1]`<br>`--arg_name name2 gain2 [size2]`<br>`...` |
| map    | `std::map<scalar, MyStruct>` | `--arg_name key1 name1 gain1 [size1]`<br>`--arg_name key2 name2 gain2 [size2]`<br>`...` |

## 2.3 Argument Constraints and Validation

`cliargs` provides powerful constraint interfaces, supporting `choices` (enumerations), `ranges`, `regex` (regular expressions), and custom `examine` callbacks.

*   Constraint logic formula:

`(choices || ranges || regex) && examine`

*   Code Example

```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    // String constraint: must be "dump" / "load" or match the regex "\d+"
    ('s', "string", "A string argument which only can be one of {'file', 'directory'})",
        cliargs::value<std::string>()
        ->choices({"dump", "load"})
        ->regex("\\d+", "an integer")
    )

    // Integer constraint: must be 1/3/5, or within [10,20]/[30,50], and must be an odd number
    ('i', "integer", "An integer argument which only can be one of {1, 3, 5}",
        cliargs::value<std::vector<int>>()
        ->choices({1, 3, 5})
        ->ranges({{10, 20}, {30, 50}})
        ->examine([](int &v) ->bool { return v % 2; }, "an odd number")
    );

```

### 2.4 Default Values and Implicit Values

*   **Default Value (`default_value`)**: The value used when the user does not specify the argument.
*   **Implicit Value (`implicit_value`)**: The value filled when the user specifies the argument name but does not provide a value (or provides an insufficient number of values).

Example Scenarios:

| Case | User Action | Result |
|------|-------------|--------|
| `cliargs::value<int>()`<br>`->default_value(1)`<br>`->implicit_value(5)` | (Not specified) | 1 |
|      | `--arg_name`  | 5 |
|      | `--arg_name 6` | 6 |
| `cliargs::value<vector<int>>()`<br>`->data_count(1, 3)`<br>`->implicit_value({0, 1, 2, 3})` | (Not specified) | Error (requires at least one value for this argument) |
|      | `--arg_name`        | Error (requires at least one value for this argument) |
|      | `--arg_name 1`      | `[1, 1, 2]` |
|      | `--arg_name 1 2`    | `[1, 2, 2]` |
|      | `--arg_name 1 2 3`  | `[1, 2, 3]` |
|      | `--arg_name 1 2 3 4` | Error (requires at most 3 values) or `4` is consumed by a positional argument |

## 2.5 Container Size Limits

You can limit the size range of containers (Vector, Matrix, Tuple).

```c++
cliargs::Parser parser("MyApp");
parser.add_args()
    // Vector: limits the number of elements to 3~5
    ('v', "vector", "A size limited vector",
        cliargs::value<std::vector<float>>()
        ->data_count(3, 5)
    )

    // Matrix: limits the number of rows to 3~5, and fixes the number of columns to 2
    ('m', "matrix", "A size limited matrix",
        cliargs::value<std::vector<std::vector<int>>>()
        ->data_count(3, 5)
        ->line_width(2, 2)
    )

    // Tuple: the first two elements are required, the third is optional
    ('t', "tuple", "A tuple with default tail",
        cliargs::value<std::tuple<std::string, int, float>>()
        ->line_width(2)
        ->implicit_value({"", 0, 1.0f})
    );

```

## 2.6 Argument Nesting and Passthrough

Supports argument passthrough similar to `--`, commonly used to pass arguments to child processes.

*   **GNU Style:** Everything after `--` will be treated as `tail` arguments.
*   **Stop-at-EOF:** For arguments of type `std::vector<std::string>`, `std::vector<char *>`, or `std::vector<const char *>`, setting the `stop_at_eof()` attribute will cause it to consume all subsequent arguments until it encounters `--`.

```c++
cliargs::Parser parser("MyProgram", "One line description of MyProgram");
...
auto result = parser.parse(argc, argv);
...
auto &tail = result.tail();

```

# 🛠️ 3. Compile-time Configuration (Configuration Macros)

To adapt to different compilation environments or disable specific features, `cliargs` provides the following macro definitions.

**Note: These macros must be defined BEFORE `#include "cliargs.hpp"`.**

| Macro | Description |
|-------|-------------|
| `CLIARGS_NO_RTTI` | Disables RTTI (Run-Time Type Information). Define this macro if your project is compiled with `-fno-rtti`. |
| `CLIARGS_NO_EXCEPTION` | Disables exception support. Once defined, the library will use `std::abort()` or error codes instead of throwing exceptions. |
| `CLIARGS_NO_WARNING` | Disables compile-time warnings. Used to suppress prompts regarding custom type `type_name`. |
| `CLIARGS_NO_REGEX` | Disables regular expression support. Mainly used to work around known bugs in `std::regex` in older GCC versions (e.g., 4.8/4.9). |

Fallback for `CLIARGS_NO_REGEX`:
If you disable regex support, you can use `ArgAttr::examine()` in conjunction with a third-party regex library (e.g., Boost.Regex) to achieve similar validation functionality.

# 📚 4. API Reference

## 4.1 General Attribute Interfaces

Applicable to arguments of all types.

| Attribute | Description |
|-----------|-------------|
| `required()` | Marks the argument as required. |
| `positional()` | Marks as a positional argument (no argument name needed). |
| `default_value(T)` | Sets the default value: returned when the user does not specify this argument. |
| `implicit_value(...)` | Sets the implicit value: returned when the user specifies only the argument name without a value. |
| `examine(Func)` | Sets a custom validation callback. |
| `hide()` | Hides this argument in the help message. |
| `concise_help()` | Hides auto-generated content (e.g., data type, default value, implicit value, and constraints) in the help message. |

### 4.1.1 Data Types for Implicit Values

```c++
cliargs::value<T1>()->implicit_value(T2);
```

| T1                        | T2 |
|---------------------------|----|
| `scalar`                  | `scalar`
| `vector<scalar>`          | `vector<scalar>`
| `vector<vector<scalar>>`  | `vector<scalar>`
| `map<key, scalar>`        | `scalar`
| `map<key, vector<scalar>>`| `vector<scalar>`
| `tuple<scalar...>`        | `tuple<scalar...>`
| `struct`                  | `struct`

### 4.1.2 examine

Applicable to all data types (including `tuple` and `struct`). Developers can register custom callback functions via this interface to check (or modify) individual values. If the callback returns `false`, the argument value provided by the user is considered invalid.

```c++
examine(std::function<bool(T &value)> func, std::string desc = "");
examine(std::function<bool(T &value, void *context)> func, std::string desc = "");
```

> Parameter Description:
> `context`: User data pointer set via `value<T>()->context(void *ctx)`.
> `desc`: Will be displayed in the help message.

## 4.2 Attributes Specific to Basic Types

| Meta Type | Attributes | Description |
|-----------|------------|-------------|
| numeric   | `choices(vector<T>)` | Sets enumeration values. |
|           | `ranges(vector<pair<T, T>>)` | Sets multiple ranges. |
|           | `range(T, T)` | Sets a single range. |
| string    | `choices(vector<string>)` | Sets enumeration values. |
|           | `regex(string)` | Sets a regular expression. |
|           | `stop_at_eof()` | Consumes all subsequent arguments (including strings starting with `-` or `--`) until `--` is encountered, or uses `--` to terminate the parsing of this argument early. |
| tuple     | `stop_at_eof()` | Takes effect when the last member is of `string` type, with the same effect as above. |
| struct    | `stop_at_eof()` | Takes effect when the last member is of `string` type, with the same effect as above. |

### 4.2.1 choices

Applicable only to single-value types (integers, floating-point numbers, and strings) and their derived composite types. This interface is not available for other types. Developers can use this interface to define a set of discrete enumerated values.

```c++
choices(std::vector<T> value_set, std::string desc = "");
```

### 4.2.2 ranges

Applicable only to numeric types (integers and floating-point numbers). This interface is not available for other types. Developers can use this interface to define valid numeric ranges. This method can be called multiple times, with each invocation specifying one or more ranges.

```c++
range(T min_value, T max_value, std::string desc = "");
ranges(std::vector<std::pair<T, T>> pairs, std::string desc = "");
```

### 4.2.3 regex

Applicable only to string types and their derived composite types. This interface is not available for other types. Developers can use this interface to set a regular expression constraint.

> Note: Be aware of known bugs in std::regex in older versions of GCC (e.g., 4.8/4.9). Use with caution.

```c++
regex(std::string regex_string, std::string desc = "");
```

## 4.3 Container-Specific Attribute Interfaces

|      |attribute |description|
|------|----------|-----------|
|vector|`data_count(int at_least, int at_most = -1)`|Set the lower and upper bounds of the vector length. (at_most = -1 indicates unlimited.)
|matrix|`data_count(int at_least, int at_most = -1)`|Set the lower and upper bounds of the number of rows in the matrix.
|      |`line_width(int at_least, int at_most = -1)`|Set the lower and upper bounds of the number of columns in the matrix.
|map   |`line_width(int at_least, int at_most = -1)`|Only applicable to map<key, vector>. Sets the lower and upper bounds of the vector length.
|tuple |`line_width(int at_least)`|Set the minimum number of elements in the tuple.

## 4.4 Parser Configuration

## 4.4.1 Parser Attribute Configuration

|attribute|description|
|---------|-----------|
|`allow_unknown` |Do not raise an error when the user specifies an undefined command-line argument name.
|`set_width`     |Set the maximum number of characters displayed per line in help messages.
|`concise_help`  |Suppress auto-generated content (such as data types, default values, implicit values, and constraints) in help messages.
|`gnu_mode`      |Enable “GNU Mode”.

### 4.4.2 add_args

|method|
|------|
|`(char flag, std::string name, std::string desc, std::string alias="")`
|`(std::string name, std::string desc, std::string alias="")`
|`(char flag, std::string name, std::string desc, std::shared_ptr<ArgAttr<T>> attr, std::string alias="")`
|`(std::string name, std::string desc, std::shared_ptr<ArgAttr<T>> attr, std::string alias="")`

## 4.5 Result Interface

### 4.5.1 error

Check whether there are any errors in argument definition or the parsing process.

### 4.5.2 print_help

Print help messages and error messages.

### 4.5.3 as

Retrieve the parsed argument value. The function returns a value of type T.

### 4.5.4 tail

All strings appearing after `--`, where the first such string is not the name of a command-line argument with the `stop_at_eof()` attribute, are collected into the Tail object returned by `cliargs::Result::tail()`.

```c++
struct cliargs::Result::Tail {
    int argc;
    char **argv;
};
```

# 💡 5. Additional Notes

## 5.1 Inverted Boolean Arguments

By default, a boolean argument has an implicit value of true. You can create an inverted switch by setting implicit_value(false).

```cpp
parser.add_args()
    ('r', "reverse_bool", "Disable feature", cliargs::value<bool>()->implicit_value(false));
```

## 5.2 Use Pointer Types with Caution

Although `char *` and `const char *` are supported, the parser points directly to the original string memory within `argv`. Do not modify this memory, and note that it becomes invalid after the `main` function returns.

## 5.3 GNU Mode Description

* **Default Mode:** Strings starting with - are treated as new arguments. If you need to pass a string value that starts with -, use the escape character \ (e.g., --name \-abc).

* **GNU Mode:** When enabled, the parser treats everything following an argument name as its value, even if it starts with -.
