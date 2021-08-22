#include "src/thread.h"
#include "src/chan.h"

typedef struct {
   int id;
   chan* in;
   chan* out;
} worker_args;

// The main function that performs something for a long time.
void* squareWorkerFunc(void* raw_args) {
   worker_args* args = raw_args;

   // Iterate over the values in the 'in' channel.
   // If there are no values in the channel at the moment, the execution
   // will be paused until the value is received.
   CHAN_FOREACH(data, ok, args->in, {
      if (!ok) {
         continue;
      }
      int num = *(int*) data;
      int* res = malloc(sizeof(int));
      *res = num * num;
      send_chan(args->out, res);

      // Sleep to simulate long work.
      usleep(1000 * 100);
   })

   // Close the channel to notify that we need to finish iterating through the channel.
   close_chan(args->out);
   return 0;
}

// A worker that will print the results from squareWorkerFunc.
void* printWorkerFunc(void* raw_args) {
   worker_args* args = raw_args;

   // Iterate over the values in the 'out' channel.
   // The 'out' channel writes squareWorkerFunc.
   CHAN_FOREACH(data, ok, args->out, {
      if (!ok) {
         continue;
      }
      int num = *(int*) data;
      printf("%d\n", num);
      free(data);
   })

   return 0;
}

int main() {
   chan* in = create_chan(10000);
   chan* out = create_chan(10000);

   worker_args args = {0, in, out};

   // Create several workers to get the job done.
   int count_workers = 8;
   thread squareWorkers[count_workers];
   for (int i = 0; i < count_workers; ++i) {
      squareWorkers[i] = create_thread(squareWorkerFunc, &args);
   }

   // And one for print results.
   thread printWorker = create_thread(printWorkerFunc, &args);

   time_t cur = time(NULL);

   // Send the data to be processed.
   int count_data = 100;
   int* data = malloc(count_data * sizeof(int));
   for (int i = 0; i < count_data; ++i) {
      data[i] = i;
      send_chan(in, &data[i]);
   }

   // Close the channel to notify that we need to finish iterating through the channel.
   close_chan(in);

   // Waiting for the completion of all workers.
   for (int i = 0; i < count_workers; ++i) {
      join_thread(squareWorkers[i]);
   }
   join_thread(printWorker);

   printf("Runtime: %ld\n", time(NULL) - cur);

   // Release the channels.
   free_chan(in);
   free_chan(out);

   free(data);
   return 0;
}
