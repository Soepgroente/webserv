#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#define PORT 3000
#define BUFFER_SIZE 1024
#define MAX_CLIENT 10

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::memset(buffer, 0, BUFFER_SIZE);

    // Non-blocking read
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_read < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // Non-blocking modda, veri yoksa hemen döner
            printf("\033[31mrecv would block\n\033[0m");
            return;
        } else {
            perror("recv failed");
            close(client_socket);
            return;
        }
    }
    std::cout << "Received request:\n" << buffer << std::endl;

    // Basit bir HTTP yanıtı oluşturma
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
    ssize_t bytes_written = send(client_socket, response.c_str(), response.size(), 0);
    if (bytes_written < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // Non-blocking modda, yazma yeri yoksa hemen döner
            printf("\033[31msend would block\n\033[0m");
            return;
        } else {
            perror("send failed");
            close(client_socket);
            return;
        }
    }

    // Bağlantıyı kapatma
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Soket oluşturma
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Soket seçeneklerini ayarlama
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Adres ve port ayarları
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Soketi belirli bir adrese ve porta bağlama
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Gelen bağlantıları dinlem
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "\033[34mServer is listening on port \033[36m" << PORT << "\033[0m" << std::endl;

    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[MAX_CLIENT];
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    int nfds = 1;
    
    while (true) {
        int poll_count = poll(fds, nfds, 5000);
        if (poll_count == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) {
                    // Gelen bağlantıları kabul etme
                    client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                    if (client_socket < 0) {
                        if (errno == EWOULDBLOCK || errno == EAGAIN) {
                            // Non-blocking modda, kabul edilecek bağlantı yoksa hemen döner
                            printf("\033[31maccept would block\n");
                            continue;
                        } else {
                            perror("accept");
                            exit(EXIT_FAILURE);
                        }
                    }

                    std::cout << "New connection, socket fd is " << client_socket << " int i = " << i << std::endl;

                    int client_flags = fcntl(client_socket, F_GETFL, 0);
                    if (client_flags == -1) {
                        perror("fcntl F_GETFL");
                        exit(EXIT_FAILURE);
                    }
                    if (fcntl(client_socket, F_SETFL, client_flags | O_NONBLOCK) == -1) {
                        perror("fcntl F_SETFL");
                        exit(EXIT_FAILURE);
                    }

                    fds[nfds].fd = client_socket;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } else {
                    handle_client(fds[i].fd);
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    nfds--;
                }
            }
        }
    }
    return 0;
}