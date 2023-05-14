#include "parser.h"
#include <fcntl.h>
#include <sys/stat.h>

#define TARGET_FILE "./settings.json"

int main(int argc, char const *argv[]) {
    int a_file = open(TARGET_FILE, 0);
    struct stat s;
    
    if (fstat(a_file, &s) == -1) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }

    char *fullbuffer = (char *) malloc(s.st_size + 1);
    fullbuffer[s.st_size] = NULL_TERMINATOR;

    if (read(a_file, fullbuffer, s.st_size) == -1 ) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    struct json_ent *parsed = parse_json(fullbuffer);

    printf("Done. No errors.\n");
    exit(EXIT_SUCCESS);
}
