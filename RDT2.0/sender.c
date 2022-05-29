#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int set_socket() {
    int socket_descriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_descriptor == -1) {
        perror("Unable to initialize socket");
        exit(1);
    }
    return socket_descriptor;
}

struct sockaddr_in set_address(int port, int isRcv) {
    struct sockaddr_in server_address;
    server_address.sin_port = htons(port);
    server_address.sin_family = AF_INET;
    if(isRcv==0) {
        int addr_conversion_ret_code = inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
        if(addr_conversion_ret_code<1) {
            perror("Error on address conversion");
            exit(1);
        }
    }
    else server_address.sin_addr.s_addr=INADDR_ANY;
    return server_address;
}

void check_bind(int socket_descriptor, struct sockaddr_in address, socklen_t address_length) {
    int bindCode = bind(socket_descriptor, (struct sockaddr*) &address, address_length);
    if(bindCode==-1) {
        perror("Error on bind");
        exit(1);
    }
}

char* make_pkt(char* data, char* checksum) {
    char* pkt;
    pkt = malloc(sizeof(data) + sizeof(checksum) + 1);
    strcpy(pkt, data);
    strcat(pkt, checksum);
    return pkt;
}

void udt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    if(rand()%2==0) {
        char* pkt;
        pkt = malloc(sizeof(data)+ 1);
        strcpy(pkt, data);
        pkt[4] = '1';
        sendto(socket_descriptor, pkt, sizeof(pkt), 0, (struct sockaddr*) &server_address, server_address_length);
    }
    else sendto(socket_descriptor, data, sizeof(data), 0, (struct sockaddr*) &server_address, server_address_length);
}

void rdt_rcv(char* rcvpkt) {
    int socket_descriptor_server = set_socket();
    struct sockaddr_in server_address = set_address(8000, 1);

    socklen_t address_length = sizeof(server_address);
    check_bind(socket_descriptor_server, server_address, address_length);
    
    ssize_t rcvpkt_length = sizeof(rcvpkt);
    recv(socket_descriptor_server, rcvpkt, sizeof(rcvpkt), 0);
    printf("%s\n", rcvpkt);
    close(socket_descriptor_server);
}

int isNACK(char* rcvpkt) {
    if(rcvpkt[0]=='N') return 1;
    else return 0;
}

int isACK(char* rcvpkt) {
    if(rcvpkt[0]=='A') return 1;
    else return 0;
}

void rdt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    char* checksum = "0";
    char rcvpkt[1024];
    char* sndpkt;
    int retransmit = 0;
    while(1) {
        if(isACK(rcvpkt)) retransmit = 0;
        if(isNACK(rcvpkt)) {
            printf("NACK received\nRetransmitting last packet...\n");
            sleep(2);
            udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            rdt_rcv(rcvpkt);
            retransmit = 1;
        }
        if(retransmit==0) {
            sndpkt = make_pkt(data, checksum);
            udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            rdt_rcv(rcvpkt);
        }
        sleep(2);
    }
}

int main() {
    printf("RDT 2.0 Sender\n");

    int socket_descriptor = set_socket();   
    struct sockaddr_in server_address = set_address(9000, 0);
    socklen_t server_address_length = sizeof(server_address);
    
    char* data = "DATA";
    rdt_send(data, socket_descriptor, server_address, server_address_length);
    return 0;
}