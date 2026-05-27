#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

/* 
Here's the example that I had in mind that aims at illustrating the different triggering mechanisms (process context vs data attribution) in the buffer cache:

Namespace 2 loads (but does not yet attach) an extension on flush (or on whichever hookpoint is triggered when a buffer cache entry gets persisted to disk), printing “ns 2 is flushing data” (this is done here to avoid flushing ns 1 entries, which would happens if done later)
Namespace 1 fills the buffer cache (opens and reads a large file)
Namespace 1 installs (load + attach) an extension on flush, printing “ns 1 data x got flushed” (x could be a block number or equivalent)
Namespace 2 now attaches its previously loaded extension
Print a visible message (e.g. “###### start flushing”)
Namespace 2 reads 1 block.

Assuming the buffer cache was roughly fully filled by namespace 1, the read from namespace 2 would flush the entry cached by 1, triggering both extensions that got installed on flush.

The expected output should look roughly like this:

###### start flushing
Ns 1 data x got flushed
Ns 2 is flushing data 
*/

char buf0[512]; // 30 512 blocks in buf cache
char buf1[30*512];

void ns_flush_test(void) {

    // nsid 0 opens and reads file
    int nsid0 = getnsid();
    printf(1, "in nsid %d\n", nsid0);

    // ns 0 loads flush ext (doesnt attach yet)
    struct extension *e0;
    if(extload("flush.ext", &e0) != 0){
        printf(1, "extload error\n");
        exit();
    }

    printf(1, "loaded flush ext into nsid 0\n");

    // switch to ns 1
    int nsid1 = mkns();
    if(nsid1 < 0){
        printf(1, "mkns error\n");
        return;
    }

    if(chns(nsid1) < 0){
        printf(1, "chns error\n");
        return;
    }
    printf(1, "switched to nsid %d\n", getnsid());

    // ns 1 fills buffer cache by reading large file 
    int fd1 = open("etos-logo.png", O_RDONLY);
    if(fd1 < 0){
        printf(1, "error\n");
        return;
    }
    int bytes_read1 = read(fd1, buf1, sizeof(buf1));
    printf(1, "nsid 1: read %d bytes\n", bytes_read1);

    // ns 1 installs ext on flush
    struct extension *e1;
    if(extload("flush.ext", &e1) != 0){
        printf(1, "extload error\n");
        exit();
    }
    extattach(e1);
    printf(1, "flush.ext attached to ns %d\n", getnsid());

    // switch back to ns 0 and attach ext
    if(chns(nsid0) < 0){
        printf(1, "chns error\n");
        return;
    }
    printf(1, "switched to nsid %d\n", getnsid());
    extattach(e0);
    printf(1, "flush.ext attached to ns %d\n", getnsid());

    // ns 0 reads 1 block 
    int fd0 = open("etos-logo.png", O_RDONLY);
    if(fd0 < 0){
        printf(1, "error\n");
        return;
    }
    int bytes_read0 = read(fd0, buf0, sizeof(buf0));
    printf(1, "nsid 0: read %d bytes\n", bytes_read0);

    printf(1, "test done\n");
}

int
main(void)
{
    printf(1, "hello from ns_flush_test!\n");

    int nsid = getnsid();
    printf(1, "starting namespace: %d\n", nsid);

    ns_flush_test();

    exit();
}