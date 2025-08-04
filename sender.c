#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "my_protocol.h"

#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

    const char *filename = "file.png";
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Get file size using fstat
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }
    off_t file_size = file_stat.st_size;
    printf("Read %ld bytes from '%s'\n", (long)file_size, filename);


    int raw_data_buffer_size = BUFFER_SIZE - sizeof(int)*3 ;
    unsigned char raw_data_buffer[raw_data_buffer_size];
    ssize_t bytes_read;
    struct PACKET packet_data = {0};
    int packet_number_record = 0;

    int port = 12345;
    const char *receiver_ipv4 = "127.0.0.1";
    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(port);
    receiver_addr.sin_addr.s_addr = inet_addr(receiver_ipv4);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);


    while ((bytes_read = read(fd, raw_data_buffer, raw_data_buffer_size)) > 0) {
        printf("Read %zd bytes\n", bytes_read);
        printf("sizeof(packet_data): %ld\n", sizeof(packet_data));
        memset(&packet_data, 0, sizeof(packet_data));

        packet_data.packet_type = RAWDATA_PACKET;
        packet_data.packet_number = packet_number_record;
        packet_data.no_of_raw_data_bytes = bytes_read;
        memcpy(packet_data.packet_raw_data_bytes, raw_data_buffer, bytes_read);

        int bytes_to_write = packet_data.no_of_raw_data_bytes;

        // if (write(fd2, packet_data.packet_raw_data_bytes, bytes_to_write) != bytes_to_write) {
        //     perror("write");
        //     break;
        // }
        int bytes_sent = 0;
        int packet_size = BUFFER_SIZE;

        bytes_sent = sendto(sockfd, (void *)&packet_data, packet_size, 0,
                (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
        if ( bytes_sent < 0) {
            perror("sendto");
            return 1;
        }
        printf("Sent buffer (%d bytes)\n", bytes_sent);

        

        printf("write call successfull\n\n");
        memset(&raw_data_buffer, 0, raw_data_buffer_size);
        packet_number_record++;
    }
    // close(fd2);

    if (bytes_read < 0) {
        perror("read");
    }

    close(fd);
    
    return 0;
}
