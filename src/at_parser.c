/**
 * @file at_parser.c
 * @author Giel Willemsen
 * @brief Implementation of a small AT command parser.
 * @version 0.1
 * @date 2023-06-14
 *
 * @copyright See LICENSE
 *
 */
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "at_parser/at_parser.h"

#ifndef min
#define min(one, two) ((one) < (two) ? (one) : (two))
#endif // min

#ifndef max
#define max(one, two) ((one) > (two) ? (one) : (two))
#endif // max

struct callback_entry
{
    at_parser_received_command callback;
    void *userdata;
    char *command;
    struct callback_entry *next;
};

typedef struct callback_entry *callback_entry_handle_t;

struct at_parser
{
    callback_entry_handle_t callbacks;
    char *buffer;
    size_t buffer_length;
    size_t buffer_used;
    char escape_char;
    char arg_separator;
};

static inline bool is_alpha_ascii(char chr)
{
    return (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'z') || (chr >= 'a' && chr <= 'Z');
}

static callback_entry_handle_t find_callback(callback_entry_handle_t start, const char *cmd, at_parser_received_command callback);
static callback_entry_handle_t find_callback_handler(callback_entry_handle_t start, const char *cmd, size_t cmd_length);
static callback_entry_handle_t get_tail(callback_entry_handle_t start);
static callback_entry_handle_t find_before(callback_entry_handle_t start, callback_entry_handle_t item);
static void remove_callback_handler(at_parser_handle_t parser, callback_entry_handle_t item);
static int add_callback_handler(at_parser_handle_t parser, const char *name, at_parser_received_command handler, void *userdata);
static int find_char(const char *str, size_t str_len, char chr);
static void remove_buffer(at_parser_handle_t parser, size_t len);
static void process_string_line(at_parser_handle_t parser, const char *str, size_t len);
static size_t get_command_length(const char *str, size_t str_len);
static bool parse_argument_list(at_parser_handle_t parser, const char *arg_list, size_t str_len, struct at_parser_argument **list, size_t *list_length);
static void free_argument_list(struct at_parser_argument *list);
static struct at_parser_argument *add_to_argument_list(struct at_parser_argument *list, size_t list_len, const char *value, size_t value_length, char escape_char);
static void sanitize_quoted_string_to(char *string, char escape_char, size_t length, char *to);
static size_t sanitize_quoted_string_length(const char *string, size_t length, char escape_char);

extern int at_parser_create(at_parser_handle_t *parser, size_t buffer_size, char escape_char, char arg_separator)
{
    at_parser_handle_t handle = calloc(1, sizeof(struct at_parser));
    if (handle == NULL)
    {
        return -1;
    }
    handle->buffer = calloc(1, buffer_size);
    if (handle->buffer == NULL)
    {
        return -1;
    }
    handle->buffer_length = buffer_size;
    handle->buffer_used = 0;
    handle->escape_char = escape_char;
    handle->arg_separator = arg_separator;
    *parser = handle;
    return 0;
}


extern void at_parser_free(at_parser_handle_t handle)
{
    if (handle != NULL)
    {
        if (handle->buffer != NULL)
        {
            free(handle->buffer);
            handle->buffer = NULL;
        }
        while(handle->callbacks != NULL)
        {
            remove_callback_handler(handle, handle->callbacks);
        }
        free(handle);
    }
}

extern int at_parser_add_command_handler(at_parser_handle_t parser, const char *command_name, at_parser_received_command handler, void *userdata)
{
    if (parser == NULL || command_name == NULL || handler == NULL)
    {
        return -1;
    }
    callback_entry_handle_t item = find_callback(parser->callbacks, command_name, handler);
    if (item == NULL)
    {
        int rc = add_callback_handler(parser, command_name, handler, userdata);
        if (rc != 0)
        {
            return -1;
        }
    }
    return 0;
}

extern int at_parser_remove_command_handler(at_parser_handle_t parser, const char *command_name, at_parser_received_command handler)
{
    if (parser == NULL || command_name == NULL || handler == NULL)
    {
        return -1;
    }
    callback_entry_handle_t callback = find_callback(parser->callbacks, command_name, handler);
    if (callback != NULL)
    {
        remove_callback_handler(parser, callback);
    }
    return 0;
}

