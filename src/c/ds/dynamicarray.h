#pragma once
#include <pebble.h>

typedef struct {
  void **data;
  size_t length;
  size_t capacity;
} ds_DynamicArray;

int ds_DynamicArray_init(ds_DynamicArray*, size_t) ;
int ds_DynamicArray_push(ds_DynamicArray*, void*) ;
void ds_DynamicArray_clear(ds_DynamicArray*) ;
void ds_DynamicArray_swap(ds_DynamicArray* first, ds_DynamicArray* second);