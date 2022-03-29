#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


char* make_pkt(char* data, char* checksum) {
    char* pkt;
    if(rand()%10==6) {
        char* alt_checksum="1";
        pkt = malloc(sizeof(data) + sizeof(checksum) + 1);
        strcpy(pkt, data);
        strcat(pkt, alt_checksum);
    }
    else {
        pkt = malloc(sizeof(data) + sizeof(checksum) + 1);
        strcpy(pkt, data);
        strcat(pkt, checksum);
    }
    return pkt;
}

void udt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    sendto(socket_descriptor, data, sizeof(data), 0, (struct sockaddr*) &server_address, server_address_length);
}

void rdt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    char* checksum = "0";
    while(1) {
        char* sndpkt = make_pkt(data, checksum);
        udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
        sleep(2);
    }
}

int main() {
    printf("RDT 2.0 Client\n");

    int socket_descriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_descriptor == -1) {
        perror("Unable to initialize socket");
        exit(1);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9000);

    socklen_t server_address_length = sizeof(server_address);
    int addr_conversion_ret_code = inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
    
    if(addr_conversion_ret_code<1) {
        perror("Error on address conversion");
        exit(1);
    }
    
    char* data = "DATA";
    rdt_send(data, socket_descriptor, server_address, server_address_length);
}