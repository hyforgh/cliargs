# 更新日志 (Release Notes)

## 3.0.0
**摘要：重大 API 重构与行为变更。** 移除了 `data_count(int fixed_count)`、 `line_width(int fixed_count)`、`cliargs_parse_by_format` 等方法，并更新了 `data_count(int, int)` 与 `line_width(int, int)` 的函数签名。

**破坏性变更 (Breaking Changes)**
* **API 移除**：
  * 彻底移除了 `data_count(int fixed_count)` 和 `line_width(int fixed_count)` 方法。
  * 彻底移除了 `cliargs_parse_by_format` 方法。（当前，暂不再支持通过解析单个字符串给结构体赋值，后续将通过更便捷的形式支持）
* **签名变更**：`data_count(int at_least, int at_most)` 方法已替换为 `data_count(int at_least, int at_most = -1)`。虽然增加了默认值以提供便利，但这改变了函数签名。依赖旧签名的函数指针或严格的模板推导的现有代码将会中断。
* **签名变更**：`line_width(int at_least, int at_most)` 方法已替换为 `line_width(int at_least, int at_most = -1)`。虽然增加了默认值以提供便利，但这改变了函数签名。依赖旧签名的函数指针或严格的模板推导的现有代码将会中断。
* **行为变更**：`cliargs::to_string` 具备自行推断自定义结构体转字符串的能力，不再依赖重载输出流 `sstd::ostream & operator << (std::ostream &, const MyStruct &)`  运算符了

**Bug 修复**
* 修复了整数边界检查失效的问题。
* 修复了 `implicit_value` 导致 `line_width` 下限被忽略的 Bug。


## 2.0.0
**摘要：重大 API 重构与行为变更。** `error()` 和 `print_help()` 方法已从 `Parser` 移至 `Result`。默认解析模式已从 GNU 模式切换为 Smart 模式 (`sensitive_mode`)。标量类型参数允许被重复指定，最后一次指定的值被采纳。

**破坏性变更 (Breaking Changes)**
* **API 迁移**：`error()` 和 `print_help()` 方法已从 `Parser` 类移至 `Result` 类。现在应调用 `Result::error()` 和 `Result::print_help()`。
* **Parser 接口更新**：
  * `Parser::error()` 现在需要传入 `const Result*` 参数。
  * `Parser::print_help()` 现在需要传入 `const Result*` 参数。
* **默认模式变更**：默认解析模式已从 GNU 模式更改为 Smart 模式 (`sensitive_mode`)。
* **模式重命名**：`Parser::sensitive_mode()` 方法已被 `Parser::gnu_mode()` 取代。
* **自定义解析 API**：内部解析函数已重命名以避免使用保留标识符。请将代码中的 `__parse_by_format` 和 `__parse_by_parser` 更新为 `cliargs_parse_by_format` 和 `cliargs_parse_by_parser`。
* **属性移除**：移除了 `ArgAttr::sensitive_mode()` 属性。
* **属性参数类型变更**：`ArgAttr::choices()` 的参数改成 `std::vector` 类型
* **头文件变更**： `cliargs.hpp` 不再包含 `set` 和 `unordered_set` 头文件

## 1.5.0
**摘要：细粒度帮助信息与模式控制。** 新增针对单个参数的敏感模式控制和精简帮助信息输出。

**新特性**
* 新增 `cliargs::value<>()->sensitive_mode()`，用于为特定参数启用敏感模式。
* 新增 `cliargs::value<>()->concise_help()`，用于在帮助信息中抑制自动生成的内容。

**Bug 修复**
* 修复了字符串类型位置参数 (positional arguments) 的相关问题。

## 1.4.0
**摘要：支持参数嵌套。** 引入 `tail()` 和 `stop_at_eof()` 用于处理嵌套的命令行结构。

**新特性**
* 通过 `cliargs::Result::tail()` 和 `stop_at_eof()` 属性，新增对参数嵌套的支持。

**Bug 修复**
* 修复了由 `--` 终止符引起的解析 Bug。

## 1.3.0
**摘要：支持别名与负数修复。** 新增对参数名称别名的支持，并解决了负值解析问题。

**新特性**
* 新增对参数名称别名 (aliases) 的支持。

**Bug 修复**
* 修复了带有负值的位置参数被错误解析的问题。

## 1.2.0
**摘要：敏感模式与值范围。** 引入严格解析（敏感模式）、隐藏参数以及数值范围约束。

**新特性**
* 引入用于更严格参数解析的敏感模式 (Sensitive Mode)。
* 新增 `hide` 属性，用于在帮助信息中隐藏特定参数。
* 新增 `range(T min_value, T max_value, ...)` 属性，用于数值约束。

**Bug 修复**
* 修复了在某些环境下二进制数解析失败的 Bug。

## 1.1.0
**摘要：支持二进制与验证。** 新增二进制数解析、布尔类型验证，并改进了值引用处理。

**新特性**
* 新增对二进制数解析的支持。
* 新增 `examine` 属性，用于验证布尔类型。

**Bug 修复**
* `default_value` 和 `implicit_value` 现已正确支持左值引用 (lvalue references)。
* 修复了帮助信息输出的格式问题。

## 1.0.0
**摘要：初始发布。** 基础的头文件 C++ 命令行参数解析库。

* 首次公开发布。
