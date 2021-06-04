#ifndef _COW_H
#define _COW_H

#include <stdlib.h>

// Copy-on-write mapped memory.
typedef struct cow cow_t, *cow;

/// Create a new copy-on-write area of `size` bytes. 
/// Writes to this instance pointer (`cow_ptr()`) are written to the allocated memory.
/// Writes to any cloned instances do not propagate to this instance.
cow_t* cow_create(size_t size);
/// Free a cow area. This should be called on all clones before the parent of those clones is freed.
void cow_free(cow_t* restrict cow);
/// Create a clone of this instance. Any writes to the returned pointer will not be propagated to the input one.
cow_t* cow_clone(const cow_t* cow);

/// Returns 1 if this instance is a clone. 0 if it is not.
int cow_is_fake(const cow_t* cow);
/// Get the size of this cow area.
size_t cow_size(const cow_t* cow);

/// Get the `void*` pointer to the start of the area.
#define cow_ptr(v) (*((void**)(v)))
/// Get the `T*` pointer to the start of the area.
#define cow_ptr_of(T, v) (*((T **)(v)))

#endif /* _COW_H */
