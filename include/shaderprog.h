#ifndef SHADERPROG_H
#define SHADERPROG_H
#if defined(_WIN64)
#include <sys/types.h>
#include <sys/stat.h>
#elif defined(__linux__)
#include <sys/stat.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>



GLuint shaderprog_load(const char*,const char*,const char*);



#endif
