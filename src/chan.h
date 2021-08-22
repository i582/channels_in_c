#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define CHAN_FOREACH(elem, ok, chan, action)        \
while (1) {                                         \
    chan_data elem = rec_chan(chan);                \
    int ok = 1;                                     \
    if (elem == NULL) {                             \
         ok = 0;                                    \
    }                                               \
    action                                          \
    if (chan_closed(chan) && chan_len(chan) == 0) { \
        break;                                      \
    }                                               \
}

#define true 1
#define false 0

typedef int bool;
typedef void* chan_data;

typedef struct {
   size_t chan_size;
   size_t count_elems;
   chan_data* buffer;
   bool closed;

   pthread_mutex_t lock;
} chan;

chan* create_chan(size_t size) {
   if (size == 0) {
      size = 1;
   }
   chan* ch = (chan*)malloc(sizeof(chan));
   ch->buffer = (chan_data*)malloc(sizeof(chan_data) * size);
   ch->chan_size = size;
   ch->count_elems = 0;
   ch->closed = 0;
   pthread_mutex_init(&ch->lock, NULL);
   return ch;
}

size_t chan_len(chan* ch) {
   return ch->count_elems;
}

bool chan_closed(chan* ch) {
   return ch->closed;
}

void close_chan(chan* ch) {
   ch->closed = true;
}

void free_chan(chan* ch) {
   if (ch->buffer != NULL) {
      free(ch->buffer);
   }
   pthread_mutex_destroy(&ch->lock);
   free(ch);
}

void send_chan(chan* ch, chan_data data) {
   if (ch->closed) {
      printf("send on closed channel\n");
      return;
   }

   while (ch->count_elems == ch->chan_size) {
      usleep(1000 * 100);
   }

   pthread_mutex_lock(&ch->lock);
   ch->buffer[ch->count_elems] = data;
   ch->count_elems++;
   pthread_mutex_unlock(&ch->lock);
}

chan_data rec_chan(chan* ch) {
   while (ch->count_elems == 0) {
      if (chan_closed(ch)) {
         return NULL;
      }
      usleep(1000 * 100);
   }

   pthread_mutex_lock(&ch->lock);
   chan_data el = ch->buffer[ch->count_elems-1];
   ch->count_elems--;
   pthread_mutex_unlock(&ch->lock);
   return el;
}
