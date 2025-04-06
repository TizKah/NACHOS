/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_prod_cons.hh"
#include "condition.hh"
#include "system.hh"

#include <stdio.h>

bool prod_done = false;
bool cons_done = false;
int buffer[1024] = {0};
int bufferTop = 0;
Lock* lockCondition = new Lock("prod_cons_test_lock");
Condition *condition = new Condition("prod_cons_test_condition", lockCondition);


void Consumer(void * arg) {
    for (int item_number = 1; item_number <= 1000; item_number++) {
        lockCondition->Acquire();
        
        while (bufferTop <= 0) {
            printf("Consumidor esperando (buffer vacio)\n");
            condition->Wait();
        }
        
        bufferTop--;
        int consumer_value = buffer[bufferTop];
        printf("Consumidor consume: %d en %d\n", consumer_value, bufferTop);
        
        condition->Signal();
        lockCondition->Release();
    }
    cons_done = true;
}

void Producer(void *arg) {
    for (int item_number = 1; item_number <= 1000; item_number++) {
        lockCondition->Acquire();
        while (bufferTop >= 3) {
            printf("Productor esperando (buffer lleno)\n");
            condition->Wait();
        }
        
        buffer[bufferTop] = item_number;
        printf("Productor produce: %d en %d\n", item_number, bufferTop);
        bufferTop++;
        
        condition->Signal();
        lockCondition->Release();
    }
    prod_done = true;
}


void
ThreadTestProdCons()
{
    Thread* cons = new Thread("consumer_thr");
    Thread* prod = new Thread("producer_thr");
    
    prod->Fork(Producer, NULL);
    cons->Fork(Consumer, NULL);

    while(!(prod_done && cons_done)) {
        currentThread->Yield();
    }
    //Producer(NULL);
    //Consumer(NULL);
}
