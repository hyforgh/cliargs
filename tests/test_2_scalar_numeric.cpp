#include "test_helper.hpp"

TEST_CASE("scalar_numeric") {
    SECTION("simple") {
        CLI_TEST_DEFINE_NORM_ARG((int), (), "--arg_name", "5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 5);
    }

    SECTION("simple-negative") {
        CLI_TEST_DEFINE_NORM_ARG((int), (), "--arg_name", "-5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == -5);
    }

    SECTION("default-used") {
        CLI_TEST_DEFINE_NORM_ARG((int), (->default_value(1)));
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 1);
    }

    SECTION("default-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((int), (->default_value(1)), "--arg_name", "5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 5);
    }

    SECTION("implicit-used") {
        CLI_TEST_DEFINE_NORM_ARG((int), (->implicit_value(2)), "--arg_name");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 2);
    }

    SECTION("implicit-coverred") {
        CLI_TEST_DEFINE_NORM_ARG((int), (->implicit_value(2)), "--arg_name", "3");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 3);
    }

    SECTION("choices-success") {
        CLI_TEST_DEFINE_NORM_ARG((int), (->choices({1, 3, 5})), "--arg_name", "3");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 3);
    }

    SECTION("choices-faield") {
        CLI_TEST_DEFINE_NORM_ARG((int), (->choices({1, 3, 5})), "--arg_name", "2");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '2', should meet constraint: in set:\\{(1|, |3|, |5){5}\\}"));
    }

    SECTION("ranges-success") {
        CLI_TEST_DEFINE_NORM_ARG((int), (->ranges({{10, 20}, {40, 60}})), "--arg_name", "50");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 50);
    }

    SECTION("ranges-faield") {
        CLI_TEST_DEFINE_NORM_ARG((int), (->ranges({{10, 20}, {40, 60}})), "--arg_name", "30");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '30', should meet constraint: within ranges:\\[\\(10, 20\\), \\(40, 60\\)\\]"));
    }

    SECTION("examine-success") {
        CLI_TEST_DEFINE_NORM_ARG((int), (
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "511");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 511);
    }

    SECTION("examine-faield") {
        CLI_TEST_DEFINE_NORM_ARG((int), (
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "512");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '512', should meet constraint: 'odd number'"));
    }

    SECTION("constraint-success-choices") {
        CLI_TEST_DEFINE_NORM_ARG((int), (
                ->choices({1, 3, 5})
                ->ranges({{10, 20}, {40, 60}})
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "3");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 3);
    }

    SECTION("constraint-success-ranges") {
        CLI_TEST_DEFINE_NORM_ARG((int), (
                ->choices({1, 3, 5})
                ->ranges({{10, 20}, {40, 60}})
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "51");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<int>() == 51);
    }

    SECTION("constraint-failed-choices-and-ranges") {
        CLI_TEST_DEFINE_NORM_ARG((int), (
                ->choices({1, 3, 5})
                ->ranges({{10, 20}, {40, 60}})
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "30");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '30', should meet constraint: "
            "\\(in set:\\{(1|, |3|, |5){5}\\} or within ranges:\\[\\(10, 20\\), \\(40, 60\\)\\]\\) and 'odd number'"));
    }

    SECTION("constraint-failed-examine") {
        CLI_TEST_DEFINE_NORM_ARG((int), (
                ->choices({1, 3, 5})
                ->ranges({{10, 20}, {40, 60}})
                ->examine([](int &v) -> bool { return v % 2; }, "odd number")),
            "--arg_name", "50");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '50', should meet constraint: "
            "\\(in set:\\{(1|, |3|, |5){5}\\} or within ranges:\\[\\(10, 20\\), \\(40, 60\\)\\]\\) and 'odd number'"));
    }
}
