#ifndef MY_PROTOCOL_H
#define MY_PROTOCOL_H

#define METADATA_PACKET 0
#define RAWDATA_PACKET 1

#define BUFFER_SIZE 1024

struct MetaData{
    int packet_type;
    int total_file_size;
    int total_packets;
    int packet_size;
};

struct PACKET{
    int packet_type;
    int packet_number;
    int no_of_raw_data_bytes;
    char packet_raw_data_bytes[BUFFER_SIZE - sizeof(int)*3];
};
#endif
