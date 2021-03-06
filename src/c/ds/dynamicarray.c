#include <pebble.h>
#include "dynamicarray.h"

int ds_DynamicArray_init(ds_DynamicArray *a, size_t capacity) {
  a->data = (void**)malloc(capacity * sizeof(void*));
  if (a->data == NULL) return 1;

  a->length = 0;
  a->capacity = capacity;

  return 0;
}

int ds_DynamicArray_push(ds_DynamicArray *a, void *element) {
  if (a->capacity == 0) {
    if (ds_DynamicArray_init(a, 1) != 0) {
      return 1;
    }
  }
  else if (a->length == a->capacity) {
    a->capacity <<= 1;
    a->data = (void**)realloc(a->data, a->capacity * sizeof(void*));
    if (a->data == NULL) {
      a->capacity >>= 1;
      return 1;
    }
  }
  a->data[a->length++] = element;
  return 0;
}

void ds_DynamicArray_clear(ds_DynamicArray *a) {
  // probably would be smarter to have a
  // method that just resets the length to zero
  // to avoid freeing/reallocating all the time
  free((void*)a->data);
  a->data = NULL;
  a->length = a->capacity = 0;
}

void ds_DynamicArray_swap(ds_DynamicArray* first, ds_DynamicArray* second) {
  const size_t tmp1 = first->length;
  const size_t tmp2 = first->capacity;
  void **tmp3 = first->data;
  first->length = second->length;
  first->capacity = second->capacity;
  first->data = second->data;
  second->length = tmp1;
  second->capacity = tmp2;
  second->data = tmp3;
}
