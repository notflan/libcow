#ifndef _COW_H
#define _COW_H

#include <stdlib.h>

// Copy-on-write mapped memory.
typedef struct cow cow_t, *cow;

cow_t* cow_create(size_t size);
void cow_free(cow_t* restrict cow);
cow_t* cow_clone(const cow_t* cow);

int cow_is_fake(const cow_t* cow);
size_t cow_size(const cow_t* cow);

#define cow_ptr(v) (*((void**)(v)))
#define cow_ptr_of(t, v) (*((t **)(v)))

#endif /* _COW_H */
