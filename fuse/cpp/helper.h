#ifndef __HELPER__
#define __HELPER__

#include <stdio.h>

#define VDEBUG 0

#define DEBUG(v ...) fprintf(stderr, "[DEBUG] " v)
#if VDEBUG
#define INFO(v ...)  fprintf(stderr, "[INFO]  " v)
#else
#define INFO(v ...)
#endif

#define OFFSET_OF(t, m)       ((unsigned long)&((t *)0)->m)
#define CONTAINER_OF(d, t, m) ((t *)(((char *)d) - OFFSET_OF(t, m)))

#endif
