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
#define RED     "\033[31m"

/*
A opens file 1
A,B open file 3
C open file 2
A reads file 1 to the full buf cache size
A, B, C attach exts
C reads 1 block of file 2, which causes eviction
we should see exts by A and C run, but not B. A runs because its blocks are being
evicted. C runs because it's the current process that causes the eviction. B's
is not run because none of its data is in the buf cache
*/
            
#define FULL_SIZE     (512 * 30)       
#define ONE_BLOCK      512                

char bigbuf[FULL_SIZE];
char oneblockbuf[ONE_BLOCK];

void
print_separator(void)
{
  printf(1, DIM "─────────────────────────────────────────\n" RESET);
}

static void
make_file(char *name, char *data, int len)
{
  int fd = open(name, O_CREATE | O_WRONLY);
  if(fd < 0){
    printf(1, RED "error: failed to create %s\n" RESET, name);
    exit();
  }
  if(write(fd, data, len) != len){
    printf(1, RED "error: failed to write %s\n" RESET, name);
    exit();
  }
  close(fd);
}

void
test_flush_attribution(void)
{
  /* -------------------- root creates files -------------------------------- */
  int root = getnsid();

  memset(bigbuf, 0, sizeof(bigbuf));
  make_file("file1", bigbuf, FULL_SIZE);
  printf(1, MAGENTA "[nsid %d] " RESET GREEN "created file1 " RESET DIM "(%d bytes)\n" RESET, root, FULL_SIZE);

  memset(oneblockbuf, 0, sizeof(oneblockbuf));
  make_file("file2", oneblockbuf, ONE_BLOCK);
  printf(1, MAGENTA "[nsid %d] " RESET GREEN "created file2 " RESET DIM "(%d bytes)\n" RESET, root, ONE_BLOCK);

  make_file("file3", oneblockbuf, ONE_BLOCK);
  printf(1, MAGENTA "[nsid %d] " RESET GREEN "created file3 " RESET DIM "(%d bytes)\n" RESET, root, ONE_BLOCK);

/* ----------------- create namespaces  -------------------------------- */
  int nsA = mkns();
  int nsB = mkns();
  int nsC = mkns();
  printf(1, BOLD "Created namespaces " RESET CYAN "A=%d " RESET YELLOW "B=%d " RESET GREEN "C=%d\n" RESET, nsA, nsB, nsC);

/* ---------- namespaces open files to establish interest ------------------- */
  printf(1, BOLD MAGENTA "\n###### establishing interest\n\n" RESET);

  // A opens file1
  if(chns(nsA) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  int fdA1 = open("file1", O_RDONLY);
  if(fdA1 < 0){ printf(1, RED "error: open file1 failed\n" RESET); return; }
  printf(1, CYAN "[nsid %d] " RESET "opened file1\n" RESET, getnsid());

  // A opens file3
  int fdA3 = open("file3", O_RDONLY);
  if(fdA3 < 0){ printf(1, RED "error: open file3 failed\n" RESET); return; }
  printf(1, CYAN "[nsid %d] " RESET "opened file3\n" RESET, getnsid());

  // B opens file3
  if(chns(nsB) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  int fdB3 = open("file3", O_RDONLY);
  if(fdB3 < 0){ printf(1, RED "error: open file3 failed\n" RESET); return; }
  printf(1, YELLOW "[nsid %d] " RESET "opened file3\n" RESET, getnsid());

  // C opens file2
  if(chns(nsC) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  int fdC2 = open("file2", O_RDONLY);
  if(fdC2 < 0){ printf(1, RED "error: open file2 failed\n" RESET); return; }
  printf(1, GREEN "[nsid %d] " RESET "opened file2\n" RESET, getnsid());

  /* ---------------- clear the whole cache ---------------- */
  // this gets rid of residual blocks left from the opens

  // if(chns(root) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  // int fdF = open("file1", O_RDONLY);
  // if(fdF < 0){ printf(1, RED "error: open filler failed\n" RESET); return; }
  // read(fdF, bigbuf, FULL_SIZE);
  // close(fdF);
  // printf(1, MAGENTA "[nsid %d] " RESET "read filler file " DIM "(clears stray cache entries from the opens above)\n" RESET, getnsid());


  // ---------------- A fills the cache with a big file ----------------

  printf(1, BOLD MAGENTA "\n###### nsid 1 fills buffer cache \n\n" RESET);
  if(chns(nsA) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  printf(1, CYAN "[nsid %d] " RESET "reading file1 " DIM "(fills the entire cache)\n" RESET, getnsid());
  int rA = read(fdA1, bigbuf, FULL_SIZE);
  printf(1, CYAN "[nsid %d] " RESET GREEN "read %d bytes\n" RESET, getnsid(), rA);

  /* ---------------- load + attach flush ext in A, B, C ---------------- */
  printf(1, BOLD MAGENTA "\n###### namespaces A, B, C attach extensions\n\n" RESET);

  struct extension *eA, *eB, *eC;

  if(chns(nsA) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  if(extload("flush.ext", &eA) != 0){ printf(1, RED "error: extload failed\n" RESET); exit(); }
  extattach(eA);
  printf(1, CYAN "[nsid %d] " RESET GREEN "flush.ext attached\n" RESET, getnsid());

  if(chns(nsB) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  if(extload("flush.ext", &eB) != 0){ printf(1, RED "error: extload failed\n" RESET); exit(); }
  extattach(eB);
  printf(1, YELLOW "[nsid %d] " RESET GREEN "flush.ext attached\n" RESET, getnsid());

  if(chns(nsC) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  if(extload("flush.ext", &eC) != 0){ printf(1, RED "error: extload failed\n" RESET); exit(); }
  extattach(eC);
  printf(1, GREEN "[nsid %d] " RESET GREEN "flush.ext attached\n" RESET, getnsid());

  /* ---------------- C reads 1 block to cause cache eviction ---------------- */
  printf(1, BOLD MAGENTA "\n###### nsid 3 reads one block, causing an eviction\n" RESET);
  printf(1, DIM "expect nsid %d and nsid %d to fire, nsid %d to stay silent\n\n" RESET, nsA, nsC, nsB);
  if(chns(nsC) < 0){ printf(1, RED "error: chns failed\n" RESET); return; }
  printf(1, GREEN "[nsid %d] " RESET "reading 1 block of file2\n" RESET, getnsid());
  int rC = read(fdC2, oneblockbuf, ONE_BLOCK);
  printf(1, GREEN "[nsid %d] " RESET GREEN "read %d bytes\n" RESET, getnsid(), rC);


  // cleanup
  close(fdA1);
  close(fdA3);
  close(fdB3);
  close(fdC2);

  print_separator();
  printf(1, BOLD "  Demo complete\n" RESET);
  print_separator();
}

int
main(void)
{
  print_separator();
  printf(1, BOLD MAGENTA "\n  Buffer Cache Attribution Demo: Disk cache eviction hook\n\n" RESET);
  print_separator();

  test_flush_attribution();
  exit();
}