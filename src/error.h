#ifndef _COW_ERROR_H
#define _COW_ERROR_H

#include <stdlib.h>

#include <cow.h>

#include "macros.h"

#ifdef __cplusplus
extern "C" {
#define restrict __restrict__
#endif

/*
Failed:
typedef struct cow_error cow_error;
union poison {
	struct cow_error {
		cow_error* prev;
		
		const char* message;

		enum cow_err  kind  : 31;
		/// Should we free this instance?
		unsigned char owned : 1;
	} e_static;
	cow_error* e_global;
};

#define COW_ERROR_NONE ((struct cow_error){ .prev = NULL, .message = NULL, .kind = COW_ERR_SUCCESS, .owned = 0 })
#define COW_POISON_NONE ((union poison){ .e_static = COW_ERROR_NONE})

void _cow_poison(cow_t* restrict cow, enum cow_err kind, const char *msg) internal;
void _cow_poison_ref(cow_t* restrict cow, cow_error* globl) internal;
*/

#ifdef __cplusplus
}
#undef restrict
#endif

#endif /* _COW_ERROR_H */
