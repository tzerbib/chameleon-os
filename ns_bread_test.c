#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

void test_bread_attribution(void) {
    char buf1[512];
    char buf2[512];
    char buf3[512];

    printf(1, "\nSTARTING TEST_BREAD_ATTRIBUTION\n");

    // nsid 1 opens and reads file
    printf(1, "in ns %d\n", getnsid());

    // attach bread extension 
    struct extension *e0;
    if(extload("bread.ext", &e0) != 0){
        printf(1, "extload error\n");
        exit();
    }
    extattach(e0);
    printf(1, "bread.ext attached to ns %d\n", getnsid());

    // read a file (ext should trigger)
    int fd1 = open("README", O_RDONLY);
    if(fd1 < 0){
        printf(1, "error\n");
        return;
    }
    int bytes_read0 = read(fd1, buf1, sizeof(buf1));
    printf(1, "ns0: read %d bytes\n", bytes_read0);

    // fd1 is still open, so ns1's ext should trigger on blocks related to fd1

    // switch to nsid 2
    int nsid2 = mkns();
    if(nsid2 < 0){
        printf(1, "mkns error\n");
        close(fd1);
        return;
    }

    if(chns(nsid2) < 0){
        printf(1, "chns error\n");
        close(fd1);
        return;
    }
    printf(1, "switched to nsid %d\n", getnsid());

    // attach bread extension to nsid 2
    struct extension *e1;
    if(extload("bread.ext", &e1) != 0){
        printf(1, "extload failed for ns1\n");
        close(fd1);
        exit();
    }
    extattach(e1);
    printf(1, "bread.ext attached to ns %d\n", getnsid());

    // nsid 2 opens and reads the same file 
    // expected behavior:
    // nsid 1 triggering ext fires (NS1 is the process doing the read)
    // nsid 2 data attribution ext fires (NS0 has file open, inode backpointer set)
    int fd2 = open("README", O_RDONLY);
    if(fd2 < 0){
        printf(1, "nsid %d: readme open failed\n", getnsid());
        close(fd2);
        return;
    }
    int bytes_read1 = read(fd2, buf1, sizeof(buf1));
    printf(1, "nsid %d: read %d bytes\n", getnsid(), bytes_read1);
    close(fd2);

    // // --- switch back to NS0 and close fd ---
    // // after this, ns_opencounts[0] should go to 0
    // // NS0's data attribution ext should no longer fire
    // if(chns(0) < 0){
    //     printf(1, "chns back to ns0 failed\n");
    //     return;
    // }
    // printf(1, "switched back to nsid %d\n", getnsid());
    // close(fd1);
    // printf(1, "ns0: closed fd, ns_opencounts[0] should now be 0\n");

    // // --- NS0 reads again after closing ---
    // // NS0's data attribution ext should NOT fire now
    // // since ns_opencounts[0] == 0
    // if(chns(nsid2) < 0){
    //     printf(1, "chns failed\n");
    //     return;
    // }
    // printf(1, "ns%d: reading after ns0 closed file\n", getnsid());
    // int fd2 = open("README", O_RDONLY);
    // if(fd2 < 0){
    //     printf(1, "ns%d: failed to open README\n", getnsid());
    //     return;
    // }
    // int bytes_read2 = read(fd2, buf2, sizeof(buf2));
    // printf(1, "ns%d: read %d bytes\n", getnsid(), bytes_read2);
    // close(fd2);

    printf(1, "TEST_BREAD_ATTRIBUTION DONE\n");
}

int
main(void)
{
    printf(1, "hello from bread attribution test!\n");

    int nsid = getnsid();
    printf(1, "starting namespace: %d\n", nsid);

    test_bread_attribution();

    exit();
}