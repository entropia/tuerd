#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern int debug;

#define log(format, ...) do { fprintf(stderr, format "\n", ##__VA_ARGS__); } while(0)
#define log_errno(format, ...) do { int errno_local = errno; fprintf(stderr, format ": %s\n", ##__VA_ARGS__, strerror(errno_local)); } while(0)
#define debug(format, ...) if(debug) { log(format, ##__VA_ARGS__); }
#define die(format, ...) do { log(format, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)
