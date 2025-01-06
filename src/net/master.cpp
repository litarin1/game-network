// master server
#include "master.hpp"

#include <arpa/inet.h>  // inet_aton()
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "general.hpp"

Master::NetError Master::start() {
    // create socket
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0) {
        ::close(m_sockfd);
        return NetError::SOCKET_ERR;
    }
    // fill address struct for bind
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // bind socket
    if (bind(m_sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        ::close(m_sockfd);
        return NetError::SOCKET_BIND_ERR;
    }
    // listen for incoming connections
    if (listen(m_sockfd, 4) != 0) {
        ::close(m_sockfd);
        return NetError::LISTEN_ERR;
    }

    // accept
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int connfd = accept(m_sockfd, (sockaddr*)&client, &len);

    // user connection handler
    _on_connection_accept(connfd);
    close();
    return NetError::OK;
}
Master::~Master() { close(); }
void Master::close(){
    if (m_sockfd) {
        ::close(m_sockfd);
        m_sockfd = 0;
    }
    printf("\nclosed\n");
}

enum class WordType {
    UNKNOWN = -1,  // "waavavwfhd uga booga" (unknown word)
    ERROR = -2,    // "hey my version^\r\n" e.g. CR_VERSION^\r\n
    VERSION = 0,  // "hey my version is 1.2.3 is it new enough" CR_VERSION 1.2.3
};
WordType get_word(const char byte) {
    switch ((Master::ClientCommandCodes)byte) {
        case Master::CR_VERSION:
            return WordType::VERSION;
        default:
            return WordType::UNKNOWN;
    }
}

void Master::_on_connection_accept(int fd){
    char command_buff[MAX_REQUEST_LENGTH];
    uint8_t command_buff_pos = 0;
    char socket_buff[MAX_REQUEST_LENGTH];
    for (;;){
        // receive data until \r\n or MAX_REQUEST_LENGTH exceeding
        bzero(command_buff, sizeof(command_buff));
        for (;;) {
            // fetch data
            bzero(socket_buff, sizeof(socket_buff));
            int n = recv(fd, socket_buff, sizeof(socket_buff), 0);
            if (n == 0) break;  // end of connection
            if (n <= 0) {
                printf("recv() == %d; errno=%d (%s)\n", n, errno,
                       strerror(errno));
                if (errno == 0) break;
                return;
            }
            
            // write to command_buff
            if (command_buff_pos + n > sizeof(command_buff)) break;
            strncpy(&command_buff[command_buff_pos], socket_buff, n);
            command_buff_pos += n;
        }
        printf("%d: %s\n", command_buff_pos, command_buff);

        // find \r\n
        // (find \n and if previous character was \r)
        uint8_t pos = 0;
        while (pos != sizeof(command_buff) && command_buff[pos] != '\n') pos++;
        // continue if not \r
        if (!(pos > 0 && command_buff[pos-1] == '\r')){
            printf("\\r\\n not found at pos: %d\n", pos);
            continue;
        }
        printf("pos: %d\n", pos);

        // get command from the string
        /*
        1. найти слово
        2. получить тип слова
        3. в зависимости от типа слова совершить действие (continue/break/отправить ответ/close(fd))
        */
        uint8_t sep_pos = 0, old_sep_pos = 0;
        enum WhatAreWeLookingFor{COMMAND, VERSION} what = COMMAND;
        // найти слово..
        while (sep_pos != pos){
            printf("f");
            if (command_buff[sep_pos] != COMMAND_SEPARATOR_CHAR){
                sep_pos++;
                continue;
            }
            // нашли место где находится слово!
            // прочитать слово..
            // find word in range command_buff[old_sep_pos]..command_buff[sep_pos]
            switch (what){
                case COMMAND:
                    printf("case COMMAND:\n");
                    switch (get_word(command_buff[old_sep_pos])){
                        case WordType::VERSION:
                            printf("  case WordType::VERSION:\n");
                            what = VERSION;
                            break;
                        case WordType::UNKNOWN:
                        case WordType::ERROR:
                            printf("  case WordType::UNKNOWN:\n");
                            break;
                    }
                    break;
                // 4 bytes (0..4'294'967'296)
                // i guess there will not be that much versions
                // version 3338 (\r\n) should not exist
                case VERSION:
                    printf("case VERSION:\n");
                    // is there even 4 bytes
                    if (pos-old_sep_pos < 4){
                        what = COMMAND;
                        break;
                    }
                    uint32_t version = *reinterpret_cast<uint32_t*>(&command_buff[old_sep_pos]);
                    if (version != _GAME_VERSION_){
                        const char reply_tmp[2] {static_cast<char>(SA_VERSION_OLD)};
                        send(fd, reply_tmp, 2, 0);
                        return;
                    }
                    const char reply_tmp[2] {static_cast<char>(SA_OK)};
                    send(fd, reply_tmp, 2, 0);
                    what = COMMAND;
                    break;
            }
            // прочитали слово и даже ответили!

            // ищем следующее слово..
            sep_pos++; // sep_pos был на ^, теперь на следующем символе
            old_sep_pos = sep_pos;
        }
        // слова закончились блинблинблин
        printf("ENDENDENDEND");
        exit(0);
    }
}