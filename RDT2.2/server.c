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

void udt_send(char* data, char* flag) {
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
    if(rand()%10==5) { // simulate corrupt response
        char* cr="corrupt";
        char* pkt = malloc(strlen(cr)+ 1);
        strcpy(pkt, cr);
        sendto(socket_descriptor_client, pkt, sizeof(pkt), 0, (struct sockaddr*) &client_address, client_address_length);
    }
    else {
        char* pkt = malloc(strlen(data)+2);
        strcpy(pkt, data);
        strcat(pkt, flag);
        sendto(socket_descriptor_client, pkt, sizeof(pkt), 0, (struct sockaddr*) &client_address, client_address_length);
    }
    close(socket_descriptor_client);
}

void deliver_data(char* data) {
    printf("%s\n", data);
}

void rdt_rcv(char* data, int socket_descriptor, struct sockaddr_in client_address, socklen_t client_address_length) {
    char exp_flag='0';
    while(1) {
        recv(socket_descriptor, data, sizeof(data), 0);
        if(corrupt(data)==0) {
            if(has_seq0(data) && exp_flag=='0') {
                deliver_data(data);
                sleep(2);
                udt_send("ACK", "0");
                exp_flag = '1';
                printf("Now expected flag is %d\n", exp_flag - '0');
            }
            else if(has_seq0(data) && exp_flag=='1') {
                printf("Duplicate packet received, not delivery: %s\n", data);
                sleep(2);
                udt_send("ACK", "0");
            }
            else if(has_seq1(data) && exp_flag=='0') {
                printf("Duplicate packet received, not delivery: %s\n", data);
                sleep(2);
                udt_send("ACK", "1");
            }
            else if(has_seq1(data) && exp_flag=='1') {
                deliver_data(data);
                sleep(2);
                udt_send("ACK", "1");
                exp_flag = '0';
                printf("Now expected flag is %d\n", exp_flag - '0');
            }
        }       
        else {
            printf("Corrupt packet received, not delivered: %s\n", data);
            sleep(2);
            if(exp_flag=='0') udt_send("ACK", "1");
            else udt_send("ACK", "0");
        }
        sleep(2);
    }
}

int main() {
    printf("RDT 2.2 Server\n");

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