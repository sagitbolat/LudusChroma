#pragma once
#include <stdint.h>

typedef size_t index_t; // NOTE: Indexing is done with size_t for portability, but this makes it more clear what the size_t represents.


// NOTE: Generatinga list with DEFINE_LIST(T) is necessary before using. 
// EXAMPLE: DEFINE_LIST(int) will generate a list of ints.

#define DEFINE_LIST(T) \
\
/*This is the base struct for the list.*/ \
\
struct T##_List { \
    T* data; \
    index_t capacity; \
    index_t length; \
    index_t Init(index_t init_capacity = 4); \
    index_t Append(T value); \
    index_t Insert(index_t index, T value); \
    index_t Remove(index_t index); \
}; \
\
/*list::Init(n) initializes the list to initial capacity of n (defaults to capacity 4)*/ \
\
index_t T##_List::Init(index_t init_capacity) { \
    this->data = (T*)calloc(init_capacity, sizeof(T)); \
    this->length = 0; \
    this->capacity = init_capacity; \
    return init_capacity; \
} \
\
/*list::Append(T value) appends the data to the end of the list in constant time (including resizing if necessary). Returns the new length of the list.*/ \
\
index_t T##_List::Append(T value) { \
    if (this->length >= this->capacity) { \
        this->capacity *=2; \
        this->data = (T*)realloc(this->data, this->capacity * sizeof(T)); \
    } \
    this->data[this->length++] = value; \
    return this->length; \
} \
\
/*list::Insert(index_t index, T value) inserts a value into specific index. O(n) time. Returns the new length of the list.*/ \
\
index_t T##_List::Insert(index_t index, T value) { \
    if (this->length >= this->capacity) { \
        this->capacity *=2; \
        this->data = (T*)realloc(this->data, this->capacity * sizeof(T)); \
    } \
    for (index_t i = this->length; i > index; --i) this->data[i] = this->data[i-1]; \
    this->data[index] = value; \
    this->length++; \
    return this->length; \
} \
\
/*list::Remove(index_t) removes the element in the specified index, then shifts the rest of the elements to keep the data contiguous. O(n) time. Returns the new length of the list.*/ \
\
index_t T##_List::Remove(index_t index) { \
    for (index_t i = index + 1; i < this->length; ++i) this->data[i-1] = this->data[i]; \
    this->length--; \
    return this->length; \
} \

