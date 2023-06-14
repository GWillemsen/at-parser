/**
 * @file at_parser.h
 * @author Giel Willemsen
 * @brief API for a basic AT command style parser.
 * @version 0.1
 * @date 2023-06-14
 * 
 * @copyright Copyright (c) 2023, See LICENSE
 * 
 */
#ifndef AT_PARSER_H
#define AT_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct at_parser* at_parser_handle_t;

/**
 * @brief The different kind of instructions that can be parsed by the parser.
 * 
 */
enum at_parser_command_type
{
    AT_PARSER_COMMAND_TYPE_QUERY,
    AT_PARSER_COMMAND_TYPE_TEST,
    AT_PARSER_COMMAND_TYPE_SET,
    AT_PARSER_COMMAND_TYPE_EXECUTE,
};

/**
 * @brief Represent a argument for a SET command that is parsed by the parser.
 * 
 */
struct at_parser_argument {
    const char* value;  ///< The start of the string (not NULL terminated).
    size_t length;      ///< The length of the string.
};

/**
 * @brief Callback that is called when a command has been parsed in the buffer.
 * 
 */
typedef void (*at_parser_received_command)(at_parser_handle_t parser, void *userdata, const char* command_name, enum at_parser_command_type type, struct at_parser_argument* argument_list, size_t argument_list_length);

/**
 * @brief Construct a new command parser.
 * 
 * @param handle The resulting handle location.
 * @param buffer_size The size of the internal AT command buffer (should be at least the length of your longest command string + \r\n).
 * @param escape_char The character that can be used to escape quote's in the set arguments.
 * @param arg_separator The character used to separate arguments in the set command.
 * @return int The success code for creating the parser. 0 on success, other on error.
 */
extern int at_parser_create(at_parser_handle_t *handle, size_t buffer_size, char escape_char, char arg_separator);

/**
 * @brief Cleans up any resources allocated by the parser.
 * 
 * @param handle The parser to delete.
 */
extern void at_parser_free(at_parser_handle_t handle);

/**
 * @brief Register a callback for when a command has parsed.
 * 
 * @param parser The parser to add the handler.
 * @param command_name The name of the AT command to listen to (AT+<command_name>).
 * @param handler The callback that should be called when the command is available.
 * @return int 0 on success, other on error.
 */
extern int at_parser_add_command_handler(at_parser_handle_t parser, const char* command_name, at_parser_received_command handler, void *userdata);

/**
 * @brief Remove a registered callback on the parser.
 * 
 * @param parser The parser to remove the callback from.
 * @param command_name The command that the callback is registered to.
 * @param handler The callback itself that should be removed.
 * @return int 0 on success, other on error.
 */
extern int at_parser_remove_command_handler(at_parser_handle_t parser, const char* command_name, at_parser_received_command handler);

/**
 * @brief Ingests the buffer and processes the current parser buffer for new commands.
 * 
 * @param parser The parser to ingest the new data.
 * @param buffer The data to ingest.
 * @param buffer_len The length of the data to ingest.
 * @return int 0 on success, other on error.
 */
extern int at_parser_process_buffer(at_parser_handle_t parser, const char* buffer, size_t buffer_len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AT_PARSER_H
