#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

// Caller is responsible for providing suitable memory through page
// Assumes page != NULL
// Fills page and writes entry point within it to *entry
// Returns 0 on success, -1 on failure
int kload_elf(char* path, char* page, void** entry) {
  int i;
  int off;
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  
  begin_op();
  
  if((ip = namei(path)) == 0){
    end_op();
    cprintf("kload_elf: fail\n");
    return -1;
  }
  cprintf("locking inode on path %s\n", path);
  ilock(ip);
  
  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf)) {
    goto bad;
  }
  if(elf.magic != ELF_MAGIC) {
    goto bad;
  }
  
  cprintf("checked elf header\n");

  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)) {
    cprintf("in loop\n");
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph)) {
      goto bad;
    }
    cprintf("ph.type %d\n", ph.type);
    // Check if the segment is LOAD type
    if(ph.type != ELF_PROG_LOAD) {
      continue;
    }

    // Check if the segment is readable and executable
    uint read_exec = (ELF_PROG_FLAG_READ | ELF_PROG_FLAG_EXEC);
    if ((ph.flags & read_exec) != read_exec) {
      continue;
    }

    // TODO: get rid of this along with the above
    // Ensure that entry point is in this segment
    if (elf.entry < ph.vaddr || elf.entry - ph.vaddr >= ph.memsz) {
      cprintf("entry point %d out of RX section: [%d, %d)\n", elf.entry, ph.vaddr, ph.vaddr + ph.memsz);
      goto bad;
    }

    cprintf("got here\n");
    if(ph.memsz < ph.filesz) {
      goto bad;
    }
    if(ph.vaddr + ph.memsz < ph.vaddr) {
      goto bad;
    }

    // TODO: probably needs to be a loop eventually
    // For now, assume the elf would fit in a page
    if (readi(ip, page, ph.off, ph.filesz) != (int)ph.filesz) {
      goto bad;
    }

    *entry = page + (elf.entry - ph.vaddr);
  }
  iunlockput(ip);
  end_op();
  ip = 0;
  cprintf("returning from kload_elf\n");
  return 0;

 bad:
  if(ip) {
    iunlockput(ip);
    end_op();
  }
  return -1;
}
