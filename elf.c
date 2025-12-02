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

int read_relocation(struct inode *ip, struct proghdr* php, uint* rel_offset, uint* rel_count) {
      Elf32_Dyn dyns[php->filesz / sizeof(Elf32_Dyn)];
      if (readi(ip, (char*)&dyns, php->off, sizeof(dyns)) != sizeof(dyns)) {
        return -1;
      }

      uint rel_entry_sz = 0;
      uint rel_total_sz = 0;
      for(Elf32_Dyn const* dyn = dyns; dyn->d_tag != ELF_DYN_TAG_NULL; dyn++) {
        switch (dyn->d_tag) {
        case ELF_DYN_TAG_REL:
          *rel_offset = dyn->d_un.d_ptr;
          break;
        case ELF_DYN_TAG_RELSZ:
          rel_total_sz = dyn->d_un.d_val;
          break;
        case ELF_DYN_TAG_RELENT:
          rel_entry_sz = dyn->d_un.d_val;
          break;
        default:
          break;
        }
      }
      if (rel_entry_sz == 0) {
        return -1;
      }
      *rel_count = rel_total_sz / rel_entry_sz;
      return 0;
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
  ilock(ip);
  
  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf)) {
    goto bad;
  }
  if(elf.magic != ELF_MAGIC) {
    goto bad;
  }
  
  char* prog_start = *mem;
  char const* prog_end = prog_start;

  uint rel_offset = 0;
  uint rel_count = 0;
  
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)) {
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph)) {
      goto bad_after_alloc;
    }

    // Read dynamic section and relocation entries
    if(ph.type == ELF_PROG_DYNAMIC) {
      if (read_relocation(ip, &ph, &rel_offset, &rel_count) == -1) {
        goto bad_after_alloc;
      }
      continue;
    }

    // Check if the segment is LOAD type
    if(ph.type != ELF_PROG_LOAD) {
      continue;
    }
    
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
    *target += (uint)prog_start;
  }
  iunlockput(ip);
  end_op();
  ip = 0;
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
