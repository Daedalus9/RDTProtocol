#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

char* make_pkt(char *flag, char* data, char* checksum) {
    char* pkt;
    pkt = malloc(sizeof(data) + sizeof(checksum) + 2);
    strcpy(pkt, flag);
    strcat(pkt, data);
    strcat(pkt, checksum);
    return pkt;
}

void udt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    if(rand()%2==0) {
        char* pkt;
        pkt = malloc(sizeof(data)+ 1);
        strcpy(pkt, data);
        pkt[5] = '1';
        sendto(socket_descriptor, pkt, sizeof(pkt), 0, (struct sockaddr*) &server_address, server_address_length);
    }
    else {
        sendto(socket_descriptor, data, sizeof(data), 0, (struct sockaddr*) &server_address, server_address_length);
    }
}

void rdt_rcv(char* rcvpkt) {
    int socket_descriptor_server = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_descriptor_server == -1) {
        perror("Unable to initialize socket");
        exit(1);
    }
    struct sockaddr_in server_address;
    server_address.sin_addr.s_addr=INADDR_ANY;
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(8000);

    socklen_t address_length = sizeof(server_address);

    int bindCode = bind(socket_descriptor_server, (struct sockaddr*) &server_address, address_length);
    if(bindCode==-1) {
        perror("Error on bind");
        exit(1);
    }
    
    ssize_t rcvpkt_length = sizeof(rcvpkt);
    recv(socket_descriptor_server, rcvpkt, sizeof(rcvpkt), 0);
    printf("%s\n", rcvpkt);
    close(socket_descriptor_server);
}

int corrupt(char* rcvpkt) {
    if (rcvpkt[0]!='N' && rcvpkt[0]!='A') {
        return 1;
    }
    else {
        return 0;
    }
}

int isNACK(char* rcvpkt) {
    if(rcvpkt[0]=='N') {
        return 1;
    }
    else return 0;
}

int isACK(char* rcvpkt) {
    if(rcvpkt[0]=='A') {
        return 1;
    }
    else return 0;
}

void rdt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    char* checksum = "0";
    char* flag="0";
    char rcvpkt[1024]="ACK";
    int retransmit = 0;
    while(1) {
        char* sndpkt;
        if(isACK(rcvpkt) && corrupt(rcvpkt)==0) {
            retransmit = 0;
        }
        if(isNACK(rcvpkt) || corrupt(rcvpkt)==1) {
            if(corrupt(rcvpkt)==1) printf("Corrupt packet received\nRetransmitting last packet...\n");
            else printf("NACK received\nRetransmitting last packet...\n");
            sleep(2);
            udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            rdt_rcv(rcvpkt);
            retransmit = 1;
        }
        if(retransmit==0) {
            sndpkt = make_pkt(flag, data, checksum);
            udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            rdt_rcv(rcvpkt);
            if(flag=="1") {
                flag="0";
            }
            else {
                flag="1";
            }
        }
        sleep(2);
    }
}

int main() {
    printf("RDT 2.1 Client\n");

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
