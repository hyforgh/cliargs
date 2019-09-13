#include "test_helper.hpp"

TEST_CASE("scalar_string") {
    SECTION("simple") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (), "--arg_name", "hello");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "hello");
    }

    SECTION("default-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (->default_value("hello")));
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "hello");
    }

    SECTION("default-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (->default_value("hello")), "--arg_name", "world");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "world");
    }

    SECTION("implicit-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (->implicit_value("world")), "--arg_name");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "world");
    }

    SECTION("implicit-coverred") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (->implicit_value("world")), "--arg_name", "hello");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "hello");
    }

    SECTION("choices-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (->choices({"hello", "world"})), "--arg_name", "hello");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "hello");
    }

    SECTION("choices-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (->choices({"hello", "world"})), "--arg_name", "cliargs");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*unexpected value 'cliargs', should meet constraint: in set:\\{(\"hello\"|, |\"world\"){3}\\}"));
    }

    SECTION("regex-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (->regex("\\d+", "integer")), "--arg_name", "512");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "512");
    }

    SECTION("regex-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (->regex("\\d+", "integer")), "--arg_name", "cliargs");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*unexpected value 'cliargs', should meet constraint: integer"));
    }

    SECTION("examine-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (
                ->examine([](std::string &v) -> bool { return !v.empty(); }, "not empty")),
            "--arg_name", "512");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "512");
    }

    SECTION("examine-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (
                ->examine([](std::string &v) -> bool { return !v.empty(); }, "not empty")),
            "--arg_name", "");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*unexpected value '', should meet constraint: 'not empty'"));
    }

    SECTION("constraint-success-choices") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (
                ->choices({"hello", "world"})
                ->regex("\\d+", "integer")
                ->examine([](std::string &v) -> bool { return !v.empty(); }, "not empty")),
            "--arg_name", "hello");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "hello");
    }

    SECTION("constraint-success-regex") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (
                ->choices({"hello", "world"})
                ->regex("\\d+", "integer")
                ->examine([](std::string &v) -> bool { return v.length() < 4; }, "shorter than 4")),
            "--arg_name", "5");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::string>() == "5");
    }

    SECTION("constraint-failed-choices-and-regex") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (
                ->choices({"hello", "world"})
                ->regex("\\d+", "integer")
                ->examine([](std::string &v) -> bool { return v.length() < 4; }, "shorter than 4")),
            "--arg_name", "5j");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*unexpected value '5j', should meet constraint: "
            "\\(in set:\\{(\"hello\"|, |\"world\"){3}\\} or integer\\) and 'shorter than 4'"));
    }

    SECTION("constraint-failed-examine") {
        CLI_TEST_DEFINE_NORM_ARG((std::string), (
                ->choices({"hello", "world"})
                ->regex("\\d+", "integer")
                ->examine([](std::string &v) -> bool { return v.length() < 4; }, "shorter than 4")),
            "--arg_name", "5120");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*unexpected value '5120', should meet constraint: "
            "\\(in set:\\{(\"hello\"|, |\"world\"){3}\\} or integer\\) and 'shorter than 4'"));
    }

#undef PARSE_NUMERIC
}
