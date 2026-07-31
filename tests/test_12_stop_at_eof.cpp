#include "test_helper.hpp"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

TEST_CASE("stop_at_eof_gnu") {
    cliargs::Parser parser;
    parser.add_args()
        ('i', "int", "An interger", cliargs::value<int>()->default_value(-1))
        ('v', "vec", "An int vector", cliargs::value<std::vector<float>>()->data_count(1, 2))
        ;
    Argv argv({"cli_test", "-i", "-1", "--", "-v", "2", "1.0"});
    auto result = parser.parse(argv.argc(), argv.argv());
    CHECK(result["int"].as<int>() == -1);
    CHECK(!result["vec"].valid());
    auto &tail = result.tail();
    const char *tail_golden[] = {"-v", "2", "1.0"};
    CHECK(tail.argc == ARRAY_LENGTH(tail_golden));
    for (int i = 0; i < tail.argc; ++i) {
        CHECK(strcmp(tail.argv[i], tail_golden[i]) == 0);
    }
}

TEST_CASE("stop_at_eof_sensitive") {
    cliargs::Parser parser;
    parser.add_args()
        ('i', "int", "An interger", cliargs::value<int>()->default_value(-1))
        ('v', "vec", "An int vector", cliargs::value<std::vector<float>>()->data_count(1, 2))
        ("group1", "Arguments group 1", cliargs::value<std::vector<char *>>()->stop_at_eof())
        ("group2", "Arguments group 2", cliargs::value<std::vector<std::string>>()->stop_at_eof())
        ;
    Argv argv({"cli_test", "-i", "-1"
        , "--group1", "-v", "3", "--help"
        , "--", "--group2", "-v", "4", "5"
        , "--", "-v", "2", "1.0"});
    auto result = parser.parse(argv.argc(), argv.argv());
    CHECK(result["int"].as<int>() == -1);
    CHECK(!result["vec"].valid());
    auto &tail = result.tail();
    const char *tail_golden[] = {"-v", "2", "1.0"};
    CHECK(tail.argc == ARRAY_LENGTH(tail_golden));
    for (int i = 0; i < tail.argc; ++i) {
        CHECK(strcmp(tail.argv[i], tail_golden[i]) == 0);
    }
    auto &group1 = result["group1"].as<std::vector<char *>>();
    const char *group1_golden[] = {"-v", "3", "--help"};
    CHECK(group1.size() == ARRAY_LENGTH(group1_golden));
    for (size_t i = 0; i < group1.size(); ++i) {
        CHECK(strcmp(group1[i], group1_golden[i]) == 0);
    }
    auto &group2 = result["group2"].as<std::vector<std::string>>();
    const char *group2_golden[] = {"-v", "4", "5"};
    CHECK(group2.size() == ARRAY_LENGTH(group2_golden));
    for (size_t i = 0; i < group2.size(); ++i) {
        CHECK(group2[i] == group2_golden[i]);
    }
}
