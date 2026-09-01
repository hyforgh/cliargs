#include "test_helper.hpp"

TEST_CASE("bool") {
    cliargs::Parser parser;
    parser.add_args()
        ('h', "help", "normal bool")
        ('n', "normal_bool", "normal bool", cliargs::value<bool>()->implicit_value(true))
        ('r', "reverse_bool", "reverse bool", cliargs::value<bool>()->implicit_value(false))
    ;

    SECTION("default") {
        Argv argv({"test_bool_default"});
        auto result = parser.parse(argv.argc(), argv.argv());
        auto &help = result["help"];
        auto &normal_bool = result["normal_bool"];
        auto &reverse_bool = result["reverse_bool"];
        REQUIRE(!result.error());
        CHECK(help.valid());
        CHECK(help.as<bool>() == false);
        CHECK(normal_bool.valid());
        CHECK(normal_bool.as<bool>() == false);
        CHECK(reverse_bool.valid());
        CHECK(reverse_bool.as<bool>() == true);
    }

    #define TEST_BOOL(...)                                    \
        Argv argv({__VA_ARGS__});                             \
        auto result = parser.parse(argv.argc(), argv.argv()); \
        auto &help = result["help"];                          \
        auto &normal_bool = result["normal_bool"];            \
        auto &reverse_bool = result["reverse_bool"];          \
        REQUIRE(!result.error());                             \
        CHECK(help.valid());                                  \
        CHECK(help.as<bool>());                               \
        CHECK(normal_bool.valid());                           \
        CHECK(normal_bool.as<bool>());                        \
        CHECK(reverse_bool.valid());                          \
        CHECK(reverse_bool.as<bool>() == false);
        
    SECTION("scatter") {
        TEST_BOOL("test_bool_scatter", "-h", "-n", "-r");
    }

    SECTION("scatter-explict-value") {
        TEST_BOOL("test_bool_scatter", "-h=true", "-n=1", "-r=false");
    }

    SECTION("scatter-explict-value") {
        TEST_BOOL("test_bool_scatter", "-h=True", "-n=1", "-r=False");
    }

    SECTION("scatter-explict-value") {
        TEST_BOOL("test_bool_scatter", "-h=yes", "-n=y", "-r=no");
    }

    SECTION("scatter-explict-value") {
        TEST_BOOL("test_bool_scatter", "-h=Yes", "-n=Y", "-r=No");
    }

    SECTION("gather") {
        TEST_BOOL("test_bool_gather", "-hnr");
    }

    SECTION("gather-explicit-value") {
        TEST_BOOL("test_bool_gather", "-hnr=0");
    }

    SECTION("gather-explicit-value") {
        TEST_BOOL("test_bool_gather", "-hnr=n");
    }

    #undef TEST_BOOL
}
