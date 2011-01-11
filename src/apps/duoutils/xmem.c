/**
 * @file  xmem.c
 * @brief Implements memory allocation/deallocation
 *
 * Copyright (c) 2007 Duolabs S.r.l.
 *
 * Changelog:
 * Date    Author      Comments
 * ------------------------------------------------------------------------
 * 301007  paguilar    Original
 */

/*****************************
 * INCLUDES
 *****************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***********************************************
 * MACROS                                      *
 ***********************************************/

/*****************************
 * FUNCTION IMPLEMENTATION
 *****************************/

/**
 * @brief Allocates num blocks of memory of a given size
 *        The new allocated memory is zeroed
 * @param size Size of unit
 * @param num Number of units to be allocated
 * @return The allocated memory
 */
void * xmalloc(size_t size, int num) {
    void        *ptr = NULL;

    if (size == 0 || num < 1) {
        fprintf(stderr, "Invalid size or number of elements!");
        return (void *)NULL;
    }

    if ((ptr = malloc(size * num)) == (void *)NULL) {
        fprintf(stderr, "Could not allocate memory!");
        return (void *)NULL;
    }

    memset(ptr, '0', size);
    return ptr;
}

/**
 * @brief Changes the size of the memory block pointed to by ptr 
 *        to size bytes, the new reallocated memory is cleared
 * @param ptr Memory to whics its size will be changed
 * @param size New size
 */
void * xrealloc(void *ptr, size_t size) {
    void        *new_ptr = NULL;

    if (size == 0)
        return NULL;

    if (ptr == NULL)
        return NULL;

    if ((new_ptr = realloc(ptr, size)) == (void *)NULL) {
        fprintf(stderr, "FATAL: Could not allocate memory!");
        return NULL;
    }

    memset(new_ptr, '0', size);
    return ptr;
}

/**
 * @brief Frees the memory pointed by ptr
 * @param ptr Pointer to the memory that will be freed
 */
void xfree(void *ptr) {
    if (ptr != NULL)
        free(ptr);
}

