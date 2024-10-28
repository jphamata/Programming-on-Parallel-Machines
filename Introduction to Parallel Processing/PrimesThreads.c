// PrimesThreads.c

// threads-based program to find the number of primes between 2 and n;
// uses the Sieve of Eratosthenes, deleting all multiples of 2, all
// multiples of 3, all multiples of 5, etc.

// for illustration purposes only; NOT claimed to be efficient

// Unix compilation: gcc -g -o primesthreads PrimesThreads.c -lpthread -lm

// usage: primesthreads n num_threads

#include <math.h>
#include <pthread.h> // required for threads usage
#include <stdio.h>
#include <stdlib.h>

#define MAX_N 100000000
#define MAX_THREADS 25

// shared variables
int n, // range to check for primeness
    nextbase, // next sieve multiplier to be used
    nthreads; // number of threads (not counting main())

int prime[MAX_N + 1]; // in the end, prime[i] = 1 if i prime, else 0

// lock for the shared variable nextbase
pthread_mutex_t nextbaselock = PTHREAD_MUTEX_INITIALIZER;

// ID structs for the threads
pthread_t id[MAX_THREADS];

// "crosses out" all odd multiples of k
void crossout (int k)
{
    int i;

    for (i = 3; i * k <= n; i += 2) {
        prime[i * k] = 0;
    }
}

// each thread runs this routine
void *worker(int tn) // tn is the thread number (0, 1, ...)
{
    int base,
        lim,
        work; // amount of work done by this thread

    work = 0;

    // no need to check multipliers bigger than sqrt(n)
    lim = sqrt(n);

    do {
        // get next sieve multiplier, avoiding duplication across threads
        // lock the lock
        pthread_mutex_lock(&nextbaselock);

        base = nextbase;

        nextbase += 2;

        // unlock
        pthread_mutex_unlock(&nextbaselock);

        if (base <= lim) {
            // don’t bother crossing out if base known composite
            if (prime[base]) {
                crossout(base);

                work++; // log work done by this thread
            }
        }
            
        else return work;
    } while (1);
}

int main(int argc, char **argv)
{
    int i,
        nprimes, // number of primes found
        work;

    n = atoi(argv[1]);

    nthreads = atoi(argv[2]);

    // mark all even numbers nonprime, and the rest "prime until
    // shown otherwise"
    for (i = 3; i <= n; i++) {
        if (i % 2 == 0) prime[i] = 0;
        else prime[i] = 1;
    }

    nextbase = 3;

    // get threads started
    for (i = 0; i < nthreads; i++) {
        // this call says to create a thread, record its ID in the array
        // id, and get the thread started executing the function worker(),
        // passing the argument i to that function
        pthread_create(&id[i], NULL, worker, i);
    }

    // wait for all done
    for (i = 0; i < nthreads; i++) {
        // this call said to wait until thread number id[i] finishes
        // execution, and to assign the return value of that thread to our
        // local variable work here
        pthread_join(id[i], &work);

        printf("%d values of base done\n", work);
    }

    // report results
    nprimes = 1;

    for (i = 3; i <= n; i++) if (prime[i]) nprimes++;

    printf("the number of primes found was %d\n", nprimes);

    return 0;
}
