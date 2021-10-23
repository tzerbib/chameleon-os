#include "types.h"
#include "defs.h"
#include "memlayout.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;


// RSDP table v1.0
// __attribute__((packed)) --> no padding between fields
struct RSDPDescriptor {
  char Signature[8];
  uint8_t Checksum;
  char OEMID[6];
  uint8_t Revision;
  uint32_t RsdtAddress;
}__attribute__((packed));


// RSDP table v2.0+
struct RSDPDescriptor20 {
  struct RSDPDescriptor firstPart;
 
  uint32_t Length;
  uint64_t XsdtAddress;
  uint8_t ExtendedChecksum;
  uint8_t reserved[3];
}__attribute__((packed));


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


// Check the RSDP table correctness //
uint8_t check_rsdp(struct RSDPDescriptor20* rsdp){
  cprintf("\tChecking for RSDP table integrity...\n");

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
  uint32_t* vebda_segptr = P2V(pebda_segptr);
  
  // physical addr (extracted from segment)
  uint16_t pebda = (*vebda_segptr << 8);

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

  // Check the RSDP table correctness //
  if(check_rsdp(rsdp) != 0){
    panic("RSDP cannot be relied on!\n");
  }

  cprintf("\tReliable RSDP table found at virtual address %p\n", rsdp);

  return (void*)rsdp;
}


// Search for the RSDP table
void rsdt_search(void){
  struct RSDPDescriptor20* rsdp = (struct RSDPDescriptor20*) rsdp_search();


  return;
}