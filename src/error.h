#ifndef _COW_ERROR_H
#define _COW_ERROR_H

#include <stdlib.h>

#include <cow.h>

#include "macros.h"

#ifdef __cplusplus
extern "C" {
#define _Thread_local thread_local
#define restrict __restrict__
#endif

void _cow_set_err(enum cow_err_kind kind) internal;
extern _Thread_local internal enum cow_err_kind _cow_last_error;

#define SOFT_ASSERT(ex, kind, ret) do { if(!(ex)) { return (_cow_last_error = (kind), (ret));  } } while(0)
#define SOFT_LASSERT(ex, kind, ret) SOFT_ASSERT(LIKELY(ex), kind, ret)
#define SOFT_UASSERT(ex, kind, ret) SOFT_ASSERT(UNLIKELY(ex), kind, ret)

#ifdef __cplusplus
}
#undef restrict
#undef _Thread_local
#endif

#endif /* _COW_ERROR_H */
