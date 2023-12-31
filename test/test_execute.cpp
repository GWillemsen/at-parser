#include "doctest.h"
#include <string.h>
#include <string>
#include <vector>
#include "at_parser/at_parser.h"
#include "parser_helpers.h"
TEST_CASE("Execute AT Commands")
{
    at_parser_handle_t handle = nullptr;
    commands.clear();
    CHECK_EQ(0, at_parser_create(&handle, 100, '\x1B', ','));

    SUBCASE("Single handles registered and single commands in buffer")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(0, commands[0].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_EXECUTE, commands[0].type);
    }

    SUBCASE("Multiple handles registered and signle command in buffer")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "ABC", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "DEF", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(0, commands[0].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_EXECUTE, commands[0].type);
    }

    SUBCASE("Multiple handles registered and multiple commands in buffer")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "ABC", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "DEF", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW\r\nAT+ABC\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(2, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(0, commands[0].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_EXECUTE, commands[0].type);
        CHECK(std::string("ABC") == commands[1].command);
        CHECK_EQ(0, commands[1].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_EXECUTE, commands[1].type);
    }

    at_parser_free(handle);
}

