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

int raw_data_packet_handler(int file_fd, struct PACKET packet_data){
    printf("Data packet: %d\n", packet_data.metadata.packet_number);

    if (file_fd < 0 ){
        printf("ERROR: file_fd is invalid\n");
        return 1;
    }

    // move lseek to appropriate position
    int offset = (PACKET_SIZE- sizeof(struct PACKET_METADTA)) * packet_data.metadata.packet_number ;
    if (lseek(file_fd, offset, SEEK_SET) == -1) {
        perror("lseek");
        return 1;
    }

    int bytes_to_write = packet_data.metadata.no_of_raw_data_bytes ;
    if (write(file_fd, packet_data.raw_data_bytes, bytes_to_write) != bytes_to_write) {
        perror("write");
        return 1;
    }

    printf("written %d bytes in file\n\n", bytes_to_write);
    return 0;

}

void info_packet_handler(int *file_fd, struct PACKET packet_data, int *total_packets, unsigned char *received_packets_flags){
    // process information packet
    printf("Info Packet received");
    struct FIle_INFO_METADATA file_info;
    memcpy(&file_info, packet_data.raw_data_bytes, sizeof(struct FIle_INFO_METADATA));
    printf("fiel_info.total_file_size: %d\n", file_info.total_file_size);
    printf("fiel_info.total_packets: %d\n", file_info.total_packets);
    printf("fiel_info.packet_size: %d\n", file_info.packet_size);
    printf("fiel_info.file_name: %s\n", file_info.file_name);

    // open the file of given file size and set fd and total packets to be recived
    *total_packets = file_info.total_packets;
    memset(received_packets_flags, 0, sizeof(received_packets_flags[0])*(file_info.total_packets));

    // strcat(file_info.file_name, file_info.file_name);
    printf("New file would be named as : %s\n",file_info.file_name);
    *file_fd = open_file(file_info.file_name, file_info.total_file_size);

}

int track_packets(unsigned char *received_packets_flags, int total_packets, int recived_packet_number, int *remaining_packets){
    if (received_packets_flags[recived_packet_number] == 0 ){
        received_packets_flags[recived_packet_number] = 1;
        (*remaining_packets)--;
    }
}


int start_receiver(){
    printf("start_receiver\n");

    int total_packets;
    const int MAX_PACKETS = 1024*253;
    unsigned char received_packets_flags[MAX_PACKETS];
    memset(received_packets_flags, 0, sizeof(received_packets_flags[0])*MAX_PACKETS);
    
    printf("Socket initialization\n");
    
    
    //initialize and bind the socket
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    
    // setting up socket should be first step;
    int sockfd = setup_socket_listen(12345);
    int remaining_packets = 0;
    struct PACKET packet_data;
    int file_fd = -1;

    while(1){

        memset(&packet_data, 0, sizeof(packet_data));

        ssize_t recv_len = recvfrom(sockfd, (void *) &packet_data, PACKET_SIZE, 0,
                                    (struct sockaddr*)&sender_addr, &addr_len);
                                    
        if (recv_len < 0) {
            perror("recvfrom");
        }
        
        // printf("Received: %d bytes\n", recv_len);
        // printf("Sender IP: %s\n", inet_ntoa(sender_addr.sin_addr));
        // printf("Sender Port: %d\n", ntohs(sender_addr.sin_port));
        // printf("Data Received : %ld packet bytes\n", recv_len);
        // printf("packet_type : %d\n", packet_data.metadata.packet_type);
        // printf("packet_number : %d\n", packet_data.metadata.packet_number);
        // printf("no_of_raw_data_bytes : %d\n", packet_data.metadata.no_of_raw_data_bytes );


        if (packet_data.metadata.packet_type == INFO_PACKET)
        {
            info_packet_handler(&file_fd, packet_data, &total_packets, received_packets_flags);
            remaining_packets = total_packets ;
            continue;
        }

        else if(packet_data.metadata.packet_type == DATA_PACKET)
        {
            if (received_packets_flags[packet_data.metadata.packet_number] != 0 ){
                continue;   // skip if the packet is already received
            }

            int error = raw_data_packet_handler(file_fd, packet_data);

            if(error != 0){
                printf("ERROR: in handling raw packet data\n");
                continue;
            }

            received_packets_flags[packet_data.metadata.packet_number] = 1;
            remaining_packets--;
        }
        else{
            printf("ERROR: Packet Type Cannot be identified\n");
        }

        if (remaining_packets <= 0){
            printf("Finished Receiving the file please check if received correctly\n");
            close(file_fd);
            file_fd = -1;
        }
    }
    return 0;
}

int main(){
    start_receiver();
}
