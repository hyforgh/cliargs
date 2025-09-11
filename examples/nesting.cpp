#include "cliargs.hpp"

int main(int argc, char *argv[]) {
    cliargs::Parser parser("nesting");
    parser.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ("version", "print version")
        ("cmd", "commands", cliargs::value<std::string>()
            ->positional()
            ->sensitive_mode()
            ->choices({"add", "log", "commit"})
            )
        ("arg", "arguments", cliargs::value<std::vector<char *>>()
            ->positional()
            )
        ;
    cliargs::Parser parser_add("nesting add");
    parser_add.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ("file-path", "commands", cliargs::value<std::vector<std::string>>()
            ->positional())
        ;
    cliargs::Parser parser_log("nesting log");
    parser_log.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ("log-size", "commands", cliargs::value<unsigned>())
        ("oneline", "arguments")
        ;
    cliargs::Parser parser_commit("nesting commit");
    parser_commit.set_width(120).add_args()
        ('h', "help", "Print this message and exit") // a bool argument
        ('m', "message", "new commit", cliargs::value<std::string>())
        ("amend", "amend commit")
        ;
    auto result = parser.parse(argc, argv);
    if (parser.error()) {
        parser.print_help();
        return -1;
    }
    auto &cmd = result["cmd"].as<std::string>();
    auto &arg = result["arg"].as<std::vector<char *>>();

    auto parse_sub = [&](cliargs::Parser &cmd_parser, cliargs::Result &cmd_result) {
        cmd_result = cmd_parser.parse(arg.size(), const_cast<char **>(arg.data()), 0);
        if (result["help"].as<bool>()) {
            if (cmd_parser.error() || cmd_result["help"].as<bool>()) {
                cmd_parser.print_help();
                if (cmd_parser.error()) {
                    exit(-1);
                }
            } else {
                parser.print_help();
            }
            exit(0);
        }
    };
    cliargs::Result cmd_result;
    if (cmd == "add") {
        parse_sub(parser_add, cmd_result);
        std::cout << "file-path:" << cliargs::to_string(cmd_result["file-path"].as<std::vector<std::string>>()) << std::endl;
    } else if (cmd == "log") {
        parse_sub(parser_log, cmd_result);
        std::cout << "log-size:" << cliargs::to_string(cmd_result["log-size"].as<unsigned>()) << std::endl;
        std::cout << " oneline:" << cliargs::to_string(cmd_result["oneline"].as<bool>()) << std::endl;
    } else if (cmd == "commit") {
        parse_sub(parser_commit, cmd_result);
        std::cout << "message:" << cliargs::to_string(cmd_result["message"].as<std::string>()) << std::endl;
        std::cout << "  amend:" << cliargs::to_string(cmd_result["amend"].as<bool>()) << std::endl;
    } else {
        parser.print_help();
        return 0;
    }
    return 0;
}
