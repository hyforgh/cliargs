#include "test_helper.hpp"

TEST_CASE("vector_numeric") {
    SECTION("appear-1") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (), "--arg_name", "5.12");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{5.12});
    }

    SECTION("appear-N") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (), "--arg_name", "5.12", "--arg_name", "1.1", "4.9");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{5.12, 1.1, 4.9});
    }

    SECTION("data_count-limit-ok") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->data_count(2, 3)), "--arg_name", "5.12", "--arg_name", "1.1");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{5.12, 1.1});
    }

    SECTION("data_count-limit-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->data_count(2, 3)), "--arg_name", "5.12");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*expects 2 ~ 3 value\\(s\\), but got 1"));
    }

    SECTION("data_count-limit-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->data_count(2, 3))
            , "--arg_name", "5.12", "--arg_name", "1.1", "--arg_name", "5.0", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many value '9'"));
    }

    SECTION("data_count-limit-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->data_count(2, 3))
            , "--arg_name", "5.12", "--arg_name", "1.1", "--arg_name", "5.0", "--arg_name", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many appearances \\['9'\\]"));
    }

    SECTION("default-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->default_value({5.12, 1.1})));
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{5.12, 1.1});
    }

    SECTION("default-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->default_value({5.12, 1.1})), "--arg_name", "4.9");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{4.9});
    }

    SECTION("implicit-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->implicit_value({5.12})), "--arg_name", "--arg_name");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{5.12, 5.12});
    }

    SECTION("implicit-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->implicit_value({5.12})), "--arg_name", "4.9", "--arg_name");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{4.9, 5.12});
    }

    SECTION("choices-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->choices({5.12, 1.1, 5.0})), "--arg_name", "1.1");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{1.1});
    }

    SECTION("choices-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->choices({5.12, 1.1, 5.0})), "--arg_name", "2");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '2', should meet constraint: in set:\\{(5\\.12|, |1\\.1|, |5.*){5}\\}"));
    }

    SECTION("ranges-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->ranges({{10.5, 20.8}, {40.5, 60.5}})), "--arg_name", "50.1");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{50.1});
    }

    SECTION("ranges-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (->ranges({{10.5, 20.8}, {40.5, 60.5}})), "--arg_name", "30.1");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '30.1', should meet constraint: within ranges:\\[\\(10\\.5, 20\\.8\\), \\(40\\.5, 60\\.5\\)\\]"));
    }

    SECTION("examine-success") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (
                ->examine([](float &v) -> bool { return v > 0; }, "positive")),
            "--arg_name", "50.1");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{50.1});
    }

    SECTION("examine-faield") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (
                ->examine([](float &v) -> bool { return v > 0; }, "positive")),
            "--arg_name", "-50.1");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '-50.1', should meet constraint: 'positive'"));
    }

    SECTION("constraint-success-choices") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (
                ->choices({5.12, 1.1, 5.0})
                ->ranges({{10.5, 20.8}, {40.5, 60.5}})
                ->examine([](float &v) -> bool { return v == (int)v; }, "integer")),
            "--arg_name", "5.0");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{5.0});
    }

    SECTION("constraint-success-ranges") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (
                ->choices({5.12, 1.1, 5.0})
                ->ranges({{10.5, 20.8}, {40.5, 60.5}})
                ->examine([](float &v) -> bool { return v == (int)v; }, "integer")),
            "--arg_name", "50");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<std::vector<float>>() == std::vector<float>{50});
    }

    SECTION("constraint-failed-choices-and-ranges") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (
                ->choices({5.12, 1.1, 5.0})
                ->ranges({{10.5, 20.8}, {40.5, 60.5}})
                ->examine([](float &v) -> bool { return v == (int)v; }, "integer")),
            "--arg_name", "4.0");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '4.0', should meet constraint: "
            "\\(in set:\\{(5\\.12|, |1\\.1|, |5.*){5}\\} or within ranges:\\[\\(10\\.5, 20\\.8\\), \\(40\\.5, 60\\.5\\)\\]\\) and 'integer'"));
    }

    SECTION("constraint-failed-examine") {
        CLI_TEST_DEFINE_NORM_ARG((std::vector<float>), (
                ->choices({5.12, 1.1, 5.0})
                ->ranges({{10.5, 20.8}, {40.5, 60.5}})
                ->examine([](float &v) -> bool { return v == (int)v; }, "integer")),
            "--arg_name", "40.8");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '40.8', should meet constraint: "
            "\\(in set:\\{(5\\.12|, |1\\.1|, |5.*){5}\\} or within ranges:\\[\\(10\\.5, 20\\.8\\), \\(40\\.5, 60\\.5\\)\\]\\) and 'integer'"));
    }
}
