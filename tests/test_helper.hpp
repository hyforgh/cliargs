#include "cliargs.hpp"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

class Argv {
public:
    Argv(const std::vector<std::string> &strings) {
        _data.resize(strings.size());
        _argv.resize(_data.size());
        for (size_t i = 0; i < strings.size(); ++i) {
            auto &str = strings[i];
            _data[i] = std::move(std::vector<char>(str.c_str(), str.c_str() + str.length() + 1));
            _argv[i] = _data[i].data();
        }
    }
    int argc() const {
        return _argv.size();
    }
    char **argv() {
        return _argv.data();
    }
private:
    std::vector<std::vector<char>> _data;
    std::vector<char *> _argv;
}; // class Argv

bool cli_error_like(const std::list<std::string> &err_list, const std::string &regex_string) {
    auto reg = std::regex(regex_string);
    for (auto &it : err_list) {
        if (std::regex_match(it, reg)) {
            return true;
        }
    }
    return false;
}

#define PP_REMOVE_PARENS(T) PP_REMOVE_PARENS_IMPL T
#define PP_REMOVE_PARENS_IMPL(...) __VA_ARGS__

#define CLI_TEST_DEFINE_NORM_ARG(data_type, attr, ...)                  \
    cliargs::Parser parser;                                             \
    parser.add_args()                                                   \
        ("arg_name", "", cliargs::value<PP_REMOVE_PARENS(data_type)>() \
            PP_REMOVE_PARENS(attr));                                    \
    Argv argv({"cli_test", __VA_ARGS__});                               \
    auto result = parser.parse(argv.argc(), argv.argv());               \
    auto &arg_value = result["arg_name"];                               \
    (void)arg_value;
