#ifndef MY_PROTOCOL_H
#define MY_PROTOCOL_H

#define INFO_PACKET 0
#define DATA_PACKET 1

#define PACKET_SIZE 1024
// #define MAX_PACKETS 1024*1024*253 
#define FIlE_INFO_METADATA_FILE_NAME_MAX_SIZE 256
// NTFS (Windows), FAT32, exFAT, ext4 (Linux), APFS (macOS), HFS+ (macOS)
// 255 is standard limit by major Operating systems but yet to think on it

struct FIle_INFO_METADATA{
    int total_file_size;
    int total_packets;
    int packet_size;
    char file_name[FIlE_INFO_METADATA_FILE_NAME_MAX_SIZE];
};

struct PACKET_METADTA{
    int packet_type;
    int packet_number;
    int no_of_raw_data_bytes;
};

#define RAW_DATA_BYTES_SIZE ( PACKET_SIZE - sizeof(struct PACKET_METADTA) )
struct PACKET{
    struct PACKET_METADTA metadata;
    char raw_data_bytes[RAW_DATA_BYTES_SIZE];
};
#endif
