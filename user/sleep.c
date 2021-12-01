#include "./user.h"
#include "../kernel/types.h"
#include "../kernel/stat.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(2, "Error: please passing a correct paramter\n");
        exit(-1);
    }
 
    int sleeptime = atoi(argv[1]);
    sleep(sleeptime);
    exit(0);
}
