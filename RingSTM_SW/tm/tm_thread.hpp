/*
* Mini Project 1
*   by: Elton Jugi Gladstone Pushparaj, Colin Grundey
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
#include "ring.hpp"

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
  BitFilter<128> rf; // range that works: <64> to <1024>
  BitFilter<128> wf;
  WriteSet* writeset;
  long commits =0, aborts =0;
};

extern __thread Tx_Context* Self;

extern pad_word_t ring_index;

extern MyRing* theRing;


#define TM_TX_VAR	Tx_Context* tx = (Tx_Context*)Self;

#define TM_ARG , Tx_Context* tx
#define TM_ARG_ALONE Tx_Context* tx
#define TM_PARAM ,tx


#define TM_FREE(a)  /*nothing for now*/

#define TM_ALLOC(a) malloc(a)

FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly);

FORCE_INLINE void tm_write(uint64_t* addr, uint64_t val, Tx_Context* tx)
{
  tx->writeset->insert(WriteSetEntry((void**)addr, *((uint64_t*)(&val))));
  tx->wf.add(addr);
}

#define TM_READ(var)       tm_read(&var, tx)
#define TM_WRITE(var, val) tm_write(&var, val, tx)


FORCE_INLINE void spin64() {
  for (int i = 0; i < 64; i++)
    nop();
}

FORCE_INLINE void tm_validate(Tx_Context* tx) {
  uintptr_t my_index = ring_index.val;
  if (my_index == tx->start_time)
    return;
  while (theRing->ring_get(my_index).ts < my_index)
    spin64();
  uintptr_t i;
  for (i = my_index; i >= tx->start_time + 1; i--) {
    if (theRing->ring_get(i).wf.intersect(&(tx->rf)))
      tm_abort(tx, 0);
  }
  while (theRing->ring_get(my_index).status != 'c') {
    spin64();
  }
  CFENCE;
  tx->start_time = my_index;
}

FORCE_INLINE uint64_t tm_read(uint64_t* addr, Tx_Context* tx)
{
WriteSetEntry log((void**)addr);
bool found = tx->writeset->find(log);

if( (tx->rf).lookup(addr) && found)
{
  return log.val;
}
uint64_t val = *addr;
(tx->rf).add(addr);
CFENCE;
tm_validate(tx);
return val;
}

FORCE_INLINE void thread_init(int id) {
  if (!Self) {
    Self = new Tx_Context();
    Tx_Context* tx = (Tx_Context*)Self;
    tx->id = id;
    tx->writeset = new WriteSet(ACCESS_SIZE);
  }
}

FORCE_INLINE void tm_sys_init() {
  // Initialize Ring
  theRing = new MyRing();
}

FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly)
{
  tx->aborts++;
  //restart the tx
  longjmp(tx->scope, 1);
}

FORCE_INLINE void tm_commit(Tx_Context* tx)
{
  if (tx->writeset->size() == 0) { //read-only
    return;
  }
  uintptr_t commit_time;
  while (1) {
    commit_time = ring_index.val;
    tm_validate(tx);
    if (__sync_bool_compare_and_swap(&(ring_index.val), commit_time, commit_time+1))
      break;
  }
  RingEntry temp = {commit_time+1, tx->wf, 'w'};
  theRing->ring_set(temp, commit_time+1);
  CFENCE;
  tx->writeset->writeback();
  CFENCE;
  theRing->set_status(commit_time+1, 'c');

  tx->commits++;
}

#define TM_BEGIN												
{															
  Tx_Context* tx = (Tx_Context*)Self;  			
  uint32_t abort_flags = _setjmp (tx->scope);				
  {									
    uintptr_t index = tx.start_time;
  while( theRing->ring_get(index).ts < tx.start_time || theRing->ring_get(index).status != 'c' )
  (tx->start_time)--;
  CFENCE;      
    #define TM_END                                  	
    tm_commit(tx);                          
  }											
}

#endif //TM_HPP
