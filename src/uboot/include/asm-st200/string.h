/*
 * include/asm-st200/string.h
 *
 * Copyright (C) 2002 STMicroelectronics Limited
 *	Author: Thierry Strudel <thierry.strudel@st.com>
 *
 */

#ifndef _ASM_SH_STRING_H
#define _ASM_SH_STRING_H


extern char *strcpy(char *__dest, const char *__src);

extern char *strncpy(char *__dest, const char *__src, size_t __n);

extern int strcmp(const char *__cs, const char *__ct);

extern int strncmp(const char *__cs, const char *__ct, size_t __n);

extern void *memset(void *__s, int __c, size_t __count);

#define __HAVE_ARCH_MEMCPY
extern void *memcpy(void *__to, __const__ void *__from, size_t __n);

extern void *memmove(void *__dest, __const__ void *__src, size_t __n);

extern void *memchr(const void *__s, int __c, size_t __n);

#define __HAVE_ARCH_MEMSET
extern void * memset(void * s,int c,size_t count);

#endif /* _ASM_SH_STRING_H */
