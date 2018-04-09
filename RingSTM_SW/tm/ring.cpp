#include "ring.hpp"

MyRing::MyRing() {
  this->end = 0;
  BitFilter<1024> filt;
  RingEntry init_entry = {0, filt, 'c'};
  for (int i = 0; i < RING_SIZE; i++) {
    values[i] = init_entry;
  }
}

void MyRing::ring_add(RingEntry what) {
  this->values[this->end] = what;
  this->end = ((this->end + 1) % RING_SIZE);
}

RingEntry MyRing::ring_get(uintptr_t loc) {
  return values[loc % RING_SIZE];
}

void MyRing::ring_set(RingEntry what, uintptr_t loc) {
  this->values[loc % RING_SIZE] = what;
}

void MyRing::set_status(uintptr_t loc, char st) {
  values[loc % RING_SIZE].status = st;
}
