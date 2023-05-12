#include <stdint-gcc.h>

enum json_vtype {
    JSON_NULL,
    JSON_BOOL,
    JSON_STRING,
    JSON_NUM,
    JSON_OBJ, // necessarily also implies that empty objects can exist
    JSON_ARRAY
};

struct json_value {
    enum json_vtype type;
    uint64_t value; // for JSON_BOOL, JSON_NULL, JSON_NUM, this is all you need, for all others cast to pointer of appropriate type
};

struct json_ent {
    char *name; // the key of the json_value
    struct json_value val;
    struct json_ent *next;
};

/*
struct json_value json_get(struct json_ent *json_list) {
    for ()
}
*/

