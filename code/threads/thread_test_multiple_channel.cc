#include "channel.hh"
#include "system.hh"
#include <stdio.h>
#include "thread_test_multiple_channel.hh"

Channel* globalChannel;


const int NUM_SENDERS = 3;
const int NUM_RECEIVERS = 3;
bool senders_done[NUM_SENDERS] = {false};
bool receivers_done[NUM_RECEIVERS] = {false};

void SenderThread(int id) {
    for (int i = 0; i < 5; ++i) {
        int msg = id + i;
        globalChannel->Send(msg);
        currentThread->Yield();  
    }
    senders_done[id] = true;
}


void ReceiverThread(int id) {
    for (int i = 0; i < 5; ++i) {
        int msg;
        globalChannel->Receive(&msg);
        currentThread->Yield();  
    }
    receivers_done[id] = true;
}


void TestMultipleSendersReceivers() {
    globalChannel = new Channel();

    for (int i = 0; i < NUM_SENDERS; i++) {
        char* name = new char[16];
        snprintf(name, 16, "Sender-%d", i);
        Thread* t = new Thread(name);
        t->Fork((VoidFunctionPtr) SenderThread, (void*) i);
    }

    
    for (int i = 0; i < NUM_RECEIVERS; i++) {
        char* name = new char[16];
        snprintf(name, 16, "Receiver-%d", i);
        Thread* t = new Thread(name);
        t->Fork((VoidFunctionPtr) ReceiverThread, (void*) i);
    }

    int max = NUM_RECEIVERS > NUM_SENDERS ? NUM_RECEIVERS : NUM_SENDERS;
    for(int i = 0; i < max; i++){
        while (!(receivers_done[i] && senders_done[i])) {
            currentThread->Yield();
        }
    }

}
