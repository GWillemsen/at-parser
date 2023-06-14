#include "doctest.h"
#include <string.h>
#include <string>
#include <vector>
#include "at_parser/at_parser.h"
#include "parser_helpers.h"

TEST_CASE("Test removing command handler")
{
    at_parser_handle_t handle = nullptr;
    commands.clear();
    CHECK_EQ(0, at_parser_create(&handle, 50, '\x1B', ','));
    CHECK_EQ(0, at_parser_add_command_handler(handle, "HELLOW", at_parser_default_received_command, NULL));
    CHECK_EQ(0, at_parser_add_command_handler(handle, "ABC", at_parser_default_received_command, NULL));

    const char *buffer = "AT+HELLOW=\"ghi\"\r\nAT+ABC=def\r\n";
    CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
    CHECK_EQ(2, commands.size());
    CHECK(std::string("HELLOW") == commands[0].command);
    CHECK_EQ(1, commands[0].arguments.size());
    CHECK_EQ(std::string("ghi"), commands[0].arguments[0]);
    CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    CHECK(std::string("ABC") == commands[1].command);
    CHECK_EQ(1, commands[1].arguments.size());
    CHECK_EQ(std::string("def"), commands[1].arguments[0]);
    CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[1].type);
    
    CHECK_EQ(0, at_parser_remove_command_handler(handle, "HELLOW", at_parser_default_received_command));
    commands.clear();

    CHECK_EQ(0, at_parser_process_buffer(handle, buffer, strlen(buffer)));
    CHECK_EQ(1, commands.size());
    CHECK(std::string("ABC") == commands[0].command);
    CHECK_EQ(1, commands[0].arguments.size());
    CHECK_EQ(std::string("def"), commands[0].arguments[0]);
    CHECK_EQ(AT_PARSER_COMMAND_TYPE_SET, commands[0].type);
    
    at_parser_free(handle);
}
