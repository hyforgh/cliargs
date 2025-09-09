#include "cliargs.hpp"

int main(int argc, char *argv[]) {
    //  Create a 'cliargs::Parser' instance
    cliargs::Parser parser("git");
    // Define arguments
    parser.set_width(120).sensitive_mode().add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ("cmd", "commands", cliargs::value<std::string>()
            ->positional()
            ->choices({"add", "log", "commit"})
            )
        ("arg", "arguments", cliargs::value<std::vector<char *>>()
            ->positional()
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
    std::cout << "cmd: " << cliargs::to_string(result["cmd"].as<std::string>()) << std::endl;
    std::cout << "arg: " << cliargs::to_string(result["arg"].as<std::vector<char *>>()) << std::endl;
    return 0;
}
