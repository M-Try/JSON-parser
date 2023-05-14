#include "./json.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define new(type) (type *) malloc(sizeof(type))

#define SEQUENCE_NOT_FOUND 0
#define SEQUENCE_FOUND 1

#define PM_STRINGKEY 0
#define PM_VALUE 1

#define NULL_TERMINATOR (char) '\0'

#define dbg_print(str) printf("[at line %d] %s\n", __LINE__, str);

typedef struct {
    char *head;
    uint8_t mode;
    struct json_ent *json_begin;
    struct json_ent *json_head;
} parser_state;


int advance(parser_state *ps, char maybe_next) {
    if (!(*(ps->head) == maybe_next)) {
        return SEQUENCE_NOT_FOUND;
    }
    ps->head++;
    return SEQUENCE_FOUND;
}

void create_json_ent(parser_state *ps) {
    struct json_ent *newent = new(struct json_ent);
    ps->json_head->next = (uint64_t) newent;
    ps->json_head = newent;
}

// --------------- Only parsing from here on


void parse_whitespace(parser_state *ps) {
    while (advance(ps, ' ') || advance(ps, '\t') || advance(ps, '\n') || advance(ps, '\r'));
}

int parse_bool_null(parser_state *ps) {
    if (strncmp(ps->head, "true", 4) == 0) {
        ps->json_head->val.type = JSON_BOOL;
        ps->json_head->val.value = 1;

        ps->head += 4;
        return SEQUENCE_FOUND;
    }
    if (strncmp(ps->head, "false", 5) == 0) {
        ps->json_head->val.type = JSON_BOOL;
        ps->json_head->val.value = 0;

        ps->head += 5;
        return SEQUENCE_FOUND;
    }
    if (strncmp(ps->head, "null", 4) == 0) {
        ps->json_head->val.type = JSON_NULL;
        ps->json_head->val.value = 0;

        ps->head += 4;
        return SEQUENCE_FOUND;
    }

    return SEQUENCE_NOT_FOUND;
}

int parse_number(parser_state *ps) {
    size_t len = strcspn(ps->head, " \t\n\r,}]");
    if (len == NULL) {
        return SEQUENCE_NOT_FOUND;
    }

    char *ch = (char *) malloc(len + 1);
    memcpy(ch, ps->head, len);
    ch[len] = NULL_TERMINATOR;
    ps->json_head->val.value = atof(ch);
    ps->json_head->val.type = JSON_NUM;

    ps->head += len;
    free(ch);

    return SEQUENCE_FOUND;
}

int parse_string(parser_state *ps) {
    if (!advance(ps, '"')) return SEQUENCE_NOT_FOUND;
    
    // TODO: check for malformed strings!!!!!
    size_t len = (int) strchr(ps->head, '"') - (int) ps->head;
    char *stringresult = (char *) malloc(len + sizeof(NULL_TERMINATOR));
    memcpy(stringresult, ps->head, len);
    stringresult[len] = NULL_TERMINATOR;
    
    ps->head += len + 1; // skip the last double quote
    if (ps->mode == PM_STRINGKEY) {
        ps->json_head->name = stringresult;
    }
    else if (ps->mode == PM_VALUE) {
        ps->json_head->val.type = JSON_STRING;
        ps->json_head->val.value = (uint64_t) stringresult;
    }

    return SEQUENCE_FOUND;
}
// forward declaration so c doesnt piss itself (hot) trying to resolve the symbol in the parse_object()
int parse_value(parser_state *ps);

int parse_object(parser_state *ps) {
    if (!advance(ps, '{')) return SEQUENCE_NOT_FOUND;
    parse_whitespace(ps);

    struct json_ent *orig_ent = ps->json_head;

    ps->json_head->val.type = JSON_OBJ;

    struct json_ent *newent = new(struct json_ent);
    ps->json_head->val.value = (uint64_t) newent;
    ps->json_head = newent;

    ps->mode = PM_STRINGKEY;
    while (parse_string(ps) == SEQUENCE_FOUND) {
        parse_whitespace(ps);
        if (!advance(ps, ':')) return SEQUENCE_NOT_FOUND;
        parse_whitespace(ps);
        ps->mode = PM_VALUE;
        if (parse_value(ps) == SEQUENCE_NOT_FOUND) return SEQUENCE_NOT_FOUND;
        create_json_ent(ps);
        parse_whitespace(ps);
        if (!advance(ps, ',')) break;
        parse_whitespace(ps);
        ps->mode = PM_STRINGKEY;
    }

    ps->json_head = orig_ent;

    if (!advance(ps, '}')) return SEQUENCE_NOT_FOUND;

    return SEQUENCE_FOUND;
}

int parse_array(parser_state *ps) {
    if (!advance(ps, '[')) return SEQUENCE_NOT_FOUND;
    parse_whitespace(ps);

    struct json_ent *orig_ent = ps->json_head;

    ps->json_head->val.type = JSON_ARRAY;
    struct json_ent *newent = new(struct json_ent);
    ps->json_head->val.value = (uint64_t) newent;
    ps->json_head = newent;

    ps->mode = PM_VALUE;
    while (parse_value(ps) == SEQUENCE_FOUND) {
        create_json_ent(ps);
        parse_whitespace(ps);
        if (!advance(ps, ',')) break;
        parse_whitespace(ps);
    }

    ps->json_head = orig_ent;

    if (!advance(ps, '}')) return SEQUENCE_NOT_FOUND;

    return SEQUENCE_FOUND;
}

int parse_value(parser_state *ps) {
    ps->mode = PM_VALUE;
    if (
        parse_object(ps) ||
        parse_array(ps) ||
        parse_bool_null(ps) ||
        parse_string(ps) ||
        parse_number(ps)
    ) return SEQUENCE_FOUND;

    return SEQUENCE_NOT_FOUND;
}

struct json_ent *parse_json(char *text) {
    struct json_ent *ent = new(struct json_ent);
    parser_state *ps = new(parser_state);
    
    ps->head = text;
    ps->json_begin = ent;
    ps->json_head = ent;
    ps->mode = PM_STRINGKEY;

    // do the actual parsing
    ps->json_begin->name = ""; // empty string because the global scope object doesnt have a string key
    int parser_status = parse_object(ps);

    ps->json_head->next = NULL;
    free(ps);

    if (parser_status == SEQUENCE_NOT_FOUND) {
        free(ent);
        printf("Bad JSON\n");
        return NULL;
    }

    return ent;
}
