#include "test_helper.hpp"

typedef std::vector<float> Vector;
typedef std::map<std::string, Vector> MapVector;

TEST_CASE("map_vector") {
    SECTION("appear-1") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (), "--arg_name", "key1", "5.12");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MapVector>().at("key1") == Vector{5.12});
    }

    SECTION("appear-N") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (), "--arg_name", "key1", "5.12", "--arg_name", "key2", "1.1", "4.9");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<MapVector>();
        CHECK(av.at("key1") == Vector{5.12});
        CHECK(av.at("key2") == Vector{1.1, 4.9});
    }

    SECTION("line_width-limit-ok") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->line_width(2, 3)), "--arg_name", "key1", "5.12", "1.1");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MapVector>().at("key1") == Vector{5.12, 1.1});
    }

    SECTION("line_width-limit-too-few") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->line_width(2, 3)), "--arg_name", "key1", "5.12");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*a\\(n\\) 'float' value is required as 'map\\[\"key1\"\\]\\[1\\]'"));
    }

    SECTION("line_width-limit-too-many") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->line_width(2, 3))
            , "--arg_name", "key1", "5.12", "1.1", "5.0", "9");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*too many value '9'"));
    }

    SECTION("default-used") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->default_value({{"key1", {5.12, 1.1}}})));
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MapVector>().at("key1") == Vector{5.12, 1.1});
    }

    SECTION("default-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->default_value({{"key1", {5.12, 1.1}}})), "--arg_name", "key1", "4.9");
        CHECK(arg_value.valid());
        CHECK(arg_value.as<MapVector>().at("key1") == Vector{4.9});
    }

    SECTION("implicit-used") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->implicit_value({5.12})), "--arg_name", "key1", "--arg_name", "key2");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<MapVector>();
        CHECK(av.at("key1") == Vector{5.12});
        CHECK(av.at("key2") == Vector{5.12});
    }

    SECTION("implicit-not-used") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->implicit_value({5.12})), "--arg_name", "key1", "4.9", "--arg_name", "key2");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<MapVector>();
        CHECK(av.at("key1") == Vector{4.9});
        CHECK(av.at("key2") == Vector{5.12});
    }

    SECTION("choices-success") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->choices({5.12, 1.1, 5.0})), "--arg_name", "key1", "1.1");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<MapVector>();
        CHECK(av.at("key1") == Vector{1.1});
    }

    SECTION("choices-faield") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->choices({5.12, 1.1, 5.0})), "--arg_name", "key1", "2");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '2', should meet constraint: in-set\\{(5\\.12|, |1\\.1|, |5.*){5}\\}"));
    }

    SECTION("ranges-success") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->ranges({{10.5, 20.8}, {40.5, 60.5}})), "--arg_name", "key1", "50.1");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<MapVector>();
        CHECK(av.at("key1") == Vector{50.1});
    }

    SECTION("ranges-faield") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (->ranges({{10.5, 20.8}, {40.5, 60.5}})), "--arg_name", "key1", "30.1");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '30.1', should meet constraint: within-ranges\\[\\(10\\.5, 20\\.8\\), \\(40\\.5, 60\\.5\\)\\]"));
    }

    SECTION("examine-success") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (
                ->examine([](float &v) -> bool { return v > 0; }, "positive")),
            "--arg_name", "key1", "50.1");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<MapVector>();
        CHECK(av.at("key1") == Vector{50.1});
    }

    SECTION("examine-faield") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (
                ->examine([](float &v) -> bool { return v > 0; }, "positive")),
            "--arg_name", "key1", "-50.1");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '-50.1', should meet constraint: 'positive'"));
    }

    SECTION("constraint-success-choices") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (
                ->choices({5.12, 1.1, 5.0})
                ->ranges({{10.5, 20.8}, {40.5, 60.5}})
                ->examine([](float &v) -> bool { return v == (int)v; }, "integer")),
            "--arg_name", "key1", "5.0");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<MapVector>();
        CHECK(av.at("key1") == Vector{5.0});
    }

    SECTION("constraint-success-ranges") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (
                ->choices({5.12, 1.1, 5.0})
                ->ranges({{10.5, 20.8}, {40.5, 60.5}})
                ->examine([](float &v) -> bool { return v == (int)v; }, "integer")),
            "--arg_name", "key1", "50");
        CHECK(arg_value.valid());
        auto &av = arg_value.as<MapVector>();
        CHECK(av.at("key1") == Vector{50});
    }

    SECTION("constraint-failed-choices-and-ranges") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (
                ->choices({5.12, 1.1, 5.0})
                ->ranges({{10.5, 20.8}, {40.5, 60.5}})
                ->examine([](float &v) -> bool { return v == (int)v; }, "integer")),
            "--arg_name", "key1", "4.0");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '4.0', should meet constraint: "
            "\\(in-set\\{(5\\.12|, |1\\.1|, |5.*){5}\\} or within-ranges\\[\\(10\\.5, 20\\.8\\), \\(40\\.5, 60\\.5\\)\\]\\) and 'integer'"));
    }

    SECTION("constraint-failed-examine") {
        CLI_TEST_DEFINE_NORM_ARG((MapVector), (
                ->choices({5.12, 1.1, 5.0})
                ->ranges({{10.5, 20.8}, {40.5, 60.5}})
                ->examine([](float &v) -> bool { return v == (int)v; }, "integer")),
            "--arg_name", "key1", "40.8");
        CHECK(parser.error());
        CHECK(cli_error_like(parser.error_details(),
            ".*invalid value '40.8', should meet constraint: "
            "\\(in-set\\{(5\\.12|, |1\\.1|, |5.*){5}\\} or within-ranges\\[\\(10\\.5, 20\\.8\\), \\(40\\.5, 60\\.5\\)\\]\\) and 'integer'"));
    }
}
