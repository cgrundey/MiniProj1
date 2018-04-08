/*
* Mini Project 1
*   by: Elton Jugi Gladstone Pushparaj, Colin Grundey
* http://www.xappsoftware.com/wordpress/2012/09/27/a-simple-implementation-of-a-circular-queue-in-c-language/
*/
#ifndef TM_HPP
#define TM_HPP 1

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "rand_r_32.h"
#include "WriteSet.hpp"
#include "BitFilter.h"

#define TABLE_SIZE 1048576

#define FORCE_INLINE __attribute__((always_inline)) inline
#define CACHELINE_BYTES 64
#define CFENCE  __asm__ volatile ("":::"memory")
#define MFENCE  __asm__ volatile ("mfence":::"memory")

using stm::WriteSetEntry;
using stm::WriteSet;

struct pad_word_t
{
  volatile uintptr_t val;
  char pad[CACHELINE_BYTES-sizeof(uintptr_t)];
};

#define nop()       __asm__ volatile("nop")
#define pause()		__asm__ ( "pause;" )

#define NUM_STRIPES  1048576



inline unsigned long long get_real_time() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &time);

  return time.tv_sec * 1000000000L + time.tv_nsec;
}

#define ACCESS_SIZE 102400

struct Tx_Context {
  int id;
  jmp_buf scope;
  uintptr_t start_time;
  BitFilter<1024> rf; // BitFilter initializes to 0
  BitFilter<1024> wf;
  WriteSet* writeset;
  long commits =0, aborts =0;
};

extern __thread Tx_Context* Self;

extern pad_word_t ring_index;

extern ring_t* theRing;


#define TM_TX_VAR	Tx_Context* tx = (Tx_Context*)Self;

#define TM_ARG , Tx_Context* tx
#define TM_ARG_ALONE Tx_Context* tx
#define TM_PARAM ,tx


#define TM_FREE(a)  /*nothing for now*/

#define TM_ALLOC(a) malloc(a)

FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly);
// Elton
FORCE_INLINE uint64_t tm_read(uint64_t* addr, Tx_Context* tx)
{
  // Looking in write set for addr
  WriteSetEntry log((void**)addr);
  bool found = tx->writeset->find(log);
  if (__builtin_expect(found, false))
    return log.val;

  /* RingSTM code here */
  uint64_t val = *addr;
  // add addr to read set signature
  // tx_validate()
  // return val;
}
// Colin
FORCE_INLINE void tm_write(uint64_t* addr, uint64_t val, Tx_Context* tx)
{
  // RingSTM Stuff
  tx->writeset->insert(WriteSetEntry((void**)addr, *((uint64_t*)(&val))));
  tx->wf.add(addr);
}
// Colin
FORCE_INLINE void tm_validate(Tx_Context* tx) {
  int my_index = ring_index;
  if (my_index == tx->start_time)
    return;
  while (theRing->values[my_index].ts < my_index)
    spin64();
  int i;
  for (i = my_index; i >= tx->start_time + 1; i--) {
    if (theRing->values[i].wf.intersect(tx->rf))
      abort(tx, 0);
  }
  while (theRing->values[my_index].status != "complete") {
    spin64();
  }
  fence(Read-Before-Read) // TODO
  tx->start_time = my_index
}

#define TM_READ(var)       tm_read(&var, tx)
#define TM_WRITE(var, val) tm_write(&var, val, tx)


FORCE_INLINE void spin64() {
  for (int i = 0; i < 64; i++)
    nop();
}

FORCE_INLINE void thread_init(int id) {
  if (!Self) {
    Self = new Tx_Context();
    Tx_Context* tx = (Tx_Context*)Self;
    tx->id = id;
    tx->writeset = new WriteSet(ACCESS_SIZE);
    // TODO: init other tx stuff
  }
}

FORCE_INLINE void tm_sys_init() {
  // Initialize Ring
  int i;
  for (i = 0; i < 1000; i++) { // TODO: length of ring???
    RingEntry temp = {0, wf, "complete"};
    ring_add(theRing, temp);
  }
}

FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly)
{
  tx->aborts++;
  //restart the tx
  longjmp(tx->scope, 1);
}
// Colin
FORCE_INLINE void tm_commit(Tx_Context* tx)
{
  if (tx->writeset->size() == 0) { //read-only
    return;
  }
  int commit_time;
  while (1) {
    commit_time = ring_index;
    tx_validate();
    if (__sync_bool_compare_and_swap(&ring_index, commit_time, commit_time+1))
      break;
  }
  theRing->values[commit_time+1] = {"writing", tx->wf, commit_time+1};
  int i;
  for (i = commit_time; i >= tx->start_time + 1; i--) {
    if (intersect(theRing->values[i].wf, tx->wf) {
      while(theRing->values[i].status == "writing")
        spin64();
    }
  }
  // TODO: Fence (read/write before write)

  tx->writeset->writeback();
  while(theRing->values[commit_time].status == "writing")
    spin64();
  // TODO: Fence (read/write before write)
  theRing->values[commit_time+1].status = "complete";

  tx->commits++;
}
// Elton
#define TM_BEGIN												\
{															\
  Tx_Context* tx = (Tx_Context*)Self;          			\
  uint32_t abort_flags = _setjmp (tx->scope);				\
  {														\
    tx->reads_pos =0;									\
    tx->writes_pos =0;									\
    tx->granted_writes_pos =0;							\
    tx->writeset->reset();								\
    tx->start_time = ring_index;
    // TODO
    // RV = ring_index


    #define TM_END                                  	\
    tm_commit(tx);                          \
  }											\
}

#endif //TM_HPP
