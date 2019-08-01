#define CHUNK 32768
#define MAX_DEVS 255 // max devices returned during discovery
#define DISC_UNIT 8  // unit * 1.28sec time spent searching for devices
#define ADDR_SIZE 18 // size in chars of a bluetooth address

typedef struct _bDevInf {
	int devId;
    char addr[ADDR_SIZE];
    char* name;
} bDevInf;

int findLocalDevices(bDevInf ** const devs, int * const numDevs);

int findDevices(bDevInf ** const devs, int * const numDevs);

int client(const char* const dest, const char* const data, int size);

int server(char addr[ADDR_SIZE], char ** const data, int* const size);
