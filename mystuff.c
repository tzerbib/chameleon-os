#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"


#define AVAILADDR ((void*)0x1000000)

extern pte_t* kpgdir;                                   // defined by vm.c
extern pte_t* walkpgdir(pte_t*, const void*, int);      // defined by vm.c
extern int mappages(pde_t*, void*, uint, uint, int);    // defined by vm.c


// RSDP table v1.0
struct RSDPDescriptor {
  char Signature[8];
  uint8_t Checksum;
  char OEMID[6];
  uint8_t Revision;
  uint32_t RsdtAddress;
}__attribute__((packed)); // --> no padding between fields


// RSDP table v2.0+
struct RSDPDescriptor20 {
  struct RSDPDescriptor firstPart;
 
  uint32_t Length;
  uint64_t XsdtAddress;
  uint8_t ExtendedChecksum;
  uint8_t reserved[3];
}__attribute__((packed));


// RSTD table Header
struct ACPISDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
};


// RSDT complete table 
struct RSDT {
  struct ACPISDTHeader h;
  uint32_t PointerToOtherSDT[];
};


// SRAT table
struct SRAT
{
    char signature[4];   // Contains "SRAT"
    uint32_t length;     // Length of entire SRAT including entries
    uint8_t  rev;        // 3
    uint8_t  checksum;   // Entire table must sum to zero
    uint8_t  OEMID[6];   // What do you think it is?
    uint64_t OEMTableID; // For the SRAT it's the manufacturer model ID
    uint32_t OEMRev;     // OEM revision for OEM Table ID
    uint32_t creatorID;  // Vendor ID of the utility used to create the table
    uint32_t creatorRev; // Blah blah
 
    uint8_t reserved[12];
} __attribute__((packed));



////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

// Look in a specified memory area for the RSDP signature
struct RSDPDescriptor20* find_rsdp(uint32_t addr, uint32_t size){
  struct RSDPDescriptor20* ptr = (void*) 0;
  
  // Browse the memory area
  // The signature is on a 16-byte boundary
  for(uint32_t offset=0; offset<size; offset+=4){
    ptr = P2V(addr + offset);
    
    // Compare signature to expected string
    if(!memcmp(ptr->firstPart.Signature, "RSD PTR ", 8)){
      return ptr;
    }
  }
  
  return (void*) 0;
}


// Check the RSDP table correctness
uint8_t check_rsdp(struct RSDPDescriptor20* rsdp){
  int checksum = 0;
  struct RSDPDescriptor* limit = &rsdp->firstPart + 1;
  for(uint8_t* i=(uint8_t*)&rsdp->firstPart; i<(uint8_t*)limit; ++i){
    checksum += *i;
  }
  
  if((uint8_t)checksum){
    return 1;
  }

  // One more check for ACPI v2.0 to 6.1
  if(rsdp->firstPart.Revision){
    checksum = 0;
    struct RSDPDescriptor20* limit20 = rsdp + 1;
    for(uint8_t* i=(uint8_t*)&rsdp->Length; i<(uint8_t*)limit20; ++i){
      checksum += *i;
    }

    if((uint8_t)checksum){
      return 1;
    }
  }

  return 0;
}


// Search for the RSDP table
void* rsdp_search(void){
  cprintf("Looking for RSDP table...\n");
  struct RSDPDescriptor20* rsdp;
  
  // First search zone: Extended Bios Data Area (EBDA)
  // Real mode segment (2 bytes) containing EBDA
  uint32_t pebda_segptr = 0x40E;
  uint16_t* vebda_segptr = P2V(pebda_segptr);
  
  // physical addr (extracted from segment)
  uint32_t pebda = (*vebda_segptr << 8);

  cprintf(
    "\tSearching into EBDA (pa: %x, va: %p)...\n",
    pebda, P2V((uint32_t)pebda)
  );
  rsdp=find_rsdp(pebda, 1024);
  
  // If RSDP table is not found
  if(!rsdp){
    // Second search zone: memory region from 0x000E0000 to 0x000FFFFF
    cprintf(
      "\tSearching between physical addresses %p and %p...\n",
      0x000E0000, 0x000FFFFF
    );

    rsdp = find_rsdp(0x000E0000, 0x000FFFFF - 0x000E0000);
  }

  // Failure
  if(!rsdp){
    panic("RSDP not found\n");
  }

  // Check the RSDP table correctness
  cprintf("\tChecking for RSDP table integrity...\n");
  if(check_rsdp(rsdp) != 0){
    panic("RSDP cannot be relied on!\n");
  }

  cprintf(
    "\tReliable RSDP table found at virtual address %p (%p)\n",
    rsdp, V2P(rsdp))
  ;

  return (void*)rsdp;
}


// Convert a physical address to a virtual address for rsdt tables
void* rsdt_p2v(uint32_t paddr){
  uint32_t page = paddr & -PGSIZE;
  return (void*)(AVAILADDR + (paddr - page));
}


// Check the RSDT table correctness
uint8_t check_rsdt(struct RSDT* rsdt){
  uint8_t checksum = 0;
  for(int i=0; i<rsdt->h.Length; ++i){
    checksum += ((uint8_t*)rsdt)[i];
  }
  
  return (checksum == 0);
}


// Search for a table given the rsdt table and the looked for signature
void* findRSDTSub(struct RSDT* rsdt, const char signature[4]){
  int entries = (rsdt->h.Length - sizeof(struct ACPISDTHeader)) / 4;
  struct ACPISDTHeader* curr;

  for(int i=0; i<entries; i++){
      curr = (struct ACPISDTHeader*) rsdt_p2v((uint32_t) rsdt->PointerToOtherSDT[i]);

      if (!strncmp(curr->Signature, signature, 4))
          return (void *) curr;
  }

  // No table found
  return (void*)0;
}


// Search for the RSDP table
void* rsdt_search(void){
  struct RSDPDescriptor20* rsdp = (struct RSDPDescriptor20*) rsdp_search();

  uint32_t prsdt = 0;
  // For ACPI v2.0 to 6.1
  if(rsdp->firstPart.Revision){
    prsdt = (uint32_t) rsdp->XsdtAddress;
  }
  else{
    prsdt = rsdp->firstPart.RsdtAddress;
  }
  
  uint32_t aligned_rsdt = prsdt & -PGSIZE;
  mappages(kpgdir, AVAILADDR, 0x1000000, aligned_rsdt, PTE_P);

  // Compute virtual address of rsdt by adding offset in page
  struct RSDT* vrsdt = rsdt_p2v(prsdt);

  // Check the RSDT table correctness
  cprintf("\tChecking for RSDT table integrity...\n");
  if(!check_rsdt(vrsdt)){
    panic("RSDT cannot be relied on!\n");
  }

  cprintf(
    "\tReliable RSDT table found at virtual address %p (%p)\n",
    vrsdt, prsdt
  );

  return vrsdt;
}


void srat_search(void){
  struct RSDT* rsdt = (struct RSDT*) rsdt_search();

  struct SRAT* srat = findRSDTSub(rsdt, "SRAT");
  if(!srat){
    panic("SRAT not found\n");
  }

  return;
}