extern int at_parser_process_buffer(at_parser_handle_t parser, const char *buffer, size_t buffer_len)
{
    if (parser == NULL || buffer == NULL)
    {
        return -1;
    }
    size_t consumed = 0;
    while (consumed != buffer_len)
    {
        size_t copy_len = min(parser->buffer_length - parser->buffer_used, buffer_len - consumed);
        if (copy_len == 0) {
            remove_buffer(parser, max(min(parser->buffer_length / 10, 1), 5)); // Drop between 1 and 5 bytes, depending on buffer size.
            continue; // There is nothing to copy now, so just ignore this iteration.
        }
        memcpy(parser->buffer + parser->buffer_used, buffer + consumed, copy_len);
        parser->buffer_used = (parser->buffer_used + copy_len);
        consumed += copy_len;
        int res = 0;
        do
        {
            res = find_char(parser->buffer, parser->buffer_used, '\r');
            if (res >= 0)
            {
                res = find_char(parser->buffer, parser->buffer_used, '\n');
                if (res >= 0)
                {
                    const size_t drop_length = res + 1;                   // could be that the '\n' is at char index 0
                    process_string_line(parser, parser->buffer, res - 1); // Remove the \r
                    remove_buffer(parser, drop_length);
                }
            }
        } while (res >= 0);
    }
    return 0;
}

static callback_entry_handle_t find_callback(callback_entry_handle_t start, const char *cmd, at_parser_received_command callback)
{
    callback_entry_handle_t current = start;
    while (current != NULL && strcmp(current->command, cmd) != 0 && current->callback != callback && current->next != NULL)
    {
        current = current->next;
    }
    if (current != NULL && strcmp(current->command, cmd) == 0 && current->callback == callback)
    {
        return current;
    }
    else
    {
        return NULL;
    }
}

static callback_entry_handle_t find_callback_handler(callback_entry_handle_t start, const char *cmd, size_t cmd_length)
{
    callback_entry_handle_t current = start;
    while (current != NULL && strncmp(current->command, cmd, cmd_length) == 0 && current->next != NULL)
    {
        current = current->next;
    }
    if (current != NULL && strncmp(current->command, cmd, cmd_length) == 0)
    {
        return current;
    }
    else
    {
        return NULL;
    }
}

static callback_entry_handle_t get_tail(callback_entry_handle_t start)
{
    callback_entry_handle_t current = start;
    while (current != NULL && current->next != NULL)
    {
        current = current->next;
    }
    return current;
}

static callback_entry_handle_t find_before(callback_entry_handle_t start, callback_entry_handle_t item)
{
    callback_entry_handle_t previous = NULL;
    callback_entry_handle_t current = start;
    while (current != item && current != NULL && current->next != NULL)
    {
        previous = current;
        current = current->next;
    }
    if (current == item)
    {
        return previous;
    }
    else
    {
        return NULL;
    }
}

static void remove_callback_handler(at_parser_handle_t parser, callback_entry_handle_t item)
{
    if (item == NULL)
    {
        return;
    }
    if (parser != NULL)
    {
        callback_entry_handle_t before = find_before(parser->callbacks, item);
        if (before != NULL)
        {
            before->next = item->next;
        }
        else
        {
            parser->callbacks = item->next;
        }
    }
    free(item->command);
    free(item);
}

static int add_callback_handler(at_parser_handle_t parser, const char *name, at_parser_received_command handler, void *userdata)
{
    callback_entry_handle_t new_item = malloc(sizeof(struct callback_entry));
    if (new_item == NULL)
    {
        return -1;
    }

    new_item->command = malloc(strlen(name) + 1);
    if (new_item->command == NULL)
    {
        free(new_item);
        return -1;
    }
    strcpy(new_item->command, name);

    new_item->callback = handler;
    new_item->userdata = userdata;
    new_item->next = NULL;

    if (parser->callbacks == NULL)
    {
        parser->callbacks = new_item;
    }
    else
    {
        get_tail(parser->callbacks)->next = new_item;
    }
    return 0;
}

static int find_char(const char *str, size_t str_len, char chr)
{
    size_t index = 0;
    while (str[index] != chr && index < str_len)
    {
        index++;
    }
    if (str[index] == chr)
    {
        return (int)index;
    }
    else
    {
        return -1;
    }
}

static void remove_buffer(at_parser_handle_t parser, size_t len)
{
    size_t remove_len = parser->buffer_used > len ? len : parser->buffer_used;
    memcpy(parser->buffer, parser->buffer + len, parser->buffer_used - remove_len);
    parser->buffer_used -= remove_len;
}

