#include "cliargs.hpp"

int main(int argc, char *argv[]) {
    //  Create a 'cliargs::Parser' instance
    cliargs::Parser parser("MyProgram", "One line description of MyProgram");
    // Define arguments
    parser.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ('i', "int", "An interger", cliargs::value<int>()->default_value(-1))
        ('s', "string", "A string", cliargs::value<std::string>()->default_value("/dev/mem"))
        ('v', "vector", "An int vector", cliargs::value<std::vector<float>>()->data_count(1, 2))
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
    std::cout << "   int: " << cliargs::to_string(result["int"].as<int>()) << std::endl;
    std::cout << "string: " << cliargs::to_string(result["string"].as<std::string>()) << std::endl;
    std::cout << "vector: " << cliargs::to_string(result["vector"].as<std::vector<float>>()) << std::endl;
    return 0;
}
