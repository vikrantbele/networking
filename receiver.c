// udp_receiver.c
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "my_protocol.h"


int setup_socket_listen(const int receiver_port){
    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(receiver_port);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    printf("Socket variables initialization\n");

    // bind(sockfd, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
    if (bind(sockfd, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0) {
        fprintf(stderr, "Error binding socket: %s\n", strerror(errno));
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("socket binding Successful on port %d...\n", receiver_port);
    fflush(stdout);
    return sockfd;
}

int open_file(const char *file_name, int total_file_size){
    int packet_size = PACKET_SIZE;
    int total_packets = total_file_size/(RAW_DATA_BYTES_SIZE);
    if (total_file_size % (RAW_DATA_BYTES_SIZE) > 0 ){
        total_packets++;
    }
    
    printf("variable initialized\n");
    // Open file for writing (create it if it doesn't exist)
    int fd = open(file_name, O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("file opened\n");

    // Move file pointer to (file_size - 1)
    if (lseek(fd, total_file_size - 1, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("got to the last of the file\n");

    // Write a single null byte to extend file size
    if (write(fd, "", 1) != 1) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("Written one byte\n");

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("lseek returned\n");
    return fd;
}

void raw_data_packet_handler(int file_fd, struct PACKET packet_data){

    // move lseek to appropriate position
    int offset = (PACKET_SIZE- sizeof(struct PACKET_METADTA)) * packet_data.metadata.packet_number ;
    if (lseek(file_fd, offset, SEEK_SET) == -1) {
        perror("lseek");
        return;
    }

    int bytes_to_write = packet_data.metadata.no_of_raw_data_bytes ;
    if (write(file_fd, packet_data.raw_data_bytes, bytes_to_write) != bytes_to_write) {
        perror("write");
        return;
    }

    printf("written %d bytes in file\n\n", bytes_to_write);

}

int getPackets(){
    printf("getsPacets\n");
    char *file_name = "output2.png";
    int total_file_size = 30728;

    int file_fd = open_file(file_name, total_file_size);

    int packet_size = PACKET_SIZE;
    int total_packets = total_file_size/(RAW_DATA_BYTES_SIZE);
    if (total_file_size % (RAW_DATA_BYTES_SIZE) > 0 )
        total_packets++;
    int received_packets_flags[total_packets];
    struct PACKET packet_data;


    memset(received_packets_flags, 0, sizeof(received_packets_flags[0])*total_packets);

    printf("Socket initialization\n");


    //initialize and bind the socket
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    // setting up socket should be first step;
    int sockfd = setup_socket_listen(12345);

    for (int i=0; i<total_packets; i++){

        printf("In packet get loop %d\n", i);
        fflush(stdout);

        memset(&packet_data, 0, sizeof(packet_data));

        ssize_t recv_len = recvfrom(sockfd, (void *) &packet_data, PACKET_SIZE, 0,
                                    (struct sockaddr*)&sender_addr, &addr_len);
                                    
        if (recv_len < 0) {
            perror("recvfrom");
        }
        
        // printf("Received: %d bytes\n", recv_len);
        // printf("Sender IP: %s\n", inet_ntoa(sender_addr.sin_addr));
        // printf("Sender Port: %d\n", ntohs(sender_addr.sin_port));

        printf("Data Received : %ld packet bytes\n", recv_len);
        printf("packet_type : %d\n", packet_data.metadata.packet_type);
        printf("packet_number : %d\n", packet_data.metadata.packet_number);
        printf("no_of_raw_data_bytes : %d\n", packet_data.metadata.no_of_raw_data_bytes );

        if (packet_data.metadata.packet_type == INFO_PACKET)
        {
            // process information packet
            continue;
        }
        else if(packet_data.metadata.packet_type == DATA_PACKET){
            printf("Data packet: %d\n", packet_data.metadata.packet_number);
            raw_data_packet_handler(file_fd, packet_data);

            // updat packet flag
            received_packets_flags[packet_data.metadata.packet_number] = 1 ;
        }
        else{
            printf("ERROR: Packet Type Cannot be identified\n");
        }
    }
    close(file_fd);

    for (int i=0; i<total_packets; i++){
        printf("Packet %d: %d\n", i, received_packets_flags[i]);
    }
    return 0;
}

int main(){
    getPackets();
}
