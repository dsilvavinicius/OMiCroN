extern "C" void * scalable_malloc(size_t size);
extern "C" void   scalable_free(void* object);
extern "C" void * scalable_realloc(void* ptr, size_t size);
extern "C" void * scalable_calloc(size_t nobj, size_t size);

#define calloc  scalable_calloc
#define malloc  scalable_malloc
#define realloc scalable_realloc
#define free    scalable_free