#include <arpa/inet.h>  // inet_aton()
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

#include <map>
#include <net/master.hpp>

void CR_VERSION(int fd);

struct cmp_str {
    bool operator()(char const* a, char const* b) const {
        return strcmp(a, b) < 0;
    }
};
// String to Enum
static const std::map<const char*, const Master::ClientCommandCodes, cmp_str> COMMANDS_SE {
    {"VERSION", Master::CR_VERSION}
};


// TODO: fill the enum-to-string map from string-to-enum
// Enum to String
static const std::map<const Master::ClientCommandCodes, const char*>
    COMMANDS_ES{{Master::CR_VERSION, "VERSION"}};
// Enum to Function pointer
static const std::map<const Master::ClientCommandCodes, void (*)(int fd)> COMMANDS_EF {
    {Master::CR_VERSION, CR_VERSION}
};

void CR_VERSION(int fd){
    uint32_t version;
    printf("  enter version (4 bytes max): ");
    scanf("%u", &version);
    printf("sent CR_VERSION%c%u", COMMAND_SEPARATOR_CHAR, version);
    printf(" [ %c%c%.4s ]\n",
           Master::CR_VERSION,
           COMMAND_SEPARATOR_CHAR,
           &version);
}

char* toLower(char* s) {
    for (char* p = s; *p; p++) *p = tolower(*p);
    return s;
}
char* toUpper(char* s) {
    for (char* p = s; *p; p++) *p = toupper(*p);
    return s;
}


int make_socket(in_addr_t ip, in_port_t port){
    in_addr addr;
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    in_addr_t ip;
    inet_aton("127.0.0.1", reinterpret_cast<in_addr*>(&ip));
    int port = atoi(argv[1]);
    printf("port: %d\n", port);
    int fd = make_socket(ip, port);

    for (;;){
        int n;
        char cmd[33];
        bzero(cmd, sizeof(cmd));

        printf("\nEnter command (l for list): ");
        n = scanf("%32s", cmd);
        if (n < 1) continue;
        if (n == 1 && cmd[0] == 'l'){
            for (auto &&i : COMMANDS_SE){
                printf("%d: %s", i.second, i.first);
            }
            printf("\n");
            continue;
        }

        // "VERSION 1234"
        //        ^
        char buff[sizeof(cmd)];
        bzero(buff, sizeof(buff));
        sscanf(cmd, "%s", buff);
        toUpper(buff);
        auto it = COMMANDS_SE.find(buff);
        if (it == COMMANDS_SE.end()) continue;
        COMMANDS_EF.find(it->second)->second(fd);
    }
    return 0;
}