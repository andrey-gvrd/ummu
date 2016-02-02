#ifndef __DBG_PRINT_H__
#define __DBG_PRINT_H__

#include <string.h> // strcpy, strcat, strlen
#include <stdio.h>  // sprintf
#include <stdarg.h> // va_list

#define USE_DBG_PRINT

#ifdef USE_DBG_PRINT
    #define GLOBAL_STR_LEN 64
    extern char global_str[GLOBAL_STR_LEN];

    /* Requires system-specific implementation */
    #define DEBUG_SEND(str) \
        do {                \
                            \
        } while (0)         \

    #define SEND_STR(s)                         \
        do {                                    \
            static const char rom_str[] = s;    \
            DEBUG_SEND(s);                      \
        } while (0)

    #define SEND_INT(i)                                     \
        do {                                                \
            memset(global_str, 0, GLOBAL_STR_LEN);          \
            snprintf(global_str, GLOBAL_STR_LEN, "%d", i);  \
            DEBUG_SEND(s);                                  \
        } while (0)
#else
    #define SEND_STR(s) do {} while (0)
    #define SEND_INT(i) do {} while (0)
#endif // USE_DBG_PRINT

void dbg_printf(const char * format, ...);

#endif // __DBG_PRINT_H__
