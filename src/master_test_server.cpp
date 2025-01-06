/// net/master server TUI test
// TODO: close sockets on Ctrl+C and other signals
#include <arpa/inet.h>  // inet_aton()
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <net/master.hpp>
#include <stdio.h>
#include <stdlib.h>

in_addr_t make_ip(const char* str_in) {
    in_addr out;
    inet_aton(str_in, &out);
    return out.s_addr;
}

std::forward_list<Host> initial_hosts = {
    {Host::MASTER, Host::OK, 20202, make_ip("127.0.0.1")}
};
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    printf("port: %d\n", port);

    Master server{port, initial_hosts};
    Master::NetError err = server.start();
    printf("NetError: %d\n", err);
}
