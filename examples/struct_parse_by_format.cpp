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

const char *__parse_by_format(MyStruct &obj, char *psz
        , std::string name, std::list<std::string> &err_list
        , void *context, char *parent) {
    const char *type_name = "\"string,float[,long]\"";
    if (!psz || !psz[0]) {
        return type_name;
    }
    // try to split the string like "data.bin,32,64" or "data.bin,32"
    std::list<std::string> sub_list;
    int b = 0;
    while (psz[b]) {
        int e = b;
        while (psz[e] && psz[e] != ',') ++e;
        sub_list.emplace_back(psz + b, psz + e);
        if (psz[e] == ',') {
            b = e + 1;
        } else {
            break;
        }
    }
    // asign struct's member
    auto err_head = std::string("format error: '") + parent + "'"
        + " as format '" + type_name
        + "', a value is required for ";
    auto it = sub_list.begin();
    if (it == sub_list.end()) {
        err_list.emplace_back(err_head + "'MyStruct::name'"); // 'MyStruct::name' is required
    } else {
        cliargs::__parse_by_format(obj.name, (char *)it->c_str(), "MyStruct::name"
            , err_list, context, parent);
        ++it;
    }
    if (it == sub_list.end()) {
        err_list.emplace_back(err_head + "'MyStruct::gain'"); // 'MyStruct::gain' is required
    } else {
        cliargs::__parse_by_format(obj.gain, (char *)it->c_str(), "MyStruct::gain"
            , err_list, context, parent);
        ++it;
    }
    if (it == sub_list.end()) {
        obj.size = 0; // "MyStruct::size" is optional, assign a default value if not specified by user
    } else {
        cliargs::__parse_by_format(obj.size, (char *)it->c_str(), "MyStruct::size"
            , err_list, context, parent);
    }
    if (sub_list.size() < 2) {
        err_list.emplace_back("'MyStruct' expects 2 ~ 3 values(s) but got " + std::to_string(sub_list.size()));
    }
    return type_name;
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
    if (parser.error()) {
        parser.print_help();
        return -1;
    }
    if (result["help"].as<bool>()) {
        parser.print_help();
        return 0;
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
