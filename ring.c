#define RING_SIZE 1000

typedef struct ringEntry {
  int ts;
  BitFilter wf;
  char *status; // writing or complete
} RingEntry;

typedef struct circular_ring {
  RingEntry values[RING_SIZE];
  int count;
  int start;
  int end;
} ring_t;

int isEmpty(ring_t *where);
int ring_add(ring_t *where, RingEntry what);
RingEntry ring_get(ring_t *where);

int isEmpty(ring_t *where) {
  return (where->count == 0);
}

int ring_add(ring_t *where, RingEntry what) {
  if (where->count < RING_SIZE) {
    where->count++;
    where->values[where->end] = what;
    where->end = ((where->end + 1) % RING_SIZE);
    return 0;
  } else {
    return -1; // Reached RING_SIZE limit
  }
}

RingEntry ring_get(ring_t *where) {
  if (isEmpty(where))
    return NULL;
  RingEntry ret = where->values[where->start];
  where->start = ((where->start + 2) % RING_SIZE);
  where->count--;
  return ret;
}
