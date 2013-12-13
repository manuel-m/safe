#ifdef _WIN32

#include <stdarg.h>
#include <stdlib.h>
#include "mmwincompat.h"



static int vasprintf(char **ret, const char *format, va_list args)
{
    va_list copy;
    va_copy(copy, args);

    *ret = 0;

    int count = vsnprintf(NULL, 0, format, args);
    if (count >= 0)
    {
        char* buffer = (char*)malloc(count + 1);
        if (buffer != NULL)
        {
            count = vsnprintf(buffer, count + 1, format, copy);
            if (count < 0)
			{
                free(buffer);
			}
            else
			{
                *ret = buffer;
			}
        }
    }
    va_end(args);

    return count;
}


int asprintf(char **ret, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int count = vasprintf(ret, format, args);
    va_end(args);
    return(count);
}


#endif /* _WIN32 */