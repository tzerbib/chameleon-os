#include "types.h"
#include "user.h"
#include "fcntl.h"

int main(void) {

    chns(1); // corresponds to the chns in x_hello
    char buf[512];
    int i, fd;
    char path[16];

    // create many files to exhaust the buffer cache
    for(i = 0; i < 10; i++){
        path[0] = 'f'; path[1] = '0' + i; path[2] = '\0';
        fd = open(path, O_CREATE | O_RDWR);
        if(fd < 0){ 
            printf(1, "open failed at %d\n", i); exit(); 
        }
        write(fd, buf, 512);
        close(fd);
        printf(1, "wrote file %d\n", i);
    }
    exit();
}