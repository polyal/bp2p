#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <errno.h>

#define DEBUG 1
#define CHUNK 32768
#define MAX_DEVS 255 // max devices returned during discovery
#define DISC_UNIT 8  // unit * 1.28sec time spent searching for devices
#define ADDR_SIZE 18 // size in chars of a bluetooth address

typedef struct _bDevInf {
    char addr[ADDR_SIZE];
    char* name;
} bDevInf;

int findDevices(bDevInf ** const devs, int * const numDevs){
    int status = -1;
    inquiry_info *ii = NULL;
    int num_rsp;
    int dev_id, sock;
    char addr[ADDR_SIZE] = { 0 };
    char name[248] = { 0 };

    *numDevs = 0;

    // get first available local bluetooth adapter
    dev_id = hci_get_route(NULL);

    // opens a socket to the bluetooth microcontroller with id dev_id
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        printf("Discovery Error: Failed to open socket.\n");
        goto findDevicesCleanup;
    }

    ii = (inquiry_info*)malloc(sizeof(inquiry_info)* MAX_DEVS);
    if (ii == NULL) goto findDevicesCleanup;
    
    // perform bluetooth discovery, clear previously discovered devices from cache
    num_rsp = hci_inquiry(dev_id, DISC_UNIT, MAX_DEVS, NULL, &ii, IREQ_CACHE_FLUSH);
    if( num_rsp < 0 ) {
        printf("Discovery Notice: 0 Devices found.\n");
        goto findDevicesCleanup;
    }

    *devs = malloc(sizeof(bDevInf)* num_rsp);
    if (*devs == NULL) goto findDevicesCleanup;

    int i;
    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
            strcpy(name, "[unknown]");

        devs[i]->name = malloc(sizeof(char)* (strlen(name)+1));
        if (devs[i]->name == NULL) goto findDevicesCleanup;

        memcpy(devs[i]->name, name, strlen(name));
        memcpy(devs[i]->addr, addr, strlen(addr));
        printf("%s  %s\n", devs[i]->addr, devs[i]->name);
    }

    *numDevs = num_rsp;
    status = 0;

findDevicesCleanup:    
    if (ii) free( ii );
    if (sock >= 0) close( sock );

    return status;
}

int client(const char *dest, const char *data, int size){
    int status = -1;
    struct sockaddr_rc addr = { 0 };
    int sock;


    if (dest == NULL || strlen(dest) != 17){
        printf("Client Error: Invalid Dest Addr.\n");
        goto clientCleanup;
    }
    else if (data == NULL || size <= 0 || size > CHUNK){
        printf("Client Error: Invalid data.\n");
        goto clientCleanup;
    }


    // allocate a socket
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock == -1){
        printf("Client Error: Cannot allocate socket. %d \n", errno);
        status = errno;
        goto clientCleanup;
    }

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    status = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (status == 0){
        status = write(sock, data, size);

        if( status == -1 ){
            printf("Client Error: Write error. %d \n", errno);
            status = errno;
            goto clientCleanup;
        }
    }
    else{
        printf("Client Error: Cannot connect to socket. %d \n", errno);
        status = errno;
        goto clientCleanup;
    }

    status = 0;

clientCleanup:
    if (sock >= 0) close(sock);

    return status;
}

int server(char addr[ADDR_SIZE], char **const data, int *size){
    int status = -1;
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buff[CHUNK] = { 0 };
    char cAddr[ADDR_SIZE] = { 0 };
    int s, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);

    if (data == NULL || size == NULL){
        printf("Server Error: Data returned cannot be NULL \n");
        goto serverCleanup;
    }

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (s == -1){
        printf("Server Error: Cannot allocate socket. %d \n", errno);
        status = errno;
        goto serverCleanup;
    }

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 1;
    status = bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    if (status == -1){
        printf("Server Error: Cannot bind name to socket. %d \n", errno);
        status = errno;
        goto serverCleanup;
    }

    // put socket into listening modeq
    status = listen(s, 1);
    if (status == -1){
        printf("Server Error: Cannot listen for connections on socket. %d \n", errno);
        status = errno;
        goto serverCleanup;
    }

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);
    if (client == -1){
        printf("Server Error: Failed to accept message. %d \n", errno);
        status = errno;
        goto serverCleanup;
    }

    ba2str( &rem_addr.rc_bdaddr, cAddr );
    printf("Server Notice: accepted connection from %s \n", cAddr);
    memcpy(addr, cAddr, ADDR_SIZE-1);

    // read data from the client
    bytes_read = read(client, buff, sizeof(buff));
    if( bytes_read == -1 ) {
        printf("Server Error: Failed to read message. %d \n", errno);
        status = errno;
        goto serverCleanup;
    }

    printf("Server Notice: Read message:\n%s\n", buff);
    *data = malloc(sizeof(char)*bytes_read);
    if (*data == NULL){
        printf("Server Error: Failed to return data\n");
        goto serverCleanup;
    }

    memcpy(*data, buff, bytes_read);
    status = 0;

serverCleanup:
    close(client);
    close(s);

    return status;
}

#if DEBUG == 1
int main(int argc, char **argv)
{
    char addr[ADDR_SIZE] = {0};
    char* data = NULL;
    bDevInf *devs = NULL;
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
        findDevices(&devs, &size);

    int i;
    for (i = 0; i < size; i++){
        printf("%s  %s\n", devs[i].addr, devs[i].name);
    }
    
    if (data) free(data);
    if (devs) free(devs);

    return 0;
}
#endif