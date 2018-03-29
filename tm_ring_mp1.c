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

struct lock_entry {
  volatile uint64_t lock_owner;
  volatile uint64_t version;
};


extern lock_entry* lock_table;

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
  int reads_pos;
  uint64_t reads[ACCESS_SIZE];
  int writes_pos;
  int granted_writes_pos;
  uint64_t writes[ACCESS_SIZE];
  bool granted_writes[ACCESS_SIZE];
  WriteSet* writeset;
  long commits =0, aborts =0;
};

extern __thread Tx_Context* Self;

extern pad_word_t global_clock;

#define TM_TX_VAR	Tx_Context* tx = (Tx_Context*)Self;

#define TM_ARG , Tx_Context* tx
#define TM_ARG_ALONE Tx_Context* tx
#define TM_PARAM ,tx


#define TM_FREE(a)  /*nothing for now*/

#define TM_ALLOC(a) malloc(a)

FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly);

FORCE_INLINE uint64_t tm_read(uint64_t* addr, Tx_Context* tx)
{
  // Looking in write set for addr
  WriteSetEntry log((void**)addr);
  bool found = tx->writeset->find(log);
  if (__builtin_expect(found, false))
  return log.val;


  uint64_t index = (reinterpret_cast<uint64_t>(addr)>>3) % TABLE_SIZE;
  lock_entry* entry_p = &(lock_table[index]);

  /* RingSTM code here */
  uint64_t val = *addr;
  // add addr to read set signature
  // tx_validate()
  // return val;
}

FORCE_INLINE void tm_write(uint64_t* addr, uint64_t val, Tx_Context* tx)
{
  // RingSTM Stuff
  tx->wset.add(addr, val);
  tx->wf.add(addr);

  // TL2 Stuff _______________________________________________
  bool alreadyExists = tx->writeset->insert(WriteSetEntry((void**)addr, *((uint64_t*)(&val))));
  if (!alreadyExists) {
    int w_pos = tx->writes_pos++;
    tx->writes[w_pos] = (reinterpret_cast<uint64_t>(addr)>>3) % TABLE_SIZE;
    tx->granted_writes[w_pos] = false;
  }
  // TL2 Stuff _______________________________________________
}

FORCE_INLINE void tm_validate(Tx_Context* tx) {
  my_index=ring_index
  if (my_index == tx->start_time)
    return;
  while (ring[my_index].ts < my_index)
    spin64();
  for i=my_index downto tx->start_time + 1
    if intersect(ring[i].wf, TX.rf)
    abort(tx, 0);
  while ring[my_index].st!=complete
    spin64();
  fence(Read-Before-Read)
  TX.start = my_index
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
  }
}

FORCE_INLINE void tm_sys_init() {
  lock_table = (lock_entry*) malloc(sizeof(lock_entry) * TABLE_SIZE);

  for (int i=0; i < TABLE_SIZE; i++) {
    lock_table[i].lock_owner = 0;
    lock_table[i].version = 0;
  }
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

  again: commit_time = ring_index;
  tx_validate();
  if (!__sync_bool_compare_and_swap(&ring_index, commit_time, commit_time+1))
    // goto again
  ring[commit_time+1] = {writing, tx->wf, commit_time+1};
  for (i = commit_time downto tx->start_time + 1) {
    if (intersect(ring[i].wf, tx->wf) {
      while(ring[i].status == writing)
        spin64();
    }
  }
  // TODO: Fence (read/write before write)

  tx->writeset->writeback();
  while(ring[commit_time].status == writing)
    spin64();
  // TODO: Fence (read/write before write)
  ring[commit_time+1].status = complete;

  tx->commits++;
}

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


    #define TM_END                                  	\
    tm_commit(tx);                          \
  }											\
}

#endif //TM_HPP
