#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define DIM     "\033[2m"
#define MAGENTA "\033[35m"

void print_separator(void) {
    printf(1, DIM "─────────────────────────────────────────\n" RESET);
}

void test_bread_attribution(void) {
    char buf1[1024];

    // nsid 0 attaches extension
    printf(1, CYAN "\n[nsid %d] " RESET "Attaching bread extension...\n", getnsid());
    struct extension *e0;
    if(extload("bread.ext", &e0) != 0){
        printf(1, "error: extload failed\n");
        exit();
    }
    extattach(e0);
    printf(1, CYAN "[nsid %d] " RESET GREEN "bread.ext attached\n" RESET, getnsid());

    // nsid 0 reads extension
    printf(1, CYAN "\n[nsid %d] " RESET "Opening README...\n", getnsid());
    int fd1 = open("README", O_RDONLY);
    if(fd1 < 0){
        printf(1, "error: open failed\n");
        return;
    }
    printf(1, CYAN "[nsid %d] " RESET "Reading file " DIM "(extension should fire for nsid 0 only)\n" RESET, getnsid());
    int bytes_read0 = read(fd1, buf1, sizeof(buf1));
    printf(1, CYAN "[nsid %d] " RESET GREEN "read %d bytes\n" RESET, getnsid(), bytes_read0);
    printf(1, DIM "  (README fd left open such that nsid 0 retains interest in these blocks)\n" RESET);

    // switch to nsid 1
    print_separator();
    int nsid2 = mkns();
    if(nsid2 < 0){
        printf(1, "error: mkns failed\n");
        close(fd1);
        return;
    }
    if(chns(nsid2) < 0){
        printf(1, "error: chns failed\n");
        close(fd1);
        return;
    }
    printf(1, YELLOW "\n[nsid %d] " RESET "Switched to new namespace\n", getnsid());

    // nsid 2 attaches extension
    struct extension *e1;
    if(extload("bread.ext", &e1) != 0){
        printf(1, "error: extload failed\n");
        close(fd1);
        exit();
    }
    extattach(e1);
    printf(1, YELLOW "[nsid %d] " RESET GREEN "bread.ext attached\n" RESET, getnsid());

    // nsid 2 reads same file
    printf(1, YELLOW "\n[nsid %d] " RESET "Opening README...\n", getnsid());
    int fd2 = open("README", O_RDONLY);
    if(fd2 < 0){
        printf(1, "error: open failed\n");
        return;
    }
    printf(1, YELLOW "[nsid %d] " RESET "Reading file " DIM "(extensions should fire for nsid 0 and nsid %d)\n" RESET, getnsid(), getnsid());
    int bytes_read1 = read(fd2, buf1, sizeof(buf1));
    printf(1, YELLOW "[nsid %d] " RESET GREEN "read %d bytes\n" RESET, getnsid(), bytes_read1);
    close(fd2);

    print_separator();
    printf(1, BOLD "  Demo complete\n" RESET);
    print_separator();
}

int
main(void)
{
    print_separator();
    printf(1, BOLD MAGENTA "\n  ChameleonOS — Buffer Cache Attribution Demo\n\n" RESET);
    print_separator();

    test_bread_attribution();
    exit();
}