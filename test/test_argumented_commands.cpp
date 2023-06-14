#include "doctest.h"
#include <string.h>
#include <string>
#include <vector>
#include "at_parser/at_parser.h"
#include "parser_helpers.h"

TEST_CASE("Set AT Commands")
{
    at_parser_handle_t handle = nullptr;
    commands.clear();
    CHECK_EQ(0, at_parser_create(&handle, 100, '\x1B', ','));

    SUBCASE("Single command registered and in buffer. Unquoted argument.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=hello_world\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(1, commands[0].arguments.size());
        CHECK_EQ(std::string("hello_world"), commands[0].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Single command registered and in buffer. Quoted argument.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"hello_world\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(1, commands[0].arguments.size());
        CHECK_EQ(std::string("hello_world"), commands[0].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Single command registered and in buffer. Quoted argument with escaped quote center.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"hello\x1B\"_world\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(1, commands[0].arguments.size());
        CHECK_EQ(std::string("hello\"_world"), commands[0].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Single command registered and in buffer. Quoted argument with escaped quote end.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"hello_world\x1B\"\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(1, commands[0].arguments.size());
        CHECK_EQ(std::string("hello_world\""), commands[0].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Single command registered and in buffer. Quoted argument with escaped quote start.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"hello_world\x1B\"\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(1, commands[0].arguments.size());
        CHECK_EQ(std::string("hello_world\""), commands[0].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Single command registered and in buffer. Quoted argument with escaped quote at start, end & center.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"\x1B\"hello\x1B\"_world\x1B\"\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(1, commands[0].arguments.size());
        CHECK_EQ(std::string("\"hello\"_world\""), commands[0].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Multiple commands registered and single in buffer. Quoted argument.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "ABC", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "DEF", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"hello_world\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(1, commands[0].arguments.size());
        CHECK_EQ(std::string("hello_world"), commands[0].arguments[0]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Multiple commands registered and single in buffer. Multiple mixed quote arguments.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "ABC", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "DEF", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"hello_world\",another,\"parameter\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(3, commands[0].arguments.size());
        CHECK_EQ(std::string("hello_world"), commands[0].arguments[0]);
        CHECK_EQ(std::string("another"), commands[0].arguments[1]);
        CHECK_EQ(std::string("parameter"), commands[0].arguments[2]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Multiple commands registered and single in buffer. Multiple mixed quote arguments. With a separator in a single quoted argument.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "ABC", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "DEF", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"hello,world\",another,\"parameter\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(1, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(3, commands[0].arguments.size());
        CHECK_EQ(std::string("hello,world"), commands[0].arguments[0]);
        CHECK_EQ(std::string("another"), commands[0].arguments[1]);
        CHECK_EQ(std::string("parameter"), commands[0].arguments[2]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    }

    SUBCASE("Multiple commands registered and multiple in buffer. Multiple mixed quote arguments. With a separator in a single quoted argument.")
    {
        CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "ABC", at_parser_default_received_command, NULL));
        CHECK_EQ(0, at_parser_add_command_handler(handle, "DEF", at_parser_default_received_command, NULL));
        const char *buffer = "AT+HELLOW=\"hello,world\",another,\"parameter\"\r\nAT+ABC=next_arg,\"some,extra\",\"escaped\x1B\"quote\"\r\n";
        CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
        CHECK_EQ(2, commands.size());
        CHECK(std::string("HELLOW") == commands[0].command);
        CHECK_EQ(3, commands[0].arguments.size());
        CHECK_EQ(std::string("hello,world"), commands[0].arguments[0]);
        CHECK_EQ(std::string("another"), commands[0].arguments[1]);
        CHECK_EQ(std::string("parameter"), commands[0].arguments[2]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);

        CHECK_EQ(3, commands[1].arguments.size());
        CHECK_EQ(std::string("next_arg"), commands[1].arguments[0]);
        CHECK_EQ(std::string("some,extra"), commands[1].arguments[1]);
        CHECK_EQ(std::string("escaped\"quote"), commands[1].arguments[2]);
        CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[1].type);
    }

    at_parser_free(handle);
}
