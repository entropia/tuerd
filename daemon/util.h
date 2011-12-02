#include <stdlib.h>

extern int debug;

#define log(format, ...) do { fprintf(stderr, format "\n", ##__VA_ARGS__); } while(0)
#define debug(format, ...) if(debug) { log(format, ##__VA_ARGS__); }
#define die(format, ...) do { log(format, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)
