/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// -- Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "syscall.h"
#include "filesys/directory_entry.hh"
#include "threads/system.hh"
#include "../lib/assert.hh"
#include "address_space.hh"
#include "args.hh"

#include <stdio.h>
#include <exception_type.hh>
#include <../filesys/open_file.hh>
#include <system.hh>

#define DEFAULT_NEW_FILE_SIZE 10000
#define MAX_BUFFER_SIZE 1024

static void
IncrementPC()
{
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// NOTE: this function is meant specifically for unexpected exceptions.  If
/// you implement a new behavior for some exception, do not extend this
/// function: assign a new handler instead.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et)
{
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);
    ASSERT(false);
}

// unsigned vpn, TranslationEntry **entry
// vpn -> virtual page number
// **entry -> tlb[N]
// Funcion que carga a la TLB la fila de la page table relacionada a la vpn que se pide

// -> debo agregar a la TLB una entrada con vpn, physicalPage, valid = true
// readOnly = false, use = false, dirty = false

//class TranslationEntry:
//    unsigned virtualPage; unsigned physicalPage; bool valid; 
//    bool readOnly; bool use; bool dirty

static int tlb_to_replace = 0;

static void 
PageFaultHandler(ExceptionType et) {
    int virtual_address = machine->ReadRegister(BAD_VADDR_REG);
    int virtual_address_page = virtual_address / PAGE_SIZE;
    DEBUG('e', "PageFaultHandler: PageFaultException generated\n");

    if(!currentThread->space->pageTable[virtual_address_page].valid){
        // En caso de no ser válida, cargamos en la TLB una página nueva 
        machine->GetMMU()->tlb[tlb_to_replace] = *currentThread->space->NewPage(virtual_address_page);
    }
    else {
        // En caso de ser válida la traemos del espacio de memoria para guardarla en la TLBs
        machine->GetMMU()->tlb[tlb_to_replace] = currentThread->space->pageTable[virtual_address_page];
    }


    tlb_to_replace = (tlb_to_replace + 1) % TLB_SIZE;
}

static void
newProcessThread(void* args) {
    DEBUG('e', "newProcessThread: Entering in the function\n");
    currentThread->space->InitRegisters();  
    currentThread->space->RestoreState();   
    machine->Run(); 
}

