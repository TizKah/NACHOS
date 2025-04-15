#include "thread_test_join.hh"
#include "thread.hh"
#include <stdio.h>

void func(void* _arg) {
    int a= 0;
    for(int i = 0; i < 1000; i++) {
        a += i;
        printf("a: %d\n",a);
    }
    printf("Fork terminado\n");
}

void ThreadTestJoin() {
    Thread *t = new Thread("Hijo", DEFAULT_PRIORITY, true);
    t->Fork(func, nullptr);
    t->Join();
    printf("Vuelvo a main\n");
}