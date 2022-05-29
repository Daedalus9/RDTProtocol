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
    if (socket_descriptor == -1) {
        perror("Unable to initialize socket");
        exit(1);
    }
    return socket_descriptor;
}

void check_bind(int socket_descriptor, struct sockaddr_in address, socklen_t address_length) {
    int bindCode = bind(socket_descriptor, (struct sockaddr*) &address, address_length);
    if(bindCode==-1) {
        perror("Error on bind");
        exit(1);
    }
}

struct sockaddr_in set_address(int port, int isSend) {
    struct sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_port=htons(port);
    if(isSend==1) {
        int addr_conversion_ret_code = inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
        if(addr_conversion_ret_code<1) {
            perror("Error on address conversion");
            exit(1);
        }
    }
    else address.sin_addr.s_addr=INADDR_ANY;
    return address;
}

int isACK(char* rcvpkt, char* flag) {
    if(rcvpkt[0]=='A' && flag=="1") return 1;
    else return 0;
}

int corrupt(char* rcvpkt) {
    if (rcvpkt[0]!='A') return 1;
    else return 0;
}

char* make_pkt(char *flag, char* data, char* checksum) {
    char* pkt;
    pkt = malloc(sizeof(data) + sizeof(checksum) + 2);
    strcpy(pkt, flag);
    strcat(pkt, data);
    strcat(pkt, checksum);
    return pkt;
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

void udt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    if(rand()%2==0) {
        char* pkt;
        pkt = malloc(sizeof(data)+ 1);
        strcpy(pkt, data);
        pkt[5] = '1';
        sendto(socket_descriptor, pkt, sizeof(pkt), 0, (struct sockaddr*) &server_address, server_address_length);
    }
    else sendto(socket_descriptor, data, sizeof(data), 0, (struct sockaddr*) &server_address, server_address_length);
}

void rdt_send(char* data, int socket_descriptor, struct sockaddr_in server_address, socklen_t server_address_length) {
    char* checksum = "0";
    char* flag = "0";
    char* flag_ack="0";
    char rcvpkt[1024];
    char* sndpkt;
    int retransmit = 0;
    sndpkt = make_pkt(flag_ack, data, checksum);
    udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
    sleep(2);
    char exp_flag='0';
    while(1) {
        rdt_rcv(rcvpkt);
        if(rcvpkt[3]=='0') flag_ack = "0";
        else flag_ack = "1";
        if(corrupt(rcvpkt)==1) {
            printf("Corrupt packet received. Resend last packet\n");
            udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            
        }
        else {
            if(isACK(rcvpkt, flag_ack)==0 && exp_flag=='0') {
                flag="1";
                exp_flag='1';
                sndpkt = make_pkt(flag, data, checksum);
                udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            }
            else if(isACK(rcvpkt, flag_ack)==1 && exp_flag=='1') {
                flag="0";
                exp_flag='0';
                sndpkt = make_pkt(flag, data, checksum);
                udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            }
            else if(isACK(rcvpkt, flag_ack)==0 && exp_flag=='1') {
                printf("ACK is number %d, but expected flag is %d\n", isACK(rcvpkt, flag_ack), exp_flag - '0');
                udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            }
            else if(isACK(rcvpkt, flag_ack)==1 && exp_flag=='0') {
                printf("ACK is number %d, but expected flag is %d\n", isACK(rcvpkt, flag_ack), exp_flag - '0');
                udt_send(sndpkt, socket_descriptor, server_address, server_address_length);
            }
        }
        sleep(2);
    }
}

int main() {
    printf("RDT 2.2 Sender\n");

    int socket_descriptor = set_socket();
    struct sockaddr_in server_address = set_address(9000, 0);
    socklen_t server_address_length = sizeof(server_address);
    
    char* data = "DATA";
    rdt_send(data, socket_descriptor, server_address, server_address_length);
}