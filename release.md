## 1.5.0
* new feature
    1. cliargs::value<>()->sensitive_mode()
    2. cliargs::value<>()->concise_help()

* fix bug
    1. fix bugs of positional arguments with string type

## 1.4.0
* new feature
    1. support nesting (`cliargs::Result::tail()` and `stop_at_eof()`)

* fix bug
    1. fix bugs casued by `--`

## 1.3.0
* new feature
    1. argument name alias

* fix bug
    1. fix positional arguments with negative value

## 1.2.0
* new feature
    1. sensitive mode
    2. hide some arguments in help message
    3. new attribute `range(T min_value, T max_value, ...)`

* fix bug
    1. binary number fails in same environment

## 1.1.0
* new feature
    1. support binary number
    2. examine bool type

* fix bug
    1. default_value & implicit_value support lvalue;
    2. help message

## 1.0.0
Initial
