#include "test_helper.hpp"

TEST_CASE("positional") {
    cliargs::Parser parser;
    parser.add_args()
        ('i', "int", "integer", cliargs::value<int>()
            ->positional()
            )
        ('f', "float", "float", cliargs::value<float>()
            ->positional()
            )
        ('v', "vector", "vector with limits", cliargs::value<std::vector<float>>()
            ->positional()
            ->data_count(2)
            )
        ;
    
    #define TEST_POSITIONAL(...)                              \
        Argv argv({"test_positional", __VA_ARGS__});          \
        auto result = parser.parse(argv.argc(), argv.argv()); \
        auto &i = result["int"];                              \
        auto &f = result["float"];                            \
        auto &v = result["vector"];                           \
        REQUIRE(!parser.error());                             \
        CHECK(i.valid());                                     \
        CHECK(i.as<int>() == -1);                             \
        CHECK(f.valid());                                     \
        CHECK(f.as<float>() == 5.0);                          \
        CHECK(v.valid());                                     \
        CHECK(v.as<std::vector<float>>() == std::vector<float> {1, 2.3});

        SECTION("normal") {
            TEST_POSITIONAL("-1", "5.0", "1", "2.3");
        }

        SECTION("named") {
            TEST_POSITIONAL("-i", "-1", "5.0", "--vector", "1", "2.3");
        }

        SECTION("new_order") {
            TEST_POSITIONAL("-f", "5.0", "1", "2.3", "-1");
        }

        SECTION("new_order_named") {
            TEST_POSITIONAL("-f", "5.0", "-i", "-1", "1", "2.3");
        }

    #undef TEST_POSITIONAL
}
