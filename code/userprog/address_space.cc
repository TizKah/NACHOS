/// Routines to manage address spaces (memory for executing user programs).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "executable.hh"
#include "threads/system.hh"

#include <string.h>

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
AddressSpace::AddressSpace(OpenFile *executable_file)
{
    ASSERT(executable_file != nullptr);

    Executable exe (executable_file);
    ASSERT(exe.CheckMagic());

    // How big is address space?

    unsigned size = exe.GetSize() + USER_STACK_SIZE;
      // We need to increase the size to leave room for the stack.
    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

    ASSERT(numPages <= physical_page_bitmap->CountClear());
      // Check we are not trying to run anything too big -- at least until we
      // have virtual memory.

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    // First, set up the translation.

    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
        int phys_new_page = physical_page_bitmap->Find();
        ASSERT(phys_new_page != -1);
        pageTable[i].virtualPage  = i;
        pageTable[i].physicalPage = phys_new_page;
        pageTable[i].valid        = true;
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
          // If the code segment was entirely on a separate page, we could
          // set its pages to be read-only.
    }

    char *mainMemory = machine->mainMemory;

    for (unsigned i = 0; i < numPages; i++) {
      memset(&mainMemory[pageTable[i].physicalPage*PAGE_SIZE],0,PAGE_SIZE);
    }
    uint32_t codeSize = exe.GetCodeSize();
    uint32_t initDataSize = exe.GetInitDataSize();

    if (codeSize > 0) {
        uint32_t virtualAddr = exe.GetCodeAddr();
        DEBUG('z', "Initializing code segment, at 0x%X, size %u\n",
              virtualAddr, codeSize);
        uint32_t virtualPage    = virtualAddr/PAGE_SIZE;
        uint32_t offset        = virtualAddr%PAGE_SIZE;
        uint32_t currentCodeSize = codeSize;
        uint32_t sizeToRead = min(PAGE_SIZE-offset,currentCodeSize);

        exe.ReadCodeBlock(&mainMemory[(pageTable[virtualPage].physicalPage*PAGE_SIZE)+offset], sizeToRead, 0);
        currentCodeSize -= sizeToRead;
        virtualPage++;
        for(;currentCodeSize > 0; virtualPage++){
          sizeToRead = min(PAGE_SIZE,currentCodeSize);
          exe.ReadCodeBlock(&mainMemory[pageTable[virtualPage].physicalPage*PAGE_SIZE], sizeToRead, codeSize-currentCodeSize);
          currentCodeSize -= sizeToRead;
        }
    }
    if (initDataSize > 0) {
        uint32_t virtualAddr = exe.GetInitDataAddr();
        DEBUG('z', "Initializing data segment, at 0x%X, size %u\n", virtualAddr, initDataSize);

        uint32_t virtualPage    = virtualAddr/PAGE_SIZE;
        uint32_t offset        = virtualAddr%PAGE_SIZE;
        uint32_t currentIDataSize = initDataSize;
        uint32_t sizeToRead = min(PAGE_SIZE-offset, currentIDataSize);

        exe.ReadDataBlock(&mainMemory[(pageTable[virtualPage].physicalPage*PAGE_SIZE)+offset], sizeToRead, 0);
        currentIDataSize -= sizeToRead;
        virtualPage++;
        for(;currentIDataSize > 0; virtualPage++){
          sizeToRead = min(PAGE_SIZE,currentIDataSize);
          exe.ReadDataBlock(&mainMemory[pageTable[virtualPage].physicalPage*PAGE_SIZE], sizeToRead, initDataSize-currentIDataSize);
          currentIDataSize -= sizeToRead;
        }
    }

}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
    for (unsigned i = 0; i < numPages; i++) {

        physical_page_bitmap->Clear(pageTable[i].physicalPage);
        
    }
    delete [] pageTable;
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++) {
        machine->WriteRegister(i, 0);
    }

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void
AddressSpace::SaveState()
{}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void
AddressSpace::RestoreState()
{
  #ifdef USE_TLB
    for(int i = 0; i < TLB_SIZE; i++) {
      machine->GetMMU()->tlb[i].valid = false;
    }
  #else 
    machine->GetMMU()->pageTable     = pageTable;
  #endif

  machine->GetMMU()->pageTableSize = numPages;
}

TranslationEntry*
AddressSpace::NewPage(int virtual_address_space) {
  pageTable[virtual_address_space].virtualPage = virtual_address_space;
  pageTable[virtual_address_space].physicalPage = physical_page_bitmap->Find();
  pageTable[virtual_address_space].valid = true;
  pageTable[virtual_address_space].readOnly = false;
  pageTable[virtual_address_space].use = false;
  pageTable[virtual_address_space].dirty = false;
  return &pageTable[virtual_address_space];
}
