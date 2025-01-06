// master server
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>

#include <forward_list>

// 128 chars + \n\0
#define MAX_REQUEST_LENGTH 130
#define COMMAND_SEPARATOR_CHAR '^'  // VERSION^1.3.2


struct Host {
    enum Type { MASTER, LOBBY, GAME, UNKNOWN } type;
    enum Status { OK = 0, UNAVAILABLE } status;
    in_port_t port;
    in_addr_t ip;  /// ipv4
    // TODO: replace port and ip with sockaddr_in
};

/// @brief Master server
class Master {
private:
    int m_sockfd;
    in_port_t port = 20202;
    enum class Status { OK = 0, STOPPED, STARTING, STOPPING, FATAL_ERROR } status;

    std::forward_list<Host> known_hosts{};


public:
    void _on_connection_accept(int connfd);
    enum NetError { OK = 0, SOCKET_ERR, SOCKET_BIND_ERR, LISTEN_ERR};
    NetError start();
    void close();

    /// @brief check all servers, described in database (servers.txt)
    void fetch_servers();

    Status getStatus() const { return status; }

    Master(const in_port_t port,
           const std::forward_list<Host>& known_hosts = {})
        : port(port), known_hosts(known_hosts) {}
    ~Master();


    /// @brief commands that are sended in socket
    /// @attention must be observed by both client and server
    enum ClientCommandCodes : int8_t {
        // Client Requests
        CR_VERSION = 'a',
    };
    /// @copydoc ClientCommandCodes
    enum ServerCommandCodes : int8_t {
        // Server Answers
        SA_OK,
        SA_BAD_REQUEST,
        SA_VERSION_OLD,
    };
    /*
      Single Message structure:
      [1 byte/uint8]      |CommandCodes|
      [any length]        |any value| (optional)
      [1 byte/char]       |COMMAND_SEPARATOR_CHAR|
      [2 bytes/char+char] |\r\n|

    */
};