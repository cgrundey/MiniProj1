#ifndef RING_H__
#define RING_H__

#include "BitFilter.h"

#define RING_SIZE 10000

typedef struct ringEntry {
  uintptr_t ts;
  BitFilter<128> wf;
  char status; // 'w' = writing or 'c' = complete
} RingEntry;

class MyRing {
public:
  MyRing();
  void ring_add(RingEntry what);
  RingEntry ring_get(uintptr_t loc);
  void ring_set(RingEntry what, uintptr_t loc);
  void set_status(uintptr_t loc, char st);

private:
  RingEntry values[RING_SIZE];
  int end;
};


#endif
