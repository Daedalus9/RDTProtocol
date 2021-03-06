#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

void extract_packet(int socket_descriptor, char* data) {
    recv(socket_descriptor, data, sizeof(data), 0);
}

void deliver_data(char* data) {
    printf("%s\n", data);
}

void rdt_rcv(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    while (1)
    {
        extract_packet(socket_descriptor, data);
        deliver_data(data);
    }
}

int set_socket() {
    int socket_descriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor == -1) {
        perror("Unable to initialize socket");
        exit(1);
    }
    return socket_descriptor;
}

struct sockaddr_in set_address(int port) {
    struct sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    return address;
}

void check_bind(int socket_descriptor, struct sockaddr_in address, socklen_t address_length) {
    int bind_code = bind(socket_descriptor, (struct sockaddr*) &address, address_length);
    if (bind_code == -1) {
        perror("Error on bind\n");
        exit(1);
    }
}

int main() {
    printf("RDT 1.0 Receiver\n");

    int socket_descriptor = set_socket();
    struct sockaddr_in address = set_address(9000);
    socklen_t address_length = sizeof(address);

    check_bind(socket_descriptor, address, address_length);

    char msg_buffer[1024];
    ssize_t msg_length = sizeof(msg_buffer);
    rdt_rcv(msg_buffer, socket_descriptor, address, address_length);
}