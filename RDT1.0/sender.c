#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

char* make_pkt(char* data) {
    char* pkt = malloc(sizeof(data));
    strcpy(pkt, data);
    return pkt;
}

void udt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    sendto(socket_descriptor, data, sizeof(data), 0, (struct sockaddr*) &server_address, server_address_length);
}

void rdt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    while(1) {
        char* packet = make_pkt(data);
        udt_send(packet, socket_descriptor, server_address, server_address_length);
        sleep(2);
    }
}

int set_socket() {
    int socket_descriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_descriptor == -1) {
        perror("Unable to initialize socket");
        exit(1);
    }
    return socket_descriptor;
}

struct sockaddr_in set_address(int port) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    int addr_conversion_ret_code = inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
    if(addr_conversion_ret_code<1) {
        perror("Error on address conversion");
        exit(1);
    }
    return server_address;
}


int main() {

    printf("RDT 1.0 Sender\n");

    int socket_descriptor = set_socket();
    struct sockaddr_in server_address = set_address(9000);
    socklen_t server_address_length = sizeof(server_address);
    
    char* data = "DATA";
    rdt_send(data, socket_descriptor, server_address, server_address_length);
    return 0;
}