static void
newProcessThreadArgs(void* argsV) {
    DEBUG('e', "newProcessThreadArgs: Entering in the function\n");
    currentThread->space->InitRegisters(); 
    currentThread->space->RestoreState();  

    char ** args = (char**) argsV;
    int argsCount = WriteArgs(args);
    int argsAddr = machine->ReadRegister(STACK_REG);

    machine->WriteRegister(4, argsCount);
    machine->WriteRegister(5, argsAddr);

    // De args.hh:
    /// CAUTION: if you intend to use this to pass command-line arguments to a
    /// new process in the C way (by passing function arguments to the starting
    /// function), then make sure to take into account the function call argument
    /// area.  This consists of some space at the top of the stack that has to be
    /// reserved when making a function call with arguments; the called function
    /// can use the area to store arguments passed in registers.  This is
    /// mandated by the MIPS ABI, and its omission may cause corruption of
    /// either `argv` or stored register values.  The area must be able to hold
    /// the 4 arguments that can be passed in registers: `a0..4`, hence it
    /// occupies 16 bytes.  Therefore, the problem is solved by substracting 24
    /// to `sp`.
    machine->WriteRegister(STACK_REG,  argsAddr - 24);

    // Codigo para debugear mas a fondo
    // DEBUG('e', "Verifying argv array in user stack AFTER WriteArgs:\n");
    // int ptr_val;
    // for (int i = 0; i < argsCount + 1; i++) { // +1 para el puntero NULL final
    //     int current_ptr_location = argsAddr + (i * 4); // Ubicación del puntero actual
    //     bool success = machine->ReadMem(current_ptr_location, 4, &ptr_val);
    //     if (success) {
    //         DEBUG('e', "  argv[%d] at vaddr %d contains value 0x%x (%d decimal)\n",
    //               i, current_ptr_location, ptr_val, ptr_val);
    //     } else {
    //         DEBUG('e', "  Failed to read argv[%d] from vaddr %d\n", i, current_ptr_location);
    //     }
    // }
    // char bufferArgs[MAX_BUFFER_SIZE];
    // int virtual_addr;
    // DEBUG('e', "Arguments count: %d\n", argsCount);
    // for( int i = 0 ; i < argsCount ; i++ ){
    //     machine -> ReadMem(argsAddr + (4*i), 4, &virtual_addr);
    //     ReadStringFromUser(virtual_addr, bufferArgs, MAX_BUFFER_SIZE);
    //     DEBUG('e', "%d - nth argument: %s, virtual addr: %d \n", i, bufferArgs, virtual_addr);
    // }

    DEBUG('e', "newProcessThreadArgs: Readed args %d, argsAddr %d\n", argsCount, argsAddr);

    machine->Run(); 
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void
SyscallHandler(ExceptionType _et)
{
    int scid = machine->ReadRegister(2);

    switch (scid) {

        case SC_HALT:
            DEBUG('e', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        /// Create a Nachos file, with `name`.
        // -- int Create(const char *name);
        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "`Create` requested for file `%s`.\n", filename);
            int file_created = fileSystem->Create(filename, DEFAULT_NEW_FILE_SIZE);
            if (!file_created) {
                DEBUG('e', "Error: cannot create file.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            
            machine->WriteRegister(2, 1);
            break;
        }

        /// Remove the Nachos file named `name`.
        // -- int Remove(const char *name);
        case SC_REMOVE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            
            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "Removing %s file.\n", filename);
            int removed = fileSystem->Remove(filename);
            machine->WriteRegister(2, removed);
            break;
        }

        /// to read and write to the file.
        // OpenFileId Open(const char *name);
        case SC_OPEN: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "'Open' Opening %s file.\n", filename);
            OpenFile* open_file = fileSystem->Open(filename);
            if(!open_file) {
                DEBUG('e', "Error: file %s not found", filename);
                machine->WriteRegister(2, -1);
                break;
            }

            int open_file_add_idx = currentThread->open_files->Add(open_file);
            DEBUG('e', "'Open' Opened %s file have id %d.\n", filename, open_file_add_idx);
            if(open_file_add_idx == -1) {
                DEBUG('e', "Error: cannot add file.\n");
                machine->WriteRegister(2, -1);
                delete open_file;
                break;
            }
            machine->WriteRegister(2, open_file_add_idx);
            break;
        }

        /// Write `size` bytes from `buffer` to the open file.
        // -- int Write(const char *buffer, int size, OpenFileId id);
        case SC_WRITE: {
            int bufferAddr = machine->ReadRegister(4);
            
            int size = machine->ReadRegister(5);
            DEBUG('e', "'Write' write size %d'\n", size);

            if(size <= 0){
                DEBUG('e', "Error: not valid size.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            
            char buffer[size + 1];
            ReadBufferFromUser(bufferAddr, buffer, size);
            
            OpenFileId open_file_idx = machine->ReadRegister(6);
            // DEBUG('e', "Syscall: SC_WRITE received - bufferAddr=0x%x, size=%d, fileId=%d\n", bufferAddr, size, open_file_idx);
            if(open_file_idx == CONSOLE_OUTPUT) {
                DEBUG('e', "'Write' Writing in console.\n");
                for(int i = 0; i < size; i++) {
                    synch_console->PutChar(buffer[i]);
                    // DEBUG('e', "  [WRITE_LOOP] Writing char %d: '%c'\n", i, buffer[i]);
                }
            } 
            else {
                OpenFile* open_file = currentThread->open_files->Get(open_file_idx);
                if(!open_file){
                    DEBUG('e', "Error: file not open.\n");
                    machine->WriteRegister(2, -1);
                    break;
                }

                DEBUG('e', "'Write' Writing in file %d.\n", open_file_idx);
                if(open_file->Write(buffer, size) < size) {
                    DEBUG('e', "Error: cannot write.\n");
                    machine->WriteRegister(2, -1);
                    delete open_file;
                    break;
                }
            }

            machine->WriteRegister(2, size);
            break;
        }

        /// Read `size` bytes from the open file into `buffer`.
        ///
        /// Return the number of bytes actually read -- if the open file is not long
        /// enough, or if it is an I/O device, and there are not enough characters to
        /// read, return whatever is available (for I/O devices, you should always
        /// wait until you can return at least one character).
        // -- int Read(char *buffer, int size, OpenFileId id);
        case SC_READ: {
            int bufferAddr = machine->ReadRegister(4);
            
            int size = machine->ReadRegister(5);
            if(size <= 0){
                DEBUG('e', "Error: not valid size.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            
            OpenFileId open_file_idx = machine->ReadRegister(6);
            
            char buffer[size + 1];
            int num_readed = 0;
            if(open_file_idx == CONSOLE_INPUT) {
                DEBUG('e', "'Read' Reading from console.\n");
                for(; num_readed < size; num_readed++) {
                    buffer[num_readed] = synch_console->GetChar();
                    if(buffer[num_readed] == '\n') {
                        num_readed++;
                        break;
                    }
                }
                // DEBUG('e', "'Read' read %d ch from console.\n", num_readed);
                machine->WriteRegister(2, num_readed);
            }
            else {
                OpenFile* open_file = currentThread->open_files->Get(open_file_idx);
                if(!open_file){
                    DEBUG('e', "Error: file not open.\n");
                    machine->WriteRegister(2, -1);
                    break;
                }
                DEBUG('e', "'Read' Reading from file %d.\n", open_file_idx);
                num_readed = open_file->Read(buffer, size);
                machine->WriteRegister(2, num_readed);
            }
            if(num_readed) WriteBufferToUser(buffer, bufferAddr, num_readed);
            break;
        }

        /// Close the file, we are done reading and writing to it.
        // -- int Close(OpenFileId id);
        case SC_CLOSE: {
            OpenFileId open_file_idx = machine->ReadRegister(4);
            // open_file_idx == CONSOLE_INPUT || open_file_idx == CONSOLE_OUTPUT
            if((open_file_idx >> 1) == 0){
                DEBUG('e', "Cant close stdout or stdin.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            OpenFile* deleted_file = currentThread->open_files->Remove(open_file_idx);
            if(!deleted_file) {
                DEBUG('e', "Error: file not created.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            
            DEBUG('e', "`Close` requested for id %u.\n", open_file_idx);
            machine->WriteRegister(2, 1);
            break;
        }

        /// Run the executable, stored in the Nachos file `name`, and return the
        /// address space identifier.
        /// --- SpaceId Exec(char *name);
        case SC_EXEC: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "'Exec': Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "'Exec': Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }
            
            OpenFile* exe = fileSystem->Open(filename);
            if(!exe) {
                DEBUG('e', "'Exec': Error: file %s not created.\n", filename);
                machine->WriteRegister(2, -1);
                break;
            }

            Thread* thread = new Thread(filename, 4, true);
            AddressSpace* addrSpace = new AddressSpace(exe);
            DEBUG('e', "'Exec': New thread and addrSpace created.\n");

            thread->space = addrSpace;
            SpaceId spaceId = thread->threadId;
            DEBUG('e', "'Exec': Executing new program, with new threadId %d with name %s.\n", spaceId, thread->GetName());
            thread->Fork(newProcessThread, NULL);

            DEBUG('e', "'Exec': Fork executed, father %s.\n", currentThread->GetName());
            delete exe;
            machine->WriteRegister(2, spaceId);
            break;
        }

        // --- SpaceId Exec2(char *name, void* args);
        case SC_EXEC2: {
            int filenameAddr = machine->ReadRegister(4);
            int argsAddr = machine->ReadRegister(5);

            if (filenameAddr == 0) {
                DEBUG('e', "'Exec': Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "'Exec': Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }
            
            OpenFile* exe = fileSystem->Open(filename);
            if(!exe) {
                DEBUG('e', "'Exec': Error: file %s not created.\n", filename);
                machine->WriteRegister(2, -1);
                break;
            }

            Thread* thread = new Thread(filename, 4, true);
            AddressSpace* addrSpace = new AddressSpace(exe);
            DEBUG('e', "'Exec': New thread and addrSpace created.\n");

            thread->space = addrSpace;
            SpaceId spaceId = thread->threadId;
            DEBUG('e', "'Exec': Executing new program, with new threadId %d with name %s.\n", spaceId, thread->GetName());
            thread->Fork(newProcessThreadArgs, SaveArgs(argsAddr));

            DEBUG('e', "'Exec': Fork executed, father %s.\n", currentThread->GetName());
            delete exe;
            machine->WriteRegister(2, spaceId);
            break;
        }
        /// Only return once the the user program `id` has finished.
        /// Return the exit status.
        /// --- int Join(SpaceId id);
        case SC_JOIN: {
            SpaceId spaceId = machine->ReadRegister(4);
            if(spaceId < 0) {
                DEBUG('e', "'Join': Invalid spaceId %d\n", spaceId);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "'Join': ThreadId %d waiting threadId %d to finish\n", currentThread->threadId, spaceId);
            DEBUG('e', "'Join': Thread %s waiting thread %s to finish\n", currentThread->GetName(), current_threads->Get(spaceId)->GetName());
            int status = current_threads->Get(spaceId)->Join();
            
            DEBUG('e', "'Join': Process %d finished with status %d\n", spaceId, status);
            machine->WriteRegister(2, status);
            break;
        }

        /// This user program is done (`status = 0` means exited normally).
        /// -- void Exit(int status);
        case SC_EXIT: {
            int status = machine->ReadRegister(4);
            DEBUG('e', "'Exit': Requested with status %d.\n", status);
            DEBUG('e', "'Exit': Thread %s finishing.\n", currentThread->GetName());
            currentThread->Finish(status);
            break;
        }
        
        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
}


/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers()
{
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &PageFaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
