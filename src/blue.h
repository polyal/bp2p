#define CHUNK 32768
#define MAX_DEVS 255 // max devices returned during discovery
#define DISC_UNIT 8  // unit * 1.28sec time spent searching for devices
#define ADDR_SIZE 18 // size in chars of a bluetooth address

typedef struct _devInf {
	int devId;
    char addr[ADDR_SIZE];
    char* name;
} devInf;

int findLocalDevices(devInf ** const devs, int * const numDevs);

int findDevices(devInf ** const devs, int * const numDevs);

int client(const char* const dest, const char* const data, int size);

int server(char addr[ADDR_SIZE], char ** const data, int* const size);


int createServer(int * const err);

int createClient(const char* const dest, int * const err);

int sendRequest(const int sock, const char * const reqData, const int size, char recData[255]);
