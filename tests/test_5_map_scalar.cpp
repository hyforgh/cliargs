#include "test_helper.hpp"

TEST_CASE("scalar_numeric") {
    SECTION("simple") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), ()
            , "--arg_name", "key1", "5", "--arg_name", "key2", "6");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 5);
        CHECK(arg_value.as<std::map<std::string, int>>().at("key2") == 6);
    }

    SECTION("default-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (->default_value({{"key1", 5}, {"key2", 6}})));
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 5);
        CHECK(arg_value.as<std::map<std::string, int>>().at("key2") == 6);
    }

    SECTION("default-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (->default_value({{"key1", 1}, {"key2", 2}}))
            , "--arg_name", "key1", "5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 5);
    }

    SECTION("implicit-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (->implicit_value(5)), "--arg_name", "key1");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 5);
    }

    SECTION("implicit-coverred") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (->implicit_value(2)), "--arg_name", "key1", "5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 5);
    }

    SECTION("choices-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (->choices({1, 3, 5})), "--arg_name", "key1",  "5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 5);
    }

    SECTION("choices-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (->choices({1, 3, 5})), "--arg_name", "key1", "2");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '2', should meet constraint: in-set\\{(1|, |3|, |5){5}\\}"));
    }

    SECTION("ranges-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (->ranges({{10, 20}, {40, 60}})), "--arg_name", "key1", "50");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 50);
    }

    SECTION("ranges-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (->ranges({{10, 20}, {40, 60}})), "--arg_name", "key1", "30");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '30', should meet constraint: within-ranges\\[\\(10, 20\\), \\(40, 60\\)\\]"));
    }

    SECTION("examine-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "key1", "5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 5);
    }

    SECTION("examine-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "key1", "512");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '512', should meet constraint: 'odd number'"));
    }

    SECTION("constraint-success-choices") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (
                ->choices({1, 3, 5})
                ->ranges({{10, 20}, {40, 60}})
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "key1", "5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 5);
    }

    SECTION("constraint-success-ranges") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (
                ->choices({1, 3, 5})
                ->ranges({{10, 20}, {40, 60}})
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "key1", "51");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::map<std::string, int>>().at("key1") == 51);
    }

    SECTION("constraint-failed-choices-and-ranges") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (
                ->choices({1, 3, 5})
                ->ranges({{10, 20}, {40, 60}})
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "key1", "30");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '30', should meet constraint: "
            "\\(in-set\\{(1|, |3|, |5){5}\\} or within-ranges\\[\\(10, 20\\), \\(40, 60\\)\\]\\) and 'odd number'"));
    }

    SECTION("constraint-failed-examine") {
        CLI_TEST_DEFINE_NORM_ARG((std::map<std::string, int>), (
                ->choices({1, 3, 5})
                ->ranges({{10, 20}, {40, 60}})
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "key1", "50");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '50', should meet constraint: "
            "\\(in-set\\{(1|, |3|, |5){5}\\} or within-ranges\\[\\(10, 20\\), \\(40, 60\\)\\]\\) and 'odd number'"));
    }
}
