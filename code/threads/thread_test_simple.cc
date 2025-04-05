/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_simple.hh"
#include "semaphore.hh"
#include "system.hh"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.

/// NOTE: The total amount of threads will be N_THREADS + 1 (the "main" thread). 
static const unsigned N_THREADS = 4; 
static bool done[N_THREADS] = {false};

#ifdef SEMAPHORE_TEST
static Semaphore sem = Semaphore(NULL, 3);
#endif

static void
SimpleThread(void *name_)
{

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {

#ifdef SEMAPHORE_TEST
        sem.P();
        DEBUG('s', "Thread %s has made a P() operation.\n", currentThread->GetName());        
#endif

        printf("*** Thread `%s` is running: iteration %u\n", currentThread->GetName(), num);

#ifdef SEMAPHORE_TEST
        sem.V();
        DEBUG('s', "Thread %s has made a V() operation.\n", currentThread->GetName());        
#endif

        currentThread->Yield();
    }
    // Falta flag para que no siga el bucle al asignar true a uno ya.
    bool already_done = false;
    for(unsigned thr_number = 0; thr_number < N_THREADS && !already_done; thr_number++) {   
        char actual_thr_number[1024];
        sprintf(actual_thr_number, "%d", thr_number);
        if (strcmp(currentThread->GetName(),actual_thr_number)==0) {
	        done[thr_number] = true;
            already_done = true;
        }
    }
    printf("!!! Thread `%s` has finished SimpleThread\n", currentThread->GetName());
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void
ThreadTestSimple()
{
    Thread** threads = (Thread **)malloc(sizeof(Thread*) * N_THREADS);
    char** thread_names = (char **)malloc(sizeof(char*) * 1024);

    for(unsigned thr_number = 0; thr_number < N_THREADS; thr_number++) {
        thread_names[thr_number] = (char *)malloc(sizeof(char) * 1024);
        
        sprintf(thread_names[thr_number], "%d", thr_number);

        threads[thr_number] = new Thread(thread_names[thr_number]);
        threads[thr_number]->Fork(SimpleThread, NULL);        
    }

    //the "main" thread also executes the same function
    SimpleThread(NULL);

    for(unsigned thr_number = 0; thr_number < N_THREADS; thr_number++) {   
        while(!done[thr_number]) {
            currentThread->Yield();
        }
    }

    printf("Test finished\n");

    // Free all the memory
    for (unsigned thr_number = 0; thr_number < N_THREADS; thr_number++) {
        free(thread_names[thr_number]);
        delete[] threads[thr_number];
    }
    free(threads);
    free(thread_names);
}
