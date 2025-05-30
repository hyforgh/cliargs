#include "test_helper.hpp"

typedef std::tuple<std::string, float, int> MyTuple;

TEST_CASE("vector_numeric") {
    SECTION("type-name") {
        CHECK(cliargs::type_traits<MyTuple>::name() == "tuple<string, float, int>");
    }

    SECTION("single-full") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), ()
            , "--arg_name", "name", "5.12", "2");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyTuple>() == MyTuple{"name", 5.12, 2});
    }

    SECTION("single-optional") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (->line_width(2))
            , "--arg_name", "name", "5.12");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyTuple>() == MyTuple{"name", 5.12, 0});
    }

    SECTION("single-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (->line_width(2))
            , "--arg_name", "name");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*a\\(n\\) 'float' value is required as 'tuple<1>'"));
    }

    SECTION("single-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (->line_width(2))
            , "--arg_name", "name", "5.12", "2", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many value '9'"));
    }

    SECTION("single-default-used") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (->default_value({"name", 5.12, 2})));
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyTuple>() == MyTuple{"name", 5.12, 2});
    }

    SECTION("single-default-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (->default_value({"name", 5.12, 2}))
            , "--arg_name", "data", "4.9", "3");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyTuple>() == MyTuple{"data", 4.9, 3});
    }

    SECTION("single-implicit-used") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (->implicit_value({"name", 5.12, 2}))
            , "--arg_name", "data", "4.9");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyTuple>() == MyTuple{"data", 4.9, 2});
    }

    SECTION("single-implicit-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (->implicit_value({"name", 5.12, 2}))
            , "--arg_name", "data", "4.9", "3");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyTuple>() == MyTuple{"data", 4.9, 3});
    }

    SECTION("single-examine-success") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (
                ->examine([](MyTuple &v) -> bool { return !std::get<0>(v).empty(); }, "<0> not empty"))
            , "--arg_name", "name", "5.12", "2");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MyTuple>() == MyTuple{"name", 5.12, 2});
    }

    SECTION("single-examine-faield") {
        CLI_TEST_DEFINE_NORM_ARG((MyTuple), (
                ->examine([](MyTuple &v) -> bool { return !std::get<0>(v).empty(); }, "<0> not empty"))
            , "--arg_name", "", "5.12", "2");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*should meet constraint: '<0> not empty'"));
    }

    SECTION("vector-full") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyTuple>), ()
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<MyTuple>>();
        CHECK(av[0] == MyTuple{"data", 1.5, 2});
        CHECK(av[1] == MyTuple{"bin", 2.5, 3});
    }

    SECTION("vector-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyTuple>)
            , (->line_width(2)->implicit_value({"name", 2, -1}))
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<MyTuple>>();
        CHECK(av[0] == MyTuple{"data", 1.5, 2});
        CHECK(av[1] == MyTuple{"bin", 2.5, -1});
    }

    SECTION("vector-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyTuple>), (->line_width(2))
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin");
        CHECK(parser.error());
        std::cout << cliargs::to_string(arg_value.as<std::vector<MyTuple>>()) << std::endl;
        CHECK(cli_error_like(parser.error_details(),
            ".*a\\(n\\) 'float' value is required as 'vector\\[1\\]<1>'"));
    }

    SECTION("vector-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyTuple>), ()
            , "--arg_name", "data", "1.5", "2" "--arg_name", "bin", "2.5", "3", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many value '9'"));
    }

    SECTION("vector-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<MyTuple>)
            , (->line_width(2)->implicit_value({"name", 2, -1}))
            , "--arg_name", "data", "1.5", "--arg_name", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<MyTuple>>();
        CHECK(av[0] == MyTuple{"data", 1.5, -1});
        CHECK(av[1] == MyTuple{"bin", 2.5, -1});
    }

    SECTION("map-full") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, MyTuple>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, MyTuple>>();
        CHECK(av.at("key1") == MyTuple{"data", 1.5, 2});
        CHECK(av.at("key2") == MyTuple{"bin", 2.5, 3});
    }

    SECTION("map-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, MyTuple>)
            , (->line_width(2)->implicit_value({"name", 2, -1}))
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, MyTuple>>();
        CHECK(av.at("key1") == MyTuple{"data", 1.5, 2});
        CHECK(av.at("key2") == MyTuple{"bin", 2.5, -1});
    }

    SECTION("map-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, MyTuple>)
            , (->line_width(2)->implicit_value({"name", 2, -1}))
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin");
        CHECK(parser.error());
        std::cout << cliargs::to_string(arg_value.as<std::map<std::string, MyTuple>>()) << std::endl;
        CHECK(cli_error_like(parser.error_details(),
            ".*a\\(n\\) 'float' value is required as 'map\\[\"key2\"\\]<1>'"));
    }

    SECTION("map-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, MyTuple>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5", "3", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many value '9'"));
    }

    SECTION("map-vector") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, std::vector<MyTuple>>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key1", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, std::vector<MyTuple>>>();
        CHECK(av.at("key1").at(0) == MyTuple{"data", 1.5, 2});
        CHECK(av.at("key1").at(1) == MyTuple{"bin", 2.5, 3});
    }
}
