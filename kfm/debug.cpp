#include <stdio.h>  
#include <stdarg.h>      
#include <config-kfm.h>

void  debugT( const char *msg, ... )
{
#ifdef TORBENSDEBUG
    va_list ap;
    va_start( ap, msg );                        // use variable arg list
    vfprintf( stdout, msg, ap );          // Torben prefers stdout :-)
    va_end( ap );
#endif
}       

void debugC( const char *msg, ...)
{
#ifdef COOLOSDEBUG
    va_list ap;
    va_start( ap, msg );                        // use variable arg list
    vfprintf( stderr, msg, ap );        // I prefer stderr :-)
    fprintf(stderr, "\n");
    va_end( ap );
#endif
}       
