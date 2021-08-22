#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SUCCESS 0

typedef void* (* thread_func)(void*);

typedef struct {
   pthread_t pointer;
   int status;
   void* join_data;
} thread;

thread create_thread(thread_func func, void* args) {
   thread t;
   t.status = pthread_create(&t.pointer, NULL, func, args);
   if (t.status != 0) {
      printf("main error: can't create thread, status = %d\n", t.status);
      return t;
   }
   return t;
}

thread detach_thread(thread this) {
   this.status = pthread_detach(this.pointer);
   if (this.status != SUCCESS) {
      printf("main error: can't detach thread, status = %d\n", this.status);
   }
   return this;
}

thread join_thread(thread this) {
   this.status = pthread_join(this.pointer, (void**) &this.join_data);
   if (this.status != SUCCESS) {
      printf("main error: can't join thread, status = %d\n", this.status);
   }
   return this;
}
