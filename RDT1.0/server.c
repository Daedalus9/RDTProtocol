#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

void rdt_rcv(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    while (1)
    {
        recv(socket_descriptor, data, sizeof(data), 0);
        printf("%s\n", data);
    }
}

int main() {
    printf("RDT 1.0 Server\n");

    int socket_descriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor == -1) {
        perror("Unable to initialize socket");
        exit(1);
    }

    struct sockaddr_in address;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_family=AF_INET;
    address.sin_port=htons(9000);

    socklen_t address_length = sizeof(address);

    int bindCode = bind(socket_descriptor, (struct sockaddr*) &address, address_length);
    if(bindCode==-1) {
        perror("Error on bind");
        exit(1);
    }

    char msg_buffer[1024];
    ssize_t msg_length = sizeof(msg_buffer);
    rdt_rcv(msg_buffer, socket_descriptor, address, address_length);
}