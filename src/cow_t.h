// Internal header for defining the `cow_t` concrete type to be used with the C and C++ APIs.
#ifndef _COW_T_H
#define _COW_T_H


#ifdef __cplusplus
#define restrict __restrict__
extern "C" {
#else
#include <stdbool.h>
#endif

#include <stdlib.h>

#include "macros.h"
#include <cow.h>

#include "error.h"

struct cow_mapped_slice {
	void* origin; // ptr to mapped memory. This *MUST* be the first field and have an offset of 0.

	size_t size; // Should be at this offset.
	int fd; // Will be ORd with ~INT_MAX if it's a clone. Will be >0 if it's the original.

	// For unboxed cow_ts. If there was an error constructing, it will be set to `true`.
	// If this is true. All resources held by this object will have been freed already.
	bool poisoned;
}; // cow_t, *cow

#ifdef __cplusplus
static_assert
#else
_Static_assert
#endif
		(offsetof(cow_t, origin) == 0, "`cow_t.origin` must have an offset of 0.");

#ifndef _COW_NO_ASSUME_ABI
#ifdef __cplusplus
static_assert
#else
_Static_assert
#endif
		(offsetof(cow_t, size) == sizeof(void*), "`cow_t.size` should have an offset equal to `sizeof(void*)` or cow_size_unsafe() becomes UB.");
#endif

cow_t _cow_create_unboxed(int rfd, size_t size) internal;
void _cow_free_unboxed(const cow_t* cow) internal;

#ifdef __cplusplus
}
#undef restruct
#endif

#endif /* _COW_T_H */
