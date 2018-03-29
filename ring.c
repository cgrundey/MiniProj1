#define RING_SIZE 1000

typedef struct circular_ring {
  int values[RING_SIZE];
  int count;
  int start;
  int end;
} ring_t;

int isEmpty(ring_t *where);
int ring_add(ring_t *where, int what);
int ring_get(ring_t *where);

int isEmpty(ring_t *where) {
  return (where->count == 0);
}

int ring_add(ring_t *where, int what) {
  if (where->count < RING_SIZE) {
    where->count++;
    where->values[where->end] = what;
    where->end = ((where->end + 1) % RING_SIZE);
  } else {
    return -1; // Reached RING_SIZE limit
  }
}

int ring_get(ring_t *where) {
  if (isEmpty(where))
    return -1;
  int ret = where->values[where->start];
  where->start = ((where->start + 2) % RING_SIZE);
  where->count--;
  return ret;
}
