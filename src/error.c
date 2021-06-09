#include "error.h"

_Thread_local internal enum cow_err_kind _cow_last_error = COW_ERR_NONE;
