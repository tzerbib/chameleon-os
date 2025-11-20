#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

// Advances *mem by one page
// Returns the end address of that page
static inline char* next_page(char** mem) {
  return *mem += PGSIZE;
}

// Caller is responsible for providing suitable memory through page
// Assumes page != NULL
// Fills page and writes entry point within it to *entry
// Returns 0 on success, -1 on failure
int kload_elf(char* path, char** mem, void** entry) {
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

  char* prog_start = *mem;
  char const* prog_end = prog_start;

  cprintf("now have %d pages allocated\n", (prog_end - prog_start) / PGSIZE);

  uint rel_offset;
  uint rel_count = 0;
  
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)) {
    cprintf("in loop\n");
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph)) {
      goto bad_after_alloc;
    }
    cprintf("ph.type %d\n", ph.type);

    // Read dynamic section and relocation entries
    if(ph.type == ELF_PROG_DYNAMIC) {
      Elf32_Dyn dyns[ph.filesz / sizeof(Elf32_Dyn)];
      if (readi(ip, (char*)&dyns, ph.off, sizeof(dyns)) != sizeof(dyns)) {
        goto bad_after_alloc;
      }

      uint rel_entry_sz = 0;
      uint rel_total_sz = 0;
      for(Elf32_Dyn const* dyn = dyns; dyn->d_tag != ELF_DYN_TAG_NULL; dyn++) {
        switch (dyn->d_tag) {
        case ELF_DYN_TAG_REL:
          rel_offset = dyn->d_un.d_ptr;
          cprintf("rel offset %d\n", rel_offset);
          break;
        case ELF_DYN_TAG_RELSZ:
          rel_total_sz = dyn->d_un.d_val;
          cprintf("rel total sz %d\n", rel_total_sz);
          break;
        case ELF_DYN_TAG_RELENT:
          rel_entry_sz = dyn->d_un.d_val;
          cprintf("rel entry sz %d\n", rel_entry_sz);
          break;
        default:
          break;
        }
      }
      if (rel_entry_sz == 0) {
        goto bad_after_alloc;
      }
      rel_count = rel_total_sz / rel_entry_sz;
      continue;
    }

    // Check if the segment is LOAD type
    if(ph.type != ELF_PROG_LOAD) {
      continue;
    }
    
    cprintf("got here\n");
    if(ph.memsz < ph.filesz) {
      goto bad_after_alloc;
    }

    // Make sure there is no overflow
    if(ph.vaddr + ph.memsz < ph.vaddr) {
      goto bad_after_alloc;
    }

    // Load the segment
    // First, allocate memory until the segment will fit
    while (ph.vaddr + ph.memsz >= (uint) (prog_end - prog_start)) {
      prog_end = next_page(mem);
      cprintf("now have %d pages allocated\n", (prog_end - prog_start) / PGSIZE);
      if (prog_end < prog_start) {
        // TODO: another failure case is allocating too much
        // The current implementation does not know what "too much" means
        goto bad_after_alloc;
      }
    }

    // TODO: think about whether filesz or memsz is right
    if (readi(ip, prog_start + ph.vaddr, ph.off, ph.filesz) != (int)ph.filesz) {
      goto bad_after_alloc;
    }
  }

  *entry = prog_start + elf.entry;

  // Perform relocation
  Elf32_Rel* rel_table = (Elf32_Rel*)(prog_start + rel_offset);
  for (uint i = 0; i < rel_count; i++) {
    uint* target = (uint*)(prog_start + rel_table[i].r_offset);
    cprintf("relocate 0x%x ->", *target);
    *target += (uint)prog_start;
    cprintf(" 0x%x at 0x%x\n", *target, target);
  }
  iunlockput(ip);
  end_op();
  ip = 0;
  cprintf("returning from kload_elf\n");
  return 0;

 bad_after_alloc:
  *mem = prog_start;
 bad:
  if(ip) {
    iunlockput(ip);
    end_op();
  }
  return -1;
}
