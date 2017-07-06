#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>


void *foo(void *t) {
    for(int i = 0; i < 3; i++) {
        printf("This is a loop");
    }
    return NULL;
}


int main(int argc, char* argv[]) { 
    printf("main: %lu", syscall(__NR_gettid));
    pthread_t t1, t2;
    pthread_create(&t1, NULL, foo, NULL);
    pthread_create(&t2, NULL, foo, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}

// RUN: clang -c -g -emit-llvm %s -o %t.bc 
// RUN: llvm-epp %t.bc -o %t.profile
// RUN: clang -v %t.epp.bc -o %t-exec -lepp-rt -lpthread 2> %t.compile 
// RUN: %t-exec > %t.log
