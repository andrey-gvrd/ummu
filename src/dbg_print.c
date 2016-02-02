#include "dbg_print.h"

#ifdef USE_DBG_PRINT
#include <stdio.h>	// vsprintf
#include <string.h> // memset
#include <stdarg.h> // va_list

char global_str[GLOBAL_STR_LEN] = {0};

void dbg_printf(const char * format, ...)
{
    va_list args;

    memset(global_str, 0, GLOBAL_STR_LEN);

    va_start(args, format);
    vsprintf(global_str, format, args);
    perror(global_str);
    va_end(args);

    DEBUG_SEND(global_str);
}
#else
void dbg_printf(const char * format, ...) {}
#endif // USE_DBG_PRINT
