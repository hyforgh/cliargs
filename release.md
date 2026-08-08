# Release Notes

## 3.0.0
**TL;DR: Major API refactor & behavior change.** The `data_count(int fixed_count)` and `line_width(int fixed_count)` method has been removed, and the `data_count(int, int)` and `line_width(int, int)` signature has been updated.

**Breaking Changes**
* **API Removal**: The `data_count(int fixed_count)` and `line_width(int fixed_count)` method has been completely removed.
* **Signature Change**: The `data_count(int at_least, int at_most)` method has been replaced with `data_count(int at_least, int at_most = -1)`. While this adds a default value for convenience, it alters the function signature. This will break existing code that relies on function pointers or strict template deduction matching the old signature.
* **Signature Change**: The `line_width(int at_least, int at_most)` method has been replaced with `line_width(int at_least, int at_most = -1)`. While this adds a default value for convenience, it alters the function signature. This will break existing code that relies on function pointers or strict template deduction matching the old signature.

**Bug Fixes**
* Fixed an issue where integer boundary checks were ineffective.
* Fixed a bug where `implicit_value` caused the lower bound of `line_width` to be ignored.


## 2.0.0
**TL;DR: Major API refactor & behavior change.** The `error()` and `print_help()` methods have been moved from `Parser` to `Result`. The default parsing mode has switched from GNU mode to Smart mode (`sensitive_mode`).

**Breaking Changes**
* **API Migration**: The `error()` and `print_help()` methods have been moved from the `Parser` class to the `Result` class. You should now call `Result::error()` and `Result::print_help()`.
* **Parser Interface Updates**: 
  * `Parser::error()` now requires a `const Result*` parameter.
  * `Parser::print_help()` now requires a `const Result*` parameter and supports additional arguments.
* **Default Mode Change**: The default parsing mode has been changed from GNU mode to Smart mode (`sensitive_mode`).
* **Mode Renaming**: The `Parser::sensitive_mode()` method has been replaced by `Parser::gnu_mode()`.
* **Custom Parsing API**: The internal parsing functions have been renamed to avoid reserved identifiers. Please update your code to use `cliargs_parse_by_format` and `cliargs_parse_by_parser` instead of `__parse_by_format` and `__parse_by_parser`.
* **Removed Attribute**: The `ArgAttr::sensitive_mode()` attribute has been removed.

## 1.5.0
**TL;DR: Granular help & mode control.** Adds per-argument control for sensitive mode and concise help output.

**New Features**
* Added `cliargs::value<>()->sensitive_mode()` to enable sensitive mode for specific arguments.
* Added `cliargs::value<>()->concise_help()` to suppress auto-generated content in help messages.

**Bug Fixes**
* Fixed an issue with positional arguments of the string type.

## 1.4.0
**TL;DR: Argument nesting support.** Introduces `tail()` and `stop_at_eof()` for handling nested command-line structures.

**New Features**
* Added support for argument nesting via `cliargs::Result::tail()` and the `stop_at_eof()` attribute.

**Bug Fixes**
* Fixed parsing bugs caused by the `--` terminator.

## 1.3.0
**TL;DR: Aliases & negative number fixes.** Adds support for argument name aliases and resolves parsing issues with negative values.

**New Features**
* Added support for argument name aliases.

**Bug Fixes**
* Fixed an issue where positional arguments with negative values were incorrectly parsed.

## 1.2.0
**TL;DR: Sensitive mode & value ranges.** Introduces strict parsing (Sensitive Mode), hidden arguments, and numeric range constraints.

**New Features**
* Introduced Sensitive Mode for stricter argument parsing.
* Added the `hide` attribute to hide specific arguments from the help message.
* Added the `range(T min_value, T max_value, ...)` attribute for numeric value constraints.

**Bug Fixes**
* Fixed a bug where binary numbers failed to parse in certain environments.

## 1.1.0
**TL;DR: Binary support & validation.** Adds binary number parsing, boolean validation, and improves value reference handling.

**New Features**
* Added support for parsing binary numbers.
* Added the `examine` attribute for validating boolean types.

**Bug Fixes**
* `default_value` and `implicit_value` now properly support lvalue references.
* Fixed formatting issues in the help message output.

## 1.0.0
**TL;DR: Initial release.** The foundational header-only C++ command-line argument parser.

* Initial public release.
