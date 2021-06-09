#ifndef _COW_H
#define _COW_H

#ifdef __cplusplus
#define restrict __restrict__
extern "C" {
#endif

#include <stdlib.h>

enum cow_err_kind {
	COW_ERR_UNKNOWN =0,
	COW_ERR_NONE =1, // Success

	/// `memfd_create()` failed.
	COW_ERR_FDCREATE,
	/// `ftruncate()` (set size) failed.
	COW_ERR_SIZE,
	/// `mmap()` failed.
	COW_ERR_MAP,

	_COW_ERR_SIZE,
};
// Message string corresponding to this error.
const char* const * cow_err_msg(enum cow_err_kind kind);
// The last error that `libcow` produced on this thread.
enum cow_err_kind cow_err();

// Copy-on-write mapped memory.
typedef struct cow_mapped_slice cow_t;

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

/// Get the size of this cow area by assuming layout. This should work assuming "cow_t.h"'s build assertions didn't fail and avoids an extra call.
/// XXX: Deprecated function unpon seeing preferable codegen for using `cow_size()` over this. Use `cow_size()` instead.
static inline
#ifdef _COW_NO_ASSUME_ABI
#define _cow_size_unsafe(v) cow_size(v)
#else
// XXX: This macro is *VERY* ABI sensitive. This shouldn't be used if the ABI has changed since the build of libcow's `cow_t.h` passed its stati assertions in *both* the C and C++ implementations.
// The C++ API uses this by default for its `Cow::size()` function.
#define _cow_size_unsafe(v) (*(((size_t*)(v))+1))
	__attribute__((deprecated("size() is safer and offers better codegen."))) 
#endif
	size_t cow_size_unsafe(const cow_t* v) { return _cow_size_unsafe(v); }

/// Get the `void*` pointer to the start of the area.
#define cow_ptr(v) (*((void**)(v)))
/// Get the `T*` pointer to the start of the area.
#define cow_ptr_of(T, v) (*((T **)(v)))
///// Get the `cow_t*` back from a pointer created with cow_ptr. 
///// NOTE: This pointer **MUST NOT** be a *copy* of a pointer created with `cow_ptr`, but the pointer returned from `cow_ptr` exactly.
//XXX: Too unsafe and not useful enough to warrant a function/macro.
//#define cow_from_ptr(p) (cow_t*)(&(p))

#ifdef __cplusplus
#undef restrict
}
#endif

#endif /* _COW_H */
