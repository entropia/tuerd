extern int debug;

#define log(format, ...) do { fprintf(stderr, format "\n", ##__VA_ARGS__); } while(0)
#define debug(format, ...) if(debug) { log(format, ##__VA_ARGS__); }
