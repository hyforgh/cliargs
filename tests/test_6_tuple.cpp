#include "test_helper.hpp"

TEST_CASE("vector_numeric") {
    SECTION("single-full") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), ()
            , "--arg_name", "name", "5.12", "2");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::tuple<std::string, float, int>>() == std::tuple<std::string, float, int>{"name", 5.12, 2});
    }

    SECTION("single-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (->line_width(2))
            , "--arg_name", "name", "5.12");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::tuple<std::string, float, int>>() == std::tuple<std::string, float, int>{"name", 5.12, 0});
    }

    SECTION("single-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (->line_width(2))
            , "--arg_name", "name");
        CHECK(result.error());
        CHECK(cli_error_like(result.error_details(),
            ".*a\\(n\\) 'float32' value is required as 'tuple.*<1>'"));
    }

    SECTION("single-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (->line_width(2))
            , "--arg_name", "name", "5.12", "2", "9");
        CHECK(result.error());
        CHECK(cli_error_like(result.error_details(),
            ".*too many value '9'"));
    }

    SECTION("single-default-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (->default_value({"name", 5.12, 2})));
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::tuple<std::string, float, int>>() == std::tuple<std::string, float, int>{"name", 5.12, 2});
    }

    SECTION("single-default-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (->default_value({"name", 5.12, 2}))
            , "--arg_name", "data", "4.9", "3");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::tuple<std::string, float, int>>() == std::tuple<std::string, float, int>{"data", 4.9, 3});
    }

    SECTION("single-implicit-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (->implicit_value({"name", 5.12, 2}))
            , "--arg_name", "data", "4.9");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::tuple<std::string, float, int>>() == std::tuple<std::string, float, int>{"data", 4.9, 2});
    }

    SECTION("single-implicit-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (->implicit_value({"name", 5.12, 2}))
            , "--arg_name", "data", "4.9", "3");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::tuple<std::string, float, int>>() == std::tuple<std::string, float, int>{"data", 4.9, 3});
    }

    SECTION("single-examine-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (
                ->examine([](std::tuple<std::string, float, int> &v) -> bool { return !std::get<0>(v).empty(); }, "<0> not empty"))
            , "--arg_name", "name", "5.12", "2");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::tuple<std::string, float, int>>() == std::tuple<std::string, float, int>{"name", 5.12, 2});
    }

    SECTION("single-examine-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::tuple<std::string, float, int>), (
                ->examine([](std::tuple<std::string, float, int> &v) -> bool { return !std::get<0>(v).empty(); }, "<0> not empty"))
            , "--arg_name", "", "5.12", "2");
        CHECK(result.error());
        CHECK(cli_error_like(result.error_details(),
            ".*should meet constraint: '<0> not empty'"));
    }

    SECTION("vector-full") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<std::tuple<std::string, float, int>>), ()
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<std::tuple<std::string, float, int>>>();
        CHECK(av[0] == std::tuple<std::string, float, int>{"data", 1.5, 2});
        CHECK(av[1] == std::tuple<std::string, float, int>{"bin", 2.5, 3});
    }

    SECTION("vector-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<std::tuple<std::string, float, int>>)
            , (->line_width(2)->implicit_value({"name", 2, -1}))
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<std::tuple<std::string, float, int>>>();
        CHECK(av[0] == std::tuple<std::string, float, int>{"data", 1.5, 2});
        CHECK(av[1] == std::tuple<std::string, float, int>{"bin", 2.5, -1});
    }

    SECTION("vector-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<std::tuple<std::string, float, int>>), (->line_width(2))
            , "--arg_name", "data", "1.5", "2", "--arg_name", "bin");
        CHECK(result.error());
        std::cout << cliargs::to_string(arg_value.as<std::vector<std::tuple<std::string, float, int>>>()) << std::endl;
        CHECK(cli_error_like(result.error_details(),
            ".*a\\(n\\) 'float32' value is required as 'vector.*\\[1\\]<1>'"));
    }

    SECTION("vector-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<std::tuple<std::string, float, int>>), ()
            , "--arg_name", "data", "1.5", "2" "--arg_name", "bin", "2.5", "3", "9");
        CHECK(result.error());
        CHECK(cli_error_like(result.error_details(),
            ".*too many value '9'"));
    }

    SECTION("vector-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<std::tuple<std::string, float, int>>)
            , (->line_width(2)->implicit_value({"name", 2, -1}))
            , "--arg_name", "data", "1.5", "--arg_name", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::vector<std::tuple<std::string, float, int>>>();
        CHECK(av[0] == std::tuple<std::string, float, int>{"data", 1.5, -1});
        CHECK(av[1] == std::tuple<std::string, float, int>{"bin", 2.5, -1});
    }

    SECTION("map-full") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, std::tuple<std::string, float, int>>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, std::tuple<std::string, float, int>>>();
        CHECK(av.at("key1") == std::tuple<std::string, float, int>{"data", 1.5, 2});
        CHECK(av.at("key2") == std::tuple<std::string, float, int>{"bin", 2.5, 3});
    }

    SECTION("map-optional") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, std::tuple<std::string, float, int>>)
            , (->line_width(2)->implicit_value({"name", 2, -1}))
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, std::tuple<std::string, float, int>>>();
        CHECK(av.at("key1") == std::tuple<std::string, float, int>{"data", 1.5, 2});
        CHECK(av.at("key2") == std::tuple<std::string, float, int>{"bin", 2.5, -1});
    }

    SECTION("map-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, std::tuple<std::string, float, int>>)
            , (->line_width(2)->implicit_value({"name", 2, -1}))
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin");
        CHECK(result.error());
        std::cout << cliargs::to_string(arg_value.as<std::map<std::string, std::tuple<std::string, float, int>>>()) << std::endl;
        CHECK(cli_error_like(result.error_details(),
            ".*a\\(n\\) 'float32' value is required as 'map.*\\[\"key2\"\\]<1>'"));
    }

    SECTION("map-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, std::tuple<std::string, float, int>>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key2", "bin", "2.5", "3", "9");
        CHECK(result.error());
        CHECK(cli_error_like(result.error_details(),
            ".*too many value '9'"));
    }

    SECTION("map-vector") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, std::vector<std::tuple<std::string, float, int>>>), ()
            , "--arg_name", "key1", "data", "1.5", "2", "--arg_name", "key1", "bin", "2.5", "3");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<std::map<std::string, std::vector<std::tuple<std::string, float, int>>>>();
        CHECK(av.at("key1").at(0) == std::tuple<std::string, float, int>{"data", 1.5, 2});
        CHECK(av.at("key1").at(1) == std::tuple<std::string, float, int>{"bin", 2.5, 3});
    }
}
