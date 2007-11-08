#include "config.h"

#if !HAVE_REALLOCF
void * reallocf(void *ptr, size_t size);
#endif
