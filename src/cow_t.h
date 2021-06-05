// Internal header for defining the `cow_t` concrete type to be used with the C and C++ APIs.
#ifndef _COW_T_H
#define _COW_T_H

#define internal __attribute__((visibility("internal")))

#ifdef __cplusplus
#define restrict __restrict__
extern "C" {
#endif

#include <stdlib.h>

#include <cow.h>

struct cow_mapped_slice {
	void* origin; // ptr to mapped memory. This *MUST* be the first field and have an offset of 0.

	int fd; // Will be ORd with ~INT_MAX if it's a clone. Will be >0 if it's the original.
	size_t size;
}; // cow_t, *cow

#ifdef __cplusplus
static_assert
#else
_Static_assert
#endif
		(offsetof(cow_t, origin) == 0, "`cow_t.origin` must have an offset of 0.");

cow_t _cow_create_unboxed(size_t size) internal;
void _cow_free_unboxed(const cow_t* cow) internal;

#ifdef __cplusplus
}
#undef restruct
#endif

#endif /* _COW_T_H */
