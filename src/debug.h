#ifndef _BR_DEBUG_H
#define _BR_DEBUG_H

#ifdef DEBUG
#define DEBUG_PRINT(...) do {                                 \
  fprintf(stdout, "Debug: %s:%d: ", __FILE__, __LINE__);      \
  fprintf(stdout, __VA_ARGS__ );                              \
  fprintf(stdout, "\n");                                      \
} while(0)

#define DEBUG_PRINT_ERR(...) do { fprintf(stderr, __VA_ARGS__ ); } while(0)
#else
#define DEBUG_PRINT(...) do {} while (0)
#define DEBUG_PRINT_ERR(...) do {} while (0)
#endif

#endif