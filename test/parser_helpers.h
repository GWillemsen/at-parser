#ifndef AT_PARSER_TEST_PARSER_HELPERS_H
#define AT_PARSER_TEST_PARSER_HELPERS_H

#include <vector>
#include <string>
#include <iostream>

#include "at_parser/at_parser.h"

struct Command
{
public:
    at_parser_command_type type;
    std::string command;
    std::vector<std::string> arguments;

    Command(at_parser_command_type type, std::string cmd) : type(type), command(cmd), arguments()
    {
    }

    Command(at_parser_command_type type, std::string cmd, std::vector<std::string> args) : type(type), command(cmd), arguments(args)
    {
    }
};

static std::vector<Command> commands;

class BaseParserFixture
{
protected:
    at_parser_handle_t handle = nullptr;
    BaseParserFixture()
    {
        std::cout << "start" << std::endl;
        commands.clear();
        CHECK_EQ(0, at_parser_create(&handle, 100, '\x1B', ','));
    }

    ~BaseParserFixture() {
        std::cout << "done" << std::endl;
    }
};


extern "C"
{
    static void at_parser_default_received_command(at_parser_handle_t parser, const char *command_name, enum at_parser_command_type type, struct at_parser_argument *argument_list, size_t argument_list_length)
    {
        Command itm = Command(type, std::string(command_name));
        for (size_t i = 0; i < argument_list_length; i++)
        {
            itm.arguments.push_back(std::string(argument_list[i].value, argument_list[i].length));
        }
        commands.push_back(itm);
    }
}

#endif // AT_PARSER_TEST_PARSER_HELPERS_H
