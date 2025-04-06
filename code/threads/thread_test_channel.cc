#include "channel.hh"
#include "system.hh"
#include "thread_test_channel.hh"
#include <stdio.h>

#define SENDER_RECEIVER_AMOUNT 1000

static Channel *ch = new Channel();
bool sender_done = false;
bool receiver_done = false;


void Sender(void* arg) {
    for(int i = 1; i <= SENDER_RECEIVER_AMOUNT; i++) {
        ch->Send(i);
    }
    sender_done = true;
}

void Receiver(void* arg) {
    int received;
    for(int i = 1; i <= SENDER_RECEIVER_AMOUNT; i++) {
        ch->Receive(&received);
    }
    receiver_done = true;
}

void ThreadTestChannel() {
    Thread* sender = new Thread("sender");
    Thread* receiver = new Thread("receiver");
    sender->Fork(Sender, NULL);
    receiver->Fork(Receiver, NULL);

    while(!(receiver_done && sender_done)) {
        currentThread->Yield();
    }

    delete sender;
    delete receiver;
    delete ch;
}
