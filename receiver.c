// udp_receiver.c
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "my_protocol.h"

#define BUFFER_SIZE 1024

int getPackets(){
    printf("getsPacets\n");
    char *file_name = "output2.png";
    // int total_file_size = 96400 ;
    int total_file_size = 30728;
    int packet_size = BUFFER_SIZE;
    int total_packets = total_file_size/(packet_size- (sizeof(int)*3));
    if (total_file_size % (packet_size- (sizeof(int)*3)) > 0 )
        total_packets++;
    int received_packets_flags[total_packets];
    struct PACKET packet_data;

    
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
    printf("Socket initialization\n");

    memset(received_packets_flags, 0, sizeof(received_packets_flags[0])*total_packets);

    printf("Socket initialization\n");


    //initialize and bind the socket
    struct sockaddr_in receiver_addr, sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    const int receiver_port = 12345;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(receiver_port);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    printf("Socket variables initialization\n");

    bind(sockfd, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));

    printf("Binding Successful\n");
    printf("bind done\n");
    fflush(stdout);

    printf("bind done\n");
    fflush(stdout);
    for (int i=0; i<total_packets; i++){

        printf("In packet get loop %d\n", i);
        fflush(stdout);

        memset(&packet_data, 0, sizeof(packet_data));

        // printf("sizeof(packet_data): %ld\n",sizeof(packet_data));

        // int received_bytes = receive_packet((void *)&packet_data, BUFFER_SIZE);

        ssize_t recv_len = recvfrom(sockfd, (void *) &packet_data, BUFFER_SIZE, 0,
                                    (struct sockaddr*)&sender_addr, &addr_len);

        printf("Data Received : %ld packet bytes\n", recv_len);

        printf("packet_type : %d\n", packet_data.packet_type);
        printf("packet_number : %d\n", packet_data.packet_number);
        printf("no_of_raw_data_bytes : %d\n", packet_data.no_of_raw_data_bytes );

        fflush(stdout);

        if (recv_len < 0) {
            perror("recvfrom");
        }

        // printf("Received: %d bytes\n", recv_len);
        // printf("Sender IP: %s\n", inet_ntoa(sender_addr.sin_addr));
        // printf("Sender Port: %d\n", ntohs(sender_addr.sin_port));

        // move lseek to appropriate position
        int offset = (BUFFER_SIZE- (sizeof(int)*3)) * packet_data.packet_number ;
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("lseek");
            break;
        }

        int bytes_to_write = packet_data.no_of_raw_data_bytes ;
        if (write(fd, packet_data.packet_raw_data_bytes, bytes_to_write) != bytes_to_write) {
            perror("write");
            break;
        }

        printf("written %d bytes in file\n\n", bytes_to_write);

        // updat packet flag
        received_packets_flags[packet_data.packet_number] = 1 ;
    }
    close(fd);

    for (int i=0; i<total_packets; i++){
        printf("Packet %d: %d\n", i, received_packets_flags[i]);
    }
    return 0;
}

int main(){
    getPackets();
}
