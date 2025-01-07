#pragma once
#include <arpa/inet.h>  // inet_aton()
#include <ctype.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <functional>

// 128 chars + \n\0
#define MAX_MESSAGE_LENGTH 130
#define MAX_REQUEST_LENGTH MAX_MESSAGE_LENGTH

inline void LOGERR(const char* str) {
    fprintf(stderr, "Error [%s:%d] %s\n", __FILE__, __LINE__, str);
}

template<typename...  Args>
inline void LOGERR(const char* format, const Args... args) {
    fprintf(stderr, "Error [%s:%d] ", __FILE__, __LINE__);
    fprintf(stderr, format, args...);
    fprintf(stderr, "\n");
}
// the second version should not consume disk space,
// but stackoverflow says it consumes stack memory...
// inline void LOGERR(const char* format, ...) {
//     fprintf(stderr, "Error [%s:%d] ", __FILE__, __LINE__);
//     va_list args;
//     va_start(args, format);
//     vfprintf(stderr, format, args);
//     va_end(args);
// }

/// @return socket file descriptor
int make_socket(in_addr_t ip, in_port_t port);
int make_socket(char* ip_string, in_port_t port){
    in_addr result;
    if (inet_aton(ip_string, &result) == 0){
        LOGERR("invalid ip %s, using 127.0.0.1", ip_string);
        inet_aton("127.0.0.1", &result);
    }
    return make_socket(result.s_addr, port);
}
/// @return socket file descriptor
int make_socket_server(in_port_t port);

/// @brief Functor that converts enum of command to value and pass it to Sender
/// @tparam CommandsEnum enum of network command codes
/// @tparam Sender functor that is called when send_message() invoked

/** @code
class Server {
    enum Commands {VERSION, ORDER_PIZZA};
    void _send_message(const char* message){
       send(fd, message, 0);
    }

public:
    CommandSender<Commands> send_message{_send_message};
}
@endcode
*/
template <typename CommandsEnum>
class CommandSender {
    char buff[MAX_MESSAGE_LENGTH];
    size_t i = 0;  // buff[0] is the command from CommandsEnum
    /// @brief sender function
    std::function<void(const char* message)> sender;

    

    /// @brief writes value to buff
    /// @tparam Value
    /// @param value
    /// @internal @details base function. clips data
    template <typename Value>
    void write_buffer(Value value) {
        for (char* p = &value; p < &value+sizeof(value) && i < sizeof(buff); i++, p++){
            buff[i] = *p;
        }
        i++;
        if (i == sizeof(buff)) {
            buff[i] = '\0';
            return;
        }
    }
    /// @internal
    /// @brief pass value
    /// @details recursive function
    /// @tparam Value single value (used for recursion)
    /// @tparam LeftValues left values
    template <typename Value, typename... LeftValues>
    void write_buffer(Value value, LeftValues... left) {
        write_buffer(value);
        write_buffer(left...);  // recursive call
    }
public:
    // send command and optional values
    // should not be more than MAX_MESSAGE_LENGTH bytes
    template <typename... Values>
    void operator()(CommandsEnum command, Values... values) {
        i = 0;
        bzero(buff, sizeof(buff));
        write_buffer(command, values...);
        sender(buff);
    }

    /// @brief constructor
    /// @param sender function that will be called when operator() is called
    CommandSender(std::function<void(const char* message)> sender)
        : sender(sender) {}
};
