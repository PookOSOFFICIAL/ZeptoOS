#pragma once

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed long long int64_t;
typedef signed int int32_t;
typedef signed short int16_t;
typedef signed char int8_t;
#ifdef __x86_64__
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
#else
typedef uint32_t uintptr_t;
typedef int32_t intptr_t;
#endif
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
#define NULL ((void *)0)
