#include <string.h>
#include <stdio.h>

const int ERROR_NOT_ENOUGH_ARGS = 1;
const int ERROR_INCORRECT_INITIAL_TOKEN = 4;
const int ERROR_INCORRECT_PROTOCOL = 5;

char *name;
char *port;
char *next_ip;
char *next_port;
int has_token;
int using_tcp;

int load_args(int argc, char **argv) {
    if (argc < 5) return ERROR_NOT_ENOUGH_ARGS;

    name = argv[1];
    port = argv[2];

    next_ip = strtok(argv[3], ":");
    next_port = strtok(NULL, ":");

    if (strcmp(argv[4], "true") == 0) {
        has_token = 0;
    } else if (strcmp(argv[4], "false") == 0) {
        has_token = 1;
    } else {
        return ERROR_INCORRECT_INITIAL_TOKEN;
    }

    if (strcmp(argv[5], "tcp") == 0) {
        using_tcp = 0;
    } else if (strcmp(argv[5], "udp") == 0) {
        using_tcp = 1;
    } else {
        return ERROR_INCORRECT_PROTOCOL;
    }

    printf(
            "{\n    name: '%s',\n    port: %s,\n    next: %s:%s,\n    has_token: %s,\n    protocol: '%s',\n}",
            name,
            port,
            next_ip,
            next_port,
            has_token ? "true" : "false",
            using_tcp ? "tcp" : "udp"
    );
}

int main(int argc, char **argv) {
    int status = load_args(argc, argv);
    if (status != 0) return status;

    return 0;
}