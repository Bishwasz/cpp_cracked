#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080

void handle_connection(int client_fd);

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure socket
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        int client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        if (!fork()) {
            close(server_fd);
            handle_connection(client_fd);
            close(client_fd);
            exit(0);
        }
        close(client_fd);
    }

    return 0;
}

void handle_connection(int client_fd) {
    char buffer[1024] = {0};
    read(client_fd, buffer, 1024);
    
    std::cout << "Request:\n" << buffer << std::endl;

    std::string response;

    // Basic response to GET requests
    if (strstr(buffer, "GET /hello") != nullptr) {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                   "<html><body><h1>Hello, World!</h1></body></html>";
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                   "<html><body><h1>404 - Not Found</h1></body></html>";
    }

    // Send response
    send(client_fd, response.c_str(), response.length(), 0);
}
