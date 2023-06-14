#include "doctest.h"
#include <string.h>
#include <string>
#include <vector>
#include "at_parser/at_parser.h"
#include "parser_helpers.h"

TEST_CASE("Test multi-buffer AT Commands")
{
    at_parser_handle_t handle = nullptr;
    commands.clear();
    CHECK_EQ(0, at_parser_create(&handle, 16, '\x1B', ','));
    CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command));
    CHECK_EQ(0, at_parser_add_command_handler(handle, "ABC", at_parser_default_received_command));
    CHECK_EQ(0, at_parser_add_command_handler(handle, "DEF", at_parser_default_received_command));

    SUBCASE("Set command")
    {
        const char *buffer = "AT+HELLOW=\"hi\"\r\nAT+ABC=def\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(2, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(1, commands[0].arguments.size());
        CHECK_EQ(std::string("hi"), commands[0].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
        CHECK(std::string("ABC") == commands[1].command);
        CHECK_EQ(1, commands[1].arguments.size());
        CHECK_EQ(std::string("def"), commands[1].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[1].type);
    }

    SUBCASE("Query command")
    {
        const char *buffer = "AT+HELLOW=?\r\nAT+ABC=?\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(2, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(0, commands[0].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_QUERY, commands[0].type);
        CHECK(std::string("ABC") == commands[1].command);
        CHECK_EQ(0, commands[1].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_QUERY, commands[1].type);
    }

    SUBCASE("Test command")
    {
        const char *buffer = "AT+HELLOW?\r\nAT+ABC?\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(2, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(0, commands[0].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_TEST, commands[0].type);
        CHECK(std::string("ABC") == commands[1].command);
        CHECK_EQ(0, commands[1].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_TEST, commands[1].type);
    }

    SUBCASE("Execute command")
    {
        const char *buffer = "AT+HELLOW\r\nAT+DEF\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(2, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(0, commands[0].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_EXECUTE, commands[0].type);
        CHECK(std::string("DEF") == commands[1].command);
        CHECK_EQ(0, commands[1].arguments.size());
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_EXECUTE, commands[1].type);
    }
}