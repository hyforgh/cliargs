#include "cliargs.hpp"

typedef std::tuple<std::string, uint64_t, uint32_t> MyTuple;

int main(int argc, char *argv[]) {
    //  Create a 'cliargs::Parser' instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");
    // Define arguments
    parser.add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ('s', "scalar", "Load data from file. usage `--scalar file_name [offset [size]]`",
            cliargs::value<MyTuple>()
            )
        ('v', "vector", "Load data from file. usage `--vector file_name [offset [size]]`",
            cliargs::value<std::vector<MyTuple>>()
            )
        ('m', "map", "Load data from file. usage `--map key file_name [offset [size]]`",
            cliargs::value<std::map<std::string, MyTuple>>()
            )
        ("tail", "Load data from file. usage `--tail file_name [offset [size]]`",
            cliargs::value<MyTuple>()
            ->implicit_value({"data.bin", 0x10, 0})
            ->examine([](MyTuple &obj) { return !std::get<0>(obj).empty(); }, "第一个元素不能为空")
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
    std::cout << "scalar: " << cliargs::to_string(result["scalar"].as<MyTuple>()) << std::endl;
    std::vector<MyTuple> load = result["vector"].as<std::vector<MyTuple>>();
    std::cout << "vector: " << cliargs::to_string(load) << std::endl;
    std::cout << "scalar: " << cliargs::to_string(result["map"].as<std::map<std::string, MyTuple>>()) << std::endl;
    std::cout << "tail: " << cliargs::to_string(result["tail"].as<MyTuple>()) << std::endl;
    return 0;
}
