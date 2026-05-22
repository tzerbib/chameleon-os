#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

void test_bread_attribution(void) {
    char buf1[512];
    char buf2[512];
    char buf3[512];

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
    // expected behavior: nsid1 and nsid2 extensions fire
    int fd2 = open("README", O_RDONLY);
    if(fd2 < 0){
        printf(1, "nsid %d: readme open failed\n", getnsid());
        close(fd2);
        return;
    }
    int bytes_read1 = read(fd2, buf1, sizeof(buf1));
    printf(1, "nsid %d: read %d bytes\n", getnsid(), bytes_read1);
    close(fd2);

    printf(1, "test_bread_attribution done\n");
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