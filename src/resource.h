#ifndef RESOURCE_H
#define RESOURCE_H

#ifdef _WIN32
#define RESOURCESTARTSYM(x) binary_ ## x ## _start
#define RESOURCESIZESYM(x) binary_ ## x ## _size
#else
#define RESOURCESTARTSYM(x) _binary_ ## x ## _start
#define RESOURCESIZESYM(x) _binary_ ## x ## _size
#endif

#define DECLRES(x) extern char RESOURCESTARTSYM(x); \
    extern char RESOURCESIZESYM(x)

#define RESOURCE(x) ((void *)& RESOURCESTARTSYM(x))
#define RESSIZE(x) ((size_t)& RESOURCESIZESYM(x))

#endif
