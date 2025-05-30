#include "test_helper.hpp"

struct MyStruct {
    std::string name;
    float gain;
    std::string desc;
};
std::ostream &operator << (std::ostream &os, const MyStruct &obj) {
    os << "{.name=\"" << obj.name << "\", .gain=" << obj.gain << ", .flag=" << obj.desc << "}";
    return os;
}
void __parse_by_parser(MyStruct &obj, cliargs::ArgParser &parser, const std::string &name) {
    parser.domain_begin(name.empty() ? "MyStruct" : name);
    if (parser.assign(obj.name, "name")) {
        parser.check(!obj.name.empty(), "an empty name");
    }
    parser.assign(obj.gain, "gain");
    parser.set_optional();
    parser.assign(obj.desc, "desc", std::string("default"));
    parser.domain_end();
}

bool operator == (const MyStruct &a, const MyStruct &b) {
    return a.name == b.name && a.gain == b.gain && a.desc == b.desc;
}

TEST_CASE("optional_invalid") {
    cliargs::Parser parser;
    parser.add_args()
        ('i', "int", "integer", cliargs::value<int>())
        ('s', "struct", "struct with option member", cliargs::value<MyStruct>())
        ;
    Argv argv({"cli_test", "-s", "mengxinghun", "5.0", "-i", "2"});
    auto result = parser.parse(argv.argc(), argv.argv());
    auto &arg_value = result["struct"];
    CHECK(arg_value.valid());
    CHECK(arg_value.as<MyStruct>() == MyStruct {"mengxinghun", 5.0, "-i"});
    CHECK(!result["int"].valid());
}

TEST_CASE("optional_valid_in_sensitive_mode") {
    cliargs::Parser parser;
    parser.sensitive_mode().add_args()
        ('i', "int", "integer", cliargs::value<int>())
        ('s', "struct", "struct with option member", cliargs::value<MyStruct>())
        ;
    Argv argv({"cli_test", "-s", "mengxinghun", "5.0", "-i", "2"});
    auto result = parser.parse(argv.argc(), argv.argv());
    auto &arg_value = result["struct"];
    CHECK(arg_value.valid());
    CHECK(arg_value.as<MyStruct>() == MyStruct {"mengxinghun", 5.0, "default"});
    CHECK(result["int"].valid());
    CHECK(result["int"].as<int>() == 2);
}

TEST_CASE("optional_valid_by_digital_detecting") {
    cliargs::Parser parser;
    parser.add_args()
        ('i', "int", "integer", cliargs::value<int>())
        ('v', "vector", "vector with limits", cliargs::value<std::vector<float>>()
            ->data_count(2, 3)
            )
        ;
    Argv argv({"cli_test", "-v", "-1", "5.0", "-i", "2"});
    auto result = parser.parse(argv.argc(), argv.argv());
    auto &arg_value = result["vector"];
    CHECK(arg_value.valid());
    CHECK(arg_value.as<std::vector<float>>() == std::vector<float> {-1, 5.0});
    CHECK(result["int"].valid());
    CHECK(result["int"].as<int>() == 2);
}