static void process_string_line(at_parser_handle_t parser, const char *str, size_t len)
{
    if (len < 4)
    {
        return;
    }
    if (str[0] != 'A' || str[1] != 'T' || str[2] != '+')
    {
        return;
    }
    const char *command_start = str + 3; // + 3 for the AT+
    const size_t command_length = get_command_length(command_start, len);
    const size_t extra_start_at = command_length + 3;
    const size_t extra_length = len - extra_start_at;

    struct at_parser_argument *args = NULL;
    size_t arg_length = 0;
    enum at_parser_command_type type = AT_PARSER_COMMAND_TYPE_EXECUTE;
    bool error = false;
    if (extra_length == 1 && str[extra_start_at] == '?')
    {
        type = AT_PARSER_COMMAND_TYPE_TEST;
    }
    else if (extra_length == 2 && (str[extra_start_at] == '=' && str[extra_start_at + 1] == '?'))
    {
        type = AT_PARSER_COMMAND_TYPE_QUERY;
    }
    else if (extra_length >= 2 && str[extra_start_at] == '=')
    {
        parse_argument_list(parser, str + extra_start_at + 1, extra_length - 1, &args, &arg_length);
        type = AT_PARSER_COMMAND_TYPE_SET;
    }
    else if (extra_length != 0)
    {
        error = true;
    }
    if (!error)
    {
        callback_entry_handle_t item = parser->callbacks;
        do
        {
            if (item != NULL)
            {
                if (strncmp(item->command, command_start, command_length) == 0)
                {
                    if (item->callback)
                    {
                        item->callback(parser, item->userdata, item->command, type, args, arg_length);
                    }
                }
                item = item->next;
            }
        } while (item != NULL);
    }
}

static size_t get_command_length(const char *str, size_t str_len)
{
    size_t length = 0;

    while (isascii(str[length]) && isalpha((int)str[length]) && length < str_len)
    {
        length++;
    }
    return length;
}

static bool parse_argument_list(at_parser_handle_t parser, const char *arg_list, size_t str_len, struct at_parser_argument **list, size_t *list_length)
{
    if (list == NULL || arg_list == NULL || list_length == NULL)
    {
        return false;
    }
    bool finished = false;
    size_t position = 0;
    char separator = parser->arg_separator;
    char escape = parser->escape_char;
    do
    {
        bool found = false;
        int quote_count = 0;
        size_t index = position;
        while (found == false && index < str_len)
        {
            if (arg_list[index] == separator && ((quote_count % 2) == 0))
            {
                found = true;
            }
            else if (arg_list[index] == '"')
            {
                if (index == 0 || (index != 0 && arg_list[index - 1] != escape))
                {
                    quote_count++;
                }
            }
            index++;
        }
        if (found == false && str_len == index && ((quote_count % 2) != 0))
        {
            free_argument_list(*list);
            return false;
        }
        else
        {
            struct at_parser_argument *new_list = add_to_argument_list(*list, *list_length, arg_list + position, index - position - (found ? 1 : 0), escape); // If last arg then no trailing ',' otherwise compensate string length.
            if (new_list == NULL)
            {
                free_argument_list(*list);
            }
            else
            {
                *list = new_list;
            }
            *list_length = *list_length + 1;
            position = index;
        }
    } while (position < str_len);
    return true;
}

static void free_argument_list(struct at_parser_argument *list)
{
    free(list);
}
static struct at_parser_argument *add_to_argument_list(struct at_parser_argument *list, size_t list_len, const char *value, size_t value_length, char escape_char)
{
    struct at_parser_argument *new_list = NULL;
    if (list == NULL)
    {
        new_list = malloc(sizeof(struct at_parser_argument));
    }
    else
    {
        new_list = realloc(list, (list_len + 1) * sizeof(struct at_parser_argument));
    }
    if (new_list != NULL)
    {
        new_list[list_len].length = sanitize_quoted_string_length(value, value_length, '\\');
        new_list[list_len].value = malloc(new_list[list_len].length);
        if (new_list[list_len].value == NULL)
        {
            free(new_list);
            return NULL;
        }
        sanitize_quoted_string_to((char *)value, escape_char, value_length, (char *)new_list[list_len].value);
        ((char *)new_list[list_len].value)[new_list[list_len].length] = '\0';
    }
    return new_list;
}

static void sanitize_quoted_string_to(char *string, char escape_char, size_t length, char *to)
{
    size_t position = 0;
    size_t target_position = 0;
    while (position < length)
    {
        if (string[position] == escape_char)
        {
            if (position + 1 < length && string[position + 1] == '"')
            {
                to[target_position] = '"';
                target_position++;
            }
            else
            {
                to[target_position] = string[position];
                target_position++;
            }
        }
        else if (string[position] == '"')
        {
            // Ignore
        }
        else
        {
            to[target_position] = string[position];
            target_position++;
        }
        position++;
    }
}

static size_t sanitize_quoted_string_length(const char *string, size_t length, char escape_char)
{
    size_t position = 0;
    size_t count = 0;
    while (position < length)
    {
        if (string[position] == escape_char)
        {
            if (position + 1 < length && string[position + 1] == '"')
            {
                count++;
                position++; // skip the 'escape' character count.
            }
            else
            {
                count++;
            }
        }
        else if (string[position] == '"')
        {
            // Ignore
        }
        else
        {
            count++;
        }
        position++;
    }
    return count;
}
