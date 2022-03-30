#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int corrupt(char* data) {
    if(data[5] - '0' ==1) {
        return 1;
    }
    else return 0;
}

void udt_send(char* data) {
    int socket_descriptor_client = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(8000);

    socklen_t client_address_length = sizeof(client_address);
    int addr_conversion_ret_code = inet_pton(AF_INET, "127.0.0.1", &client_address.sin_addr);
    
    if(addr_conversion_ret_code<1) {
        perror("Error on address conversion");
        exit(1);
    }
    sendto(socket_descriptor_client, data, sizeof(data), 0, (struct sockaddr*) &client_address, client_address_length);
    close(socket_descriptor_client);
}

void extract_packet(int socket_descriptor, char* data) {
    recv(socket_descriptor, data, sizeof(data), 0);
}

void deliver_data(char* data) {
    printf("%s\n", data);
}

void rdt_rcv(char* data, int socket_descriptor, struct sockaddr_in client_address, socklen_t client_address_length) {
    while (1)
    {
        recv(socket_descriptor, data, sizeof(data), 0);
        if(corrupt(data)==1) {
            deliver_data(data);
            sleep(2);
            udt_send("NACK");
        }
        else {
            deliver_data(data);
            sleep(2);
            udt_send("ACK");
        }
    }
}

int main() {
    printf("RDT 2.1 Server\n");

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