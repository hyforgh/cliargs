#include "cliargs.hpp"

typedef std::tuple<std::string, uint64_t, uint32_t> MyTuple;

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
    if (parser.assign(obj.name, "name")) { // MyStruct::name required a string value
        parser.check(!obj.name.empty(), "an empty string");
    }
    parser.assign(obj.offset, "offset"); // MyStruct::offset required an uint64 value
    parser.set_optional(); // the followwing member is optional
    parser.assign(obj.size, "size", (uint64_t)0);
    parser.domain_end();
}

int main(int argc, char *argv[]) {
    //  Create a 'cliargs::Parser' instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");
    // Define arguments
    parser.add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ('s', "str", "A string enum",
            cliargs::value<std::string>()
            ->choices({"dump", "load"})->regex("\\d+", "a integer")
            // the value can only be one of {"dump", "load"} or an interger
        )
        ('i', "int", "An interger enum",
            cliargs::value<std::vector<int>>()
            ->choices({1, 3, 5})->ranges({{10, 20}, {30, 50}})
            ->examine([](int &v) -> bool { return v % 2; }, "an odd number")
            // the value can only be one of {1, 3, 5} or in range [10, 20] or [30, 50] and be an odd number
        )
        ('m', "map", "An std::map<std::string, int>",
            cliargs::value<std::map<std::string, int>>()
            ->choices({1, 3, 5})->ranges({{10, 20}, {30, 50}})
            ->examine([](int &v) -> bool { return v % 2; }, "an odd number")
            // the value can only be one of {1, 3, 5} or in range [10, 20] or [30, 50] and be an odd number
        )
        ('t', "tuple", "A tuple enum",
            cliargs::value<MyTuple>()
            ->examine([](MyTuple &obj) -> bool { return !std::get<0>(obj).empty(); }
                , "first element of tuple must not be empty")
        )
        ('u', "user", "An struct enum",
            cliargs::value<MyStruct>()
            ->examine([](MyStruct &obj) -> bool { return obj.size > 0; }, "size should greater than 0")
        )
        ;
    // Parse
    auto result = parser.parse(argc, argv);
    if (parser.error()) {
        parser.print_help();
        return -1;
    }
    if (result["help"].as<bool>()) {
        parser.print_help();
        return 0;
    }
    // Use result
    std::cout << "str: " << cliargs::to_string(result["str"].as<std::string>()) << std::endl;
    std::cout << "int: " << cliargs::to_string(result["int"].as<std::vector<int>>()) << std::endl;
    std::cout << "tuple: " << cliargs::to_string(result["tuple"].as<MyTuple>()) << std::endl;
    std::cout << "user: " << cliargs::to_string(result["user"].as<MyStruct>()) << std::endl;
    return 0;
}
