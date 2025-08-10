#include "cliargs.hpp"

// Declare struct
struct MyStruct {
    std::string name;
    int64_t offset;
    uint64_t size;
    std::string desc;
};
// overload oerator << for printing default-value and enum-value in help
std::ostream &operator << (std::ostream &os, const MyStruct &obj) {
    os << "{.name=" << obj.name
       << ", .offset=" << obj.offset
       << ", .size=" << obj.size
       << ", .desc=" << obj.desc
       << "}";
    return os;
}

void __parse_by_parser(MyStruct &obj, cliargs::ArgParser &parser, const std::string &name = "MyStruct") {
    parser.domain_begin(name);
    parser.assign(obj.name, "name"); // MyStruct::name required a string value
    parser.assign(obj.offset, "offset"); // MyStruct::offset required an uint64 value
    parser.set_optional(); // the followwing member is optional
    parser.assign(obj.size, "size", (uint64_t)0);
    parser.assign(obj.desc, "desc");
    parser.domain_end();
}

int g_value_default = 0;
const int g_value_implicit = -1;

int main(int argc, char *argv[]) {
    //  Create a 'cliargs::Parser' instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");
    // Define arguments
    parser.set_width(100).sensitive_mode().add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ('v', "value", "An interger",
            cliargs::value<int>()
            ->positional()
            ->default_value(g_value_default)
            ->implicit_value(g_value_implicit)
            ->choices({-1, 0})->range(10, 20)->range(30, 40)
            )
        ('s', "string", "A string",
            cliargs::value<std::string>()
            ->positional()
            ->default_value("/dev/mem")
            ->choices({"a", "b"})->regex("/dev/.*")
            ->examine([](const std::string &v, void *context) -> bool {
                return !v.empty();
            }, "not be empty")
            )
        ('b', "base_address", "Specify a base address for the followwing operations",
            cliargs::value<uint64_t>()
            ->default_value(0)
            ->choices({7, 8})->ranges({{0, 10}, {20, 30}})->ranges({{40, 50}})
            ->examine([](const uint64_t &v, void *context) -> bool { return v % 2 == 0; }
                , "be multiples of 2")
            , "ba"
            )
        ('d', "dump", "Dump data slices to files. usage '--dump filename address size [skip]'",
            cliargs::value<std::vector<std::tuple<std::string, uint64_t, uint64_t, uint64_t>>>()
                // The first three members of the tuple are mandatory, and the last one is optional
            ->implicit_value({"", 0, 0, 0})->line_width(2)
                // If user only specifies first 3 values, the last one while be automatically setted to 3
            )
        ('l', "load", "Load data from file. usage `--load file_name [offset [size [desc]]]`",
            cliargs::value<std::vector<MyStruct>>()->data_count(1, -1)
            )
        ('f', "flag", "set flag", cliargs::value<int>())
        ('a', "args", "args for subprocess",
            cliargs::value<std::vector<char *>>()
            ->stop_at_eof()
            )
        ;
    // Parse
    auto result = parser.parse(argc, argv);
    if (parser.error() || result["help"].as<bool>()) {
        parser.print_help();
        return parser.error() ? -1 : 0;
    }
    // Use result
    std::cout << "value: " << cliargs::to_string(result["value"].as<int>()) << std::endl;
    const std::string &mem_file = result["string"].as<std::string>();
    std::cout << "string: " << cliargs::to_string(mem_file) << std::endl;
    std::cout << "base_address: " << cliargs::to_string(result["base_address"].as<uint64_t>()) << std::endl;
    const auto &dump = result["dump"].as<std::vector<std::tuple<std::string, uint64_t, uint64_t, uint64_t>>>();
    std::cout << "dump: " << cliargs::to_string(dump) << std::endl;
    const std::vector<MyStruct> &load = result["load"].as<std::vector<MyStruct>>();
    std::cout << "load: " << cliargs::to_string(load) << std::endl;
    std::cout << "flag: " << cliargs::to_string(result["flag"].as<int>()) << std::endl;
    std::cout << "tail: " << cliargs::to_string(result.tail().argv, result.tail().argc) << std::endl;
    auto args = result["args"].as<std::vector<char *>>();
    std::cout << "args: " << cliargs::to_string(args.data(), args.size()) << std::endl;

    // group1
    cliargs::Parser parser2("subprocess");
    parser2.add_args()
        ('i', "int", "An interger", cliargs::value<int>()->default_value(-1))
        ('s', "string", "A string", cliargs::value<std::string>()->default_value("/dev/mem"), "str")
        ('v', "vector", "An int vector", cliargs::value<std::vector<float>>()->data_count(1, 2))
        ;
    auto result2 = parser2.parse(args.size(), args.data(), 0);
    std::cout << "2    int: " << cliargs::to_string(result2["int"].as<int>()) << std::endl;
    std::cout << "2 string: " << cliargs::to_string(result2["string"].as<std::string>()) << std::endl;
    std::cout << "2 vector: " << cliargs::to_string(result2["vector"].as<std::vector<float>>()) << std::endl;
    std::cout << "2   tail: " << cliargs::to_string(result2.tail().argv, result2.tail().argc) << std::endl;

    // group2
    cliargs::Parser parser3("MyProgram", "One line description of MyProgram");
    parser3.set_width(120).add_args()
        ('i', "int", "An interger", cliargs::value<int>()->default_value(-1))
        ('s', "string", "A string", cliargs::value<std::string>()->default_value("/dev/mem"), "str")
        ('v', "vector", "An int vector", cliargs::value<std::vector<float>>()->data_count(1, 2))
        ("group1", "Arguments group 1", cliargs::value<std::vector<char *>>()->stop_at_eof())
        ("group2", "Arguments group 2", cliargs::value<std::vector<char *>>()->stop_at_eof())
        ;
    auto result3 = parser3.parse(result.tail().argc, result.tail().argv, 0);
    std::cout << "3    int: " << cliargs::to_string(result3["int"].as<int>()) << std::endl;
    std::cout << "3 string: " << cliargs::to_string(result3["string"].as<std::string>()) << std::endl;
    std::cout << "3 vector: " << cliargs::to_string(result3["vector"].as<std::vector<float>>()) << std::endl;
    std::cout << "3 group1: " << cliargs::to_string(result3["group1"].as<std::vector<char *>>()) << std::endl;
    std::cout << "3 group2: " << cliargs::to_string(result3["group2"].as<std::vector<char *>>()) << std::endl;
    std::cout << "3   tail: " << cliargs::to_string(result3.tail().argv, result3.tail().argc) << std::endl;

    return 0;
}
