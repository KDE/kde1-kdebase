#include <stdio.h>  
#include <stdarg.h>      
#include <config-kfm.h>

#ifdef TORBENSDEBUG
void  debugT( const char *msg, ... )
{
    va_list ap;
    va_start( ap, msg );                        // use variable arg list
    vfprintf( stdout, msg, ap );          // Torben prefers stdout :-)
    va_end( ap );
#else
void debugT(const char *, ... )
{
#endif
}       

#ifdef COOLSDEBUG
void debugC( const char *msg, ...)
{
    va_list ap;
    va_start( ap, msg );                        // use variable arg list
    vfprintf( stderr, msg, ap );        // I prefer stderr :-)
    fprintf(stderr, "\n");
    va_end( ap );
#else
void debugC(const char *, ... )
{
#endif
}       
