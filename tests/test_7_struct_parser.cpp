#include "test_helper.hpp"

struct MyStruct {
    std::string name;
    float gain;
    int flag;
};
std::ostream &operator << (std::ostream &os, const MyStruct &obj) {
    os << "{.name=\"" << obj.name << "\", .gain=" << obj.gain << ", .flag=" << obj.flag << "}";
    return os;
}
void __parse_by_parser(MyStruct &obj, cliargs::ArgParser &parser, const std::string &name) {
    parser.domain_begin(name.empty() ? "MyStruct" : name);
    if (parser.assign(obj.name, "name")) {
        parser.check(!obj.name.empty(), "an empty name");
    }
    parser.assign(obj.gain, "gain");
    parser.set_optional();
    parser.assign(obj.flag, "flag", -1);
    parser.domain_end();
}

bool operator == (const MyStruct &a, const MyStruct &b) {
    return a.name == b.name && a.gain == b.gain && a.flag == b.flag;
}

TEST_CASE("struct_parser") {
    SECTION("type-name") {
        CHECK(cliargs::type_traits<MyStruct>::name() == "{string, float[, int]}");
    }

    SECTION("single-full") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (), "--arg_name", "data", "1.5", "2");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyStruct>() == MyStruct{"data", 1.5, 2});
    }

    SECTION("single-optional") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (), "--arg_name", "data", "1.5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyStruct>() == MyStruct{"data", 1.5, -1});
    }

    SECTION("single-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (), "--arg_name", "data");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*a\\(n\\) 'float' value is required as 'MyStruct\\.gain'"));
    }

    SECTION("single-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (), "--arg_name", "data", "1.5", "2", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many value '9'"));
    }

    SECTION("single-default-used") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (->default_value({"data", 1.5, 2})));
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyStruct>() == MyStruct{"data", 1.5, 2});
    }

    SECTION("single-default-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (->default_value({"bin", 2.5, 3}))
            , "--arg_name", "data", "1.5", "2");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyStruct>() == MyStruct{"data", 1.5, 2});
    }

    SECTION("single-implicit-used") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (->implicit_value({"data", 1.5, 2}))
            , "--arg_name");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyStruct>() == MyStruct{"data", 1.5, 2});
    }

    SECTION("single-implicit-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (->implicit_value({"bin", 2.5, 3}))
            , "--arg_name", "data", "1.5", "2");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyStruct>() == MyStruct{"data", 1.5, 2});
    }

    SECTION("single-examine-success") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (
                ->examine([](MyStruct &v) -> bool { return !v.name.empty(); }, "name not empty"))
            , "--arg_name", "name", "5.12", "2");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyStruct>() == MyStruct{"name", 5.12, 2});
    }

    SECTION("single-examine-faield") {
        CLI_TEST_DEFINE_NORM_ARG((MyStruct), (
                ->examine([](MyStruct &v) -> bool { return !v.name.empty(); }, "name not empty"))
            , "--arg_name", "", "5.12", "2");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*should meet constraint: 'name not empty'"));
    }

    SECTION("vector-full") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyStruct>), ()
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<MyStruct>>();
        CHECK(av[0] == MyStruct{"data", 1.5, 2});
        CHECK(av[1] == MyStruct{"bin", 2.5, 3});
    }

    SECTION("vector-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyStruct>), ()
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<MyStruct>>();
        CHECK(av[0] == MyStruct{"data", 1.5, 2});
        CHECK(av[1] == MyStruct{"bin", 2.5, -1});
    }

    SECTION("vector-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyStruct>), ()
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*a\\(n\\) 'float' value is required as 'vector\\[1\\].gain'"));
    }

    SECTION("vector-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyStruct>), ()
            , "--arg_name", "data", "1.5", "2" "--arg_name", "bin", "2.5", "3", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many value '9'"));
    }

    SECTION("vector-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyStruct>), ()
            , "--arg_name", "data", "1.5", "--arg_name", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<MyStruct>>();
        CHECK(av[0] == MyStruct{"data", 1.5, -1});
        CHECK(av[1] == MyStruct{"bin", 2.5, -1});
    }

    SECTION("map-full") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, MyStruct>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, MyStruct>>();
        CHECK(av.at("key1") == MyStruct{"data", 1.5, 2});
        CHECK(av.at("key2") == MyStruct{"bin", 2.5, 3});
    }

    SECTION("map-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, MyStruct>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, MyStruct>>();
        CHECK(av.at("key1") == MyStruct{"data", 1.5, 2});
        CHECK(av.at("key2") == MyStruct{"bin", 2.5, -1});
    }

    SECTION("map-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, MyStruct>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*a\\(n\\) 'float' value is required as 'map\\[\"key2\"\\]\\.gain'"));
    }

    SECTION("map-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, MyStruct>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5", "3", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many value '9'"));
    }

    SECTION("map-vector") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, std::vector<MyStruct>>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key1", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, std::vector<MyStruct>>>();
        CHECK(av.at("key1").at(0) == MyStruct{"data", 1.5, 2});
        CHECK(av.at("key1").at(1) == MyStruct{"bin", 2.5, 3});
    }

#undef PARSE_NUMERIC
}
