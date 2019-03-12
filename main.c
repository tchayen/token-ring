#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>

// SOURCES:
// -> http://www.microhowto.info/howto/listen_for_and_accept_tcp_connections_in_c.html
// -> https://stackoverflow.com/questions/12763268/why-is-bind-used-in-tcp-why-is-it-used-only-on-server-side-and-not-in-client
// -> https://stackoverflow.com/questions/34073871/socket-programming-whats-the-difference-between-listen-and-accept
// -> https://stackoverflow.com/questions/489036/how-does-the-socket-api-accept-function-work

// DOCUMENTATION SNIPPETS

// -> https://linux.die.net/man/3/exit
// exit() function causes normal process termination and the value of status & 0377 is returned to the parent.

// -> https://linux.die.net/man/3/htonl
// htons() function converts the unsigned short integer hostshort from host byte order to network byte order.
// htonl() function converts the unsigned integer long from host byte order to network byte order.

// -> https://linux.die.net/man/3/strtol
// strtol() function converts the initial part of the string in nptr to a long integer value according to the given
// base, which must be between 2 and 36 inclusive, or be the special value 0.

// -> https://linux.die.net/man/3/inet_addr
// inet_addr() function converts the Internet host address cp from IPv4 numbers-and-dots notation into binary data in
// network byte order.

// -> http://man7.org/linux/man-pages/man2/connect.2.html
// connect() system call connects the socket referred to by the file
// descriptor sockfd to the address specified by addr.

// -> https://linux.die.net/man/3/strtok
// strtok() function parses a string into a sequence of tokens. On the first call to strtok() the string to be parsed
// should be specified in str. In each subsequent call that should parse the same string, str should be NULL.

// -> http://man7.org/linux/man-pages/man2/accept.2.html
// accept() system call is used with connection-based socket types (SOCK_STREAM, SOCK_SEQPACKET). It extracts the first
// connection request on the queue of pending connections for the listening socket, sockfd, creates a new connected
// socket, and returns a new file descriptor referring to that socket. The newly created socket is not in the listening
// state. The original socket sockfd is unaffected by this call.

// -> https://linux.die.net/man/2/read
// read() attempts to read up to count bytes from file descriptor fd into the buffer starting at buf.

// -> https://linux.die.net/man/2/write
// write() writes up to count bytes from the buffer pointed buf to the file referred to by the file descriptor fd.

// -> https://linux.die.net/man/2/close
// close() closes a file descriptor, so that it no longer refers to any file and may be reused.

#define TRUE 1
#define FALSE 0

typedef struct sockaddr_in address;

typedef enum {
    INIT,
    MESSAGE,
    QUIT,
} MessageType;

typedef struct {
    MessageType type;
    int value;
    int next_port; // For `INIT`, means that given client wants to join *before* the one with `next_port`.
} Token;

const int ERROR_NOT_ENOUGH_ARGS = 101;
const int ERROR_INCORRECT_INITIAL_TOKEN = 102;
const int ERROR_INCORRECT_PROTOCOL = 103;
const int ERROR_CANT_CREATE_SOCKET = 104;
const int ERROR_BIND_FAILED = 105;
const int ERROR_LISTEN_FAILED = 106;
const int ERROR_ACCEPT_FAILED = 107;
const int ERROR_UNIMPLEMENTED = 108;

const int BASE_TEN = 10;
const int IP_V4 = AF_INET;
const unsigned int LOCALHOST_IP = htonl(INADDR_ANY);

char *CLIENT_NAME;
int CLIENT_PORT;
unsigned int NEXT_IP;
int NEXT_PORT;
int HAS_TOKEN;
int USING_TCP;

void load_args(int argc, char **argv) {
    if (argc < 5) exit(ERROR_NOT_ENOUGH_ARGS);

    CLIENT_NAME = argv[1];
    CLIENT_PORT = (int) strtol(argv[2], NULL, BASE_TEN);

    char *next_ip_string = strtok(argv[3], ":");
    NEXT_IP = inet_addr(next_ip_string);
    NEXT_PORT = (int) strtol(strtok(NULL, ":"), NULL, BASE_TEN);

    if (strcmp(argv[4], "true") == 0) {
        HAS_TOKEN = TRUE;
    } else if (strcmp(argv[4], "false") == 0) {
        HAS_TOKEN = FALSE;
    } else {
        exit(ERROR_INCORRECT_INITIAL_TOKEN);
    }

    if (strcmp(argv[5], "tcp") == 0) {
        USING_TCP = TRUE;
    } else if (strcmp(argv[5], "udp") == 0) {
        USING_TCP = FALSE;
    } else {
        exit(ERROR_INCORRECT_PROTOCOL);
    }

    printf(
            "{\n"
            "    \"clientName\": \"%s\",\n"
            "    \"clientPort\": %d,\n"
            "    \"next\": \"%s:%d\",\n"
            "    \"hasToken\": %s,\n"
            "    \"protocol\": \"%s\",\n"
            "}\n",
            CLIENT_NAME,
            CLIENT_PORT,
            next_ip_string,
            NEXT_PORT,
            HAS_TOKEN ? "true" : "false",
            USING_TCP ? "tcp" : "udp"
    );
}

address get_addr(unsigned int converted_ip, int port) {
    address addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = IP_V4;
    addr.sin_port = htons((__uint16_t) port);
    addr.sin_addr.s_addr = converted_ip;
    return addr;
}

int get_socket_fd() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) exit(ERROR_CANT_CREATE_SOCKET);
    return socket_fd;
}

void call_bind(int socket_fd, address addr) {
    if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) != 0) exit(ERROR_BIND_FAILED);
}

void call_listen(int socket_fd) {
    if (listen(socket_fd, SOMAXCONN) != 0) exit(ERROR_LISTEN_FAILED);
}

int call_connect(int socket, address addr) {
    return connect(socket, (const struct sockaddr *) &addr, sizeof(addr));
}

int call_accept(int socket_fd) {
    int new_socket = accept(socket_fd, NULL, NULL);
    if (new_socket == -1) exit(ERROR_ACCEPT_FAILED);
    return new_socket;
}

int main(int argc, char **argv) {
    load_args(argc, argv);

    if (USING_TCP) {
        address client_addr = get_addr(LOCALHOST_IP, CLIENT_PORT);
        int client_socket_fd = get_socket_fd();
        call_bind(client_socket_fd, client_addr);
        call_listen(client_socket_fd);

        address next_addr = get_addr(NEXT_IP, NEXT_PORT);
        int next_socket_fd = get_socket_fd();
        call_connect(next_socket_fd, next_addr);

        if (HAS_TOKEN) {
            Token token;
            memset(&token, 0, sizeof(token));
            token.type = INIT;
            token.next_port = NEXT_PORT;
            write(next_socket_fd, &token, sizeof(token));
        } else {
            // wait for INIT
        }

        int stop = FALSE;
        while (!stop) {
            Token token;
            memset(&token, 0, sizeof(token));
            client_socket_fd = call_accept(client_socket_fd);
            read(client_socket_fd, &token, sizeof(token));
            switch (token.type) {
                case INIT:
                    break;
                case MESSAGE:
                    token.value = rand();
                    write(next_socket_fd, &token, sizeof(token));
                    break;
                case QUIT:
                    stop = TRUE;
                    break;
            }
            sleep(1000);
        }
        close(client_socket_fd);
        close(next_socket_fd);
    } else {
        exit(ERROR_UNIMPLEMENTED);
    }

    return 0;
}