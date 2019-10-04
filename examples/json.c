#include "json.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int json_parse_value(const char ** cursor, json_value * parent);

static void skip_whitespace(const char** cursor) {
    if (**cursor == '\0') return;
    while (iscntrl(**cursor) || isspace(**cursor)) ++(*cursor);
}

static int has_char(const char** cursor, char character) {
    skip_whitespace(cursor);
    int success = **cursor == character;
    if (success) ++(*cursor);
    return success;
}

static int json_parse_object(const char** cursor, json_value* parent) {
    json_value result = { .type = TYPE_OBJECT };
    vector_init(&result.value.object, sizeof(json_value));

    int success = 1;
    while (success && !has_char(cursor, '}')) {
        json_value key = { .type = TYPE_NULL };
        json_value value = { .type = TYPE_NULL };
        success = json_parse_value(cursor, &key);
        success = success && has_char(cursor, ':');
        success = success && json_parse_value(cursor, &value);

        if (success) {
            vector_push_back(&result.value.object, &key);
            vector_push_back(&result.value.object, &value);
        }
        else {
            json_free_value(&key);
            break;
        }
        skip_whitespace(cursor);
        if (has_char(cursor, '}')) break;
        else if (has_char(cursor, ',')) continue;
        else success = 0;
    }

    if (success) {
        *parent = result;
    }
    else {
        json_free_value(&result);
    }

    return success;
    return 1;
}

static int json_parse_array(const char** cursor, json_value* parent) {
    int success = 1;
    if (**cursor == ']') {
        ++(*cursor);
        return success;
    }
    while (success) {
        json_value new_value = { .type = TYPE_NULL };
        success = json_parse_value(cursor, &new_value);
        if (!success) break;
        skip_whitespace(cursor);
        vector_push_back(&parent->value.array, &new_value);
        skip_whitespace(cursor);
        if (has_char(cursor, ']')) break;
        else if (has_char(cursor, ',')) continue;
        else success = 0;
    }
    return success;
}


void json_free_value(json_value* val) {
    if (!val) return;

    switch (val->type) {
        case TYPE_STRING:
            free(val->value.string);
            val->value.string = NULL;
            break;
        case TYPE_ARRAY:
        case TYPE_OBJECT:
            vector_foreach(&(val->value.array), (void(*)(void*))json_free_value);
            vector_free(&(val->value.array));
            break;
    }

    val->type = TYPE_NULL;
}

int json_is_literal(const char** cursor, const char* literal) {
    size_t cnt = strlen(literal);
    if (strncmp(*cursor, literal, cnt) == 0) {
        *cursor += cnt;
        return 1;
    }
    return 0;
}

static int json_parse_value(const char** cursor, json_value* parent) {
    // Eat whitespace
    int success = 0;
    skip_whitespace(cursor);
    switch (**cursor) {
        case '\0':
            // If parse_value is called with the cursor at the end of the string
            // that's a failure
            success = 0;
            break;
        case '"':
            ++*cursor;
            const char* start = *cursor;
            char* end = strchr(*cursor, '"');
            if (end) {
                size_t len = end - start;
                char* new_string = malloc((len + 1) * sizeof(char));
                memcpy(new_string, start, len);
                new_string[len] = '\0';

                if(len != strlen(new_string)) {
          exit(100);
        }

                parent->type = TYPE_STRING;
                parent->value.string = new_string;

                *cursor = end + 1;
                success = 1;
            }
            break;
        case '{':
            ++(*cursor);
            skip_whitespace(cursor);
            success = json_parse_object(cursor, parent);
            break;
        case '[':
            parent->type = TYPE_ARRAY;
            vector_init(&parent->value.array, sizeof(json_value));
            ++(*cursor);
            skip_whitespace(cursor);
            success = json_parse_array(cursor, parent);
            if (!success) {
                vector_free(&parent->value.array);
            }
            break;
        case 't': {
            success = json_is_literal(cursor, "true");
            if (success) {
                parent->type = TYPE_BOOL;
                parent->value.boolean = 1;
            }
            break;
        }
        case 'f': {
            success = json_is_literal(cursor, "false");
            if (success) {
                parent->type = TYPE_BOOL;
                parent->value.boolean = 0;
            }
            break;
        }
        case 'n':
            success = json_is_literal(cursor, "null");
            break;
        default: {
            char* end;
            double number = strtod(*cursor, &end);
            if (*cursor != end) {
                parent->type = TYPE_NUMBER;
                parent->value.number = number;
                *cursor = end;
                success = 1;
            }
        }
    }

    return success;
}


int json_parse(const char* input, json_value* result) {
    return json_parse_value(&input, result);
}


int main(int argc, char *argv[]) {
    char my_string[BUFFER];
    json_value result = { .type = TYPE_NULL };
    int ret = -1;
    if (argc == 1) {
        char *v = fgets(my_string, BUFFER, stdin);
        if (!v) {
          exit(1);
        }
        int read = strlen(my_string);
        if (my_string[read-1] ==  '\n'){
            my_string[read-1] = '\0';
        }
    } else {
        strcpy(my_string, argv[1]);
        int read = strlen(my_string);
        if (my_string[read-1] ==  '\n'){
            my_string[read-1] = '\0';
        }
    }
    printf("val: <%s>\n", my_string);
    ret = json_parse(my_string, &result);
    json_free_value(&result);
    if (ret == 1) return 0;
    return 1;
}

