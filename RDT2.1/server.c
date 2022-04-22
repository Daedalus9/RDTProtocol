#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int corrupt(char* data) {
    if(data[5] - '0' ==1) return 1;
    else return 0;
}

int has_seq0(char* rcvpkt) {
    if(rcvpkt[0]=='0') return 1;
    else return 0;
}

int has_seq1(char* rcvpkt) {
    if(rcvpkt[0]=='1') return 1;
    else return 0;
}

int set_socket() {
    int socket_descriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor == -1) {
        perror("Unable to initialize socket");
        exit(1);
    }
    return socket_descriptor;
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

void udt_send(char* data) {
    int socket_descriptor_client = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in client_address = set_address(8000, 1);

    socklen_t client_address_length = sizeof(client_address);
   
    if(rand()%10==5) { // simulate corrupt response
        char* cr="corrupt";
        char* pkt = malloc(strlen(cr)+ 1);
        strcpy(pkt, cr);
        sendto(socket_descriptor_client, pkt, sizeof(pkt), 0, (struct sockaddr*) &client_address, client_address_length);
    }
    else sendto(socket_descriptor_client, data, sizeof(data), 0, (struct sockaddr*) &client_address, client_address_length);
    close(socket_descriptor_client);
}

void extract_packet(int socket_descriptor, char* data) {
    recv(socket_descriptor, data, sizeof(data), 0);
}

void deliver_data(char* data) {
    printf("%s\n", data);
}

void rdt_rcv(char* data, int socket_descriptor, struct sockaddr_in client_address, socklen_t client_address_length) {
    char exp_flag = '0';
    while(1) {
        recv(socket_descriptor, data, sizeof(data), 0);
        if(corrupt(data)==0 && has_seq0(data) && exp_flag=='0') {
            deliver_data(data);
            sleep(2);
            udt_send("ACK");
            exp_flag = '1';
            printf("Now expected flag is %d\n", exp_flag - '0');
        }
        else if(corrupt(data)==1) {
            printf("Corrupt packet received, not delivered: %s\n", data);
            sleep(2);
            udt_send("NACK");
        }
        else if(corrupt(data)==0 && has_seq0(data) && exp_flag=='1') {
            printf("Duplicate packet received. Not delivered: %s\n", data);
            printf("Received flag %d, expected flag %d. Probably caused by corrupt ACK send\n", data[0] - '0', exp_flag - '0');
            sleep(2);
            udt_send("ACK");
        }
        else if(corrupt(data)==0 && has_seq1(data) && exp_flag=='1') {
            deliver_data(data);
            udt_send("ACK");
            exp_flag = '0';
            printf("Now expected flag is %d\n", exp_flag - '0');
        }
        else if(corrupt(data)==0 && has_seq1(data) && exp_flag=='0') {
            printf("Duplicate packet received. Not delivered: %s\n", data);
            printf("Received flag %d, expected flag %d. Probably caused by corrupt ACK send\n", data[0] - '0', exp_flag - '0');
            sleep(2);
            udt_send("ACK");
        }
        else {
            printf("Corrupt packet received, not delivered: %s\n", data);
            udt_send("NACK");
        }
    }
}

void check_bind(int socket_descriptor, struct sockaddr_in address, socklen_t address_length) {
    int bindCode = bind(socket_descriptor, (struct sockaddr*) &address, address_length);
    if(bindCode==-1) {
        perror("Error on bind");
        exit(1);
    }
}

int main() {
    printf("RDT 2.1 Server\n");
    int socket_descriptor = set_socket();
    struct sockaddr_in address = set_address(9000, 0);

    socklen_t address_length = sizeof(address);
    check_bind(socket_descriptor, address, address_length);

    char msg_buffer[1024];
    ssize_t msg_length = sizeof(msg_buffer);
    rdt_rcv(msg_buffer, socket_descriptor, address, address_length);
}