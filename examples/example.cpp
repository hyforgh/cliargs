#include "cliargs.hpp"

// Declare struct
struct MyStruct {
    std::string name;
    uint64_t offset;
    uint64_t size;
};
// overload oerator << for printing default-value and enum-value in help
std::ostream &operator << (std::ostream &os, const MyStruct &obj) {
    os << "{.name=" << obj.name << ", .offset=" << obj.offset << ", .size=" << obj.size << "}";
    return os;
}

void __parse_by_parser(MyStruct &obj, cliargs::ArgParser &parser, const std::string &name = "MyStruct") {
    parser.domain_begin(name);
    parser.assign(obj.name, "name"); // MyStruct::name required a string value
    parser.assign(obj.offset, "offset"); // MyStruct::offset required an uint64 value
    parser.set_optional(); // the followwing member is optional
    parser.assign(obj.size, "size", (uint64_t)0);
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
        ('l', "load", "Load data from file. usage `--load file_name [offset [size]]`",
            cliargs::value<std::vector<MyStruct>>()->data_count(1, -1)
            )
        ('f', "flag", "set flag", cliargs::value<int>())
        ;
    // Parse
    auto result = parser.parse(argc, argv);
    if (parser.error() || result["help"].as<bool>()) {
        parser.print_help();
        return parser.error() ? -1 : 0;
    }
    // Use result
    std::cout << "value: " << cliargs::to_string(result["value"].as<int>()) << std::endl;
    std::string mem_file = result["string"].as<std::string>();
    std::cout << "string: " << cliargs::to_string(mem_file) << std::endl;
    std::cout << "base_address: " << cliargs::to_string(result["base_address"].as<uint64_t>()) << std::endl;
    auto dump = result["dump"].as<std::vector<std::tuple<std::string, uint64_t, uint64_t, uint64_t>>>();
    std::cout << "dump: " << cliargs::to_string(dump) << std::endl;
    std::vector<MyStruct> load = result["load"].as<std::vector<MyStruct>>();
    std::cout << "load: " << cliargs::to_string(load) << std::endl;
    std::cout << "flag: " << cliargs::to_string(result["flag"].as<int>()) << std::endl;
    return 0;
}
