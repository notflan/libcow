
#ifndef internal
#define internal __attribute__((visibility("internal")))
#endif /* internal */

#define LIKELY(ex) __builtin_expect(!!(ex), 1)
#define UNLIKELY(ex) __builtin_expect(!!(ex), 0)

#define box(t) aligned_alloc(_Alignof(t), sizeof(t))

#if defined(DEBUG) || defined(COW_TRACE)
#define TRACE(msg, ...) (fprintf(stderr, "<libcow> [TRACE] %s->%s():%d: " msg "\n", __FILE__, __func__, __LINE__, __VA_ARGS__), (void)0)
#else
#define TRACE(msg, ...) ((void)0)
#endif

#if !defined(COW_NO_ASSERT)
#define ASSERT(expr, msg) do { if(!(expr)) die("assertion failed: `" #expr  "`: " msg);  } while(0)
#else
#define ASSERT(op, msg) ((void)0)
#endif

#define LASSERT(expr, msg) ASSERT(LIKELY(expr), "(unexpected) " msg)
#define UASSERT(expr, msg) ASSERT(UNLIKELY(expr), "(expected) " msg)


