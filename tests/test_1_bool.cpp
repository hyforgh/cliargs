#include "test_helper.hpp"

TEST_CASE("bool") {
    cliargs::Parser parser;
    parser.add_args()
        ('h', "help", "normal bool")
        ('b', "normal_bool", "normal bool", cliargs::value<bool>()->implicit_value(true))
        ('r', "reverse_bool", "reverse bool", cliargs::value<bool>()->implicit_value(false))
    ;

    SECTION("scatter") {
#define TEST_BOOL(...)                                        \
        Argv argv({__VA_ARGS__});                             \
        auto result = parser.parse(argv.argc(), argv.argv()); \
        auto &help = result["help"];                          \
        auto &normal_bool = result["normal_bool"];            \
        auto &reverse_bool = result["reverse_bool"];          \
                                                              \
        REQUIRE(!parser.error());                             \
        CHECK(help.valid());                                  \
        CHECK(help.as<bool>());                               \
        CHECK(normal_bool.valid());                           \
        CHECK(normal_bool.as<bool>());                        \
        CHECK(reverse_bool.valid());                          \
        CHECK(reverse_bool.as<bool>() == false);
        
        TEST_BOOL("test_bool_scatter", "-h", "-b", "-r");
    }

    SECTION("gather") {
        TEST_BOOL("test_bool_gather", "-hbr");

#undef TEST_BOOL
    }
}
