#include "cliargs.hpp"

struct MyStruct {
    std::string name;
    float gain;
    long size;
};
// overload oerator << for printing default-value and enum-value in help
std::ostream &operator << (std::ostream &os, const MyStruct &obj) {
    os << "{.name=" << obj.name << ", .gain=" << obj.gain << ", .size=" << obj.size << "}";
    return os;
}

void __parse_by_parser(MyStruct &obj, cliargs::ArgParser &parser, const std::string &name) {
    parser.domain_begin(name.empty() ? "MyStruct" : name); // tell ArgParser the struct's name
    if (parser.assign(obj.name, "name")) { // MyStruct::name required a string value
        parser.check(!obj.name.empty(), "invalid name: empty");
    }
    parser.assign(obj.gain, "gain"); // MyStruct::gain required an uint64 value
    parser.set_optional(); // the followwing member is optional
    parser.assign(obj.size, "size", (long)0); // specify a default value for optional member
    parser.domain_end();
}

int main(int argc, char *argv[]) {
    //  Create a 'cliargs::Parser' instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");
    // Define arguments
    parser.add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ('s', "my_struct", "usage `--my_struct file_name [gain [size]]`",
            cliargs::value<MyStruct>()
            )
        ('v', "vector", "`--vector file_name [gain [size]]`",
            cliargs::value<std::vector<MyStruct>>()
            ->examine([](MyStruct &obj, void *context, void *data) -> bool {
                auto &data_vec = *reinterpret_cast<std::vector<MyStruct> *>(data);
                for (auto &it : data_vec) {
                    if (it.name == obj.name) {
                        if (it.gain <= obj.gain && it.gain + it.size > obj.gain) {
                            return false;
                        }
                        if (obj.gain <= it.gain && obj.gain + obj.size > it.gain) {
                            return false;
                        }
                    }
                }
                return true;
            }, "文件内容范围不可交叠")
            )
        ('m', "map", "usage `--map key file_name [gain [size]]`",
            cliargs::value<std::map<std::string, MyStruct>>()
            )
        ("tail", "usage `--tail file_name [gain [size]]`",
            cliargs::value<MyStruct>()
            ->implicit_value(MyStruct {.name = "data"})
            )
        ("enum", "usage `--enum file_name [gain [size]]`",
            cliargs::value<MyStruct>()->examine([](MyStruct &obj) -> bool {
                static std::set<std::string> s_name_enum = {"a", "b", "s"};
                return s_name_enum.find(obj.name) != s_name_enum.end();
            }, "name set: {'a', 'b', 's'}")
            )
        ;
    // Parse
    auto result = parser.parse(argc, argv);
    if (parser.error() || result["help"].as<bool>()) {
        parser.print_help();
        return parser.error() ? -1 : 0;
    }
    // Use result
    std::cout << "my_struct: " << cliargs::to_string(result["my_struct"].as<MyStruct>()) << std::endl;
    std::vector<MyStruct> load = result["vector"].as<std::vector<MyStruct>>();
    std::cout << "vector: " << cliargs::to_string(load) << std::endl;
    std::cout << "   map: " << cliargs::to_string(result["map"].as<std::map<std::string, MyStruct>>()) << std::endl;
    std::cout << "  tail: " << cliargs::to_string(result["tail"].as<MyStruct>()) << std::endl;
    std::cout << "  enum: " << cliargs::to_string(result["enum"].as<MyStruct>()) << std::endl;
    return 0;
}
