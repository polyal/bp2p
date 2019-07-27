#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <errno.h>

#define CHUNK 32768

void findDevices(){
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };

    // get first available local bluetooth adapter
    dev_id = hci_get_route(NULL);

    // opens a socket to the bluetooth microcontroller with id dev_id
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    }

    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    
    // perform bluetooth discovery
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
            strcpy(name, "[unknown]");
        printf("%s  %s\n", addr, name);
    }

    free( ii );
    close( sock );

}

int client(const char *dest, const char *data, int size){
    struct sockaddr_rc addr = { 0 };
    int s, status;

    if (dest == NULL || strlen(dest) != 17){
        printf("Client Error: Invalid Dest Addr.\n");
        return -1;
    }
    else if (data == NULL || size <= 0 || size > CHUNK){
        printf("Client Error: Invalid data.\n");
        return -1;
    }


    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (s == -1){
        printf("Client Error: Cannot allocate socket. %d \n", errno);
        return errno;
    }

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (status == 0){
        status = write(s, data, size);

        if( status == -1 )
            printf("Client Error: Write error. %d \n", errno);
        
    }
    else{
        printf("Client Error: Cannot connect to socket. %d \n", errno);
    }

    status = close(s);

    if (status == -1){
        printf("Client Error: Close error. %d \n", errno);
        status = errno;
    }

    return status;
}

int server(char addr[18], char **data, int *size){
    int status = 0;
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buff[CHUNK] = { 0 };
    char cAddr[18] = { 0 };
    int s, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);

    if (data == NULL || size == NULL){
         printf("Server Error: Data returned cannot be NULL \n");
         return -1;
    }

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (s == -1){
        printf("Server Error: Cannot allocate socket. %d \n", errno);
        return errno;
    }

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 1;
    status = bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    if (status == -1){
        printf("Server Error: Cannot bind name to socket. %d \n", errno);
    }

    // put socket into listening modeq
    status = listen(s, 1);
    if (status == -1){
        printf("Server Error: Cannot listen for connections on socket. %d \n", errno);
    }

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);
    if (client == -1){
        printf("Server Error: Failed to accept message. %d \n", errno);
    }
    else{
        ba2str( &rem_addr.rc_bdaddr, cAddr );
        printf("Server Notice: accepted connection from %s \n", cAddr);
        memcpy(addr, cAddr, 17);
    }

    // read data from the client
    bytes_read = read(client, buff, sizeof(buff));
    if( bytes_read == -1 ) {
        printf("Server Error: Failed to read message. %d \n", errno);
    }
    else{
        printf("Server Notice: Read message:\n%s\n", buff);
        *data = malloc(sizeof(char)*bytes_read);

        if (*data == NULL)
            printf("Server Error: Failed to return data\n");
        else{
            memcpy(*data, buff, bytes_read);
        }
    }

    // close connection
    close(client);
    close(s);

    return status;
}

int main(int argc, char **argv)
{
    char addr[18];
    char* data = NULL;
    int size = 0;

    if (argc < 2){
        printf("Usage: ./a.out [-c | -s | -f]\n");
        return 0;
    }

    if (strcmp ("-c", argv[1]) == 0)
        client("34:DE:1A:1D:F4:0B", "Send This Data", sizeof("Send This Data\0"));
    else if (strcmp ("-s", argv[1]) == 0)
        server(addr, &data, &size);
    else if (strcmp ("-f", argv[1]) == 0)
        findDevices();

    if (data) free(data);

    return 0;
}