/* Define if the C++ compiler supports BOOL */
#undef HAVE_BOOL

#undef VERSION 

#undef PACKAGE

/* defines if you have dlopen and co */
#undef HAVE_DYNAMIC_LOADING

/* defines if having libgif (always 1) */
#undef HAVE_LIBGIF

/* defines if having libjpeg (always 1) */
#undef HAVE_LIBJPEG

/* defines if having libtiff */
#undef HAVE_LIBTIFF

/* defines if having libpng */
#undef HAVE_LIBPNG

/* defines which to take for ksize_t */
#undef ksize_t

/* define if you have setenv */
#undef HAVE_FUNC_SETENV

/* define if shadow under linux */
#undef HAVE_SHADOW

/* define if you have XPM support */
#undef HAVE_XPM

/* define if you have GL (Mesa, OpenGL, ...)*/
#undef HAVE_GL

/* define if you have PAM (Pluggable Authentication Modules); Redhat-Users! */
#undef HAVE_PAM

/* define this if you  compile --with-pam for SOLARIS (but not for Linux) */
#undef PAM_MESSAGE_NONCONST

/* if defined, changes the default name of the PAM service used by KDE */
#undef KDE_PAM_SERVICE

/* Define to 1 if NLS is requested.  */
#undef ENABLE_NLS

/* define if you have the PAM lib. Now, we have two different users, this will change */
#undef HAVE_PAM_LIB

/* define if you have a PAM implementation with the pam_misc library */
#undef HAVE_PAM_MISC

/* define if you have shadow library */
#undef HAVE_SHADOW_LIB

/* define, where to find the X server */
#undef XBINDIR

/* define, where to find the XDM configurations */
#undef XDMDIR

/* Define if you have getdomainname */
#undef HAVE_GETDOMAINNAME  

/* Define if you have gethostname */
#undef HAVE_GETHOSTNAME  

/* Define if you have dlopen and co. */
#undef HAVE_DLFCN

/* Define if you have shl_load and co. */
#undef HAVE_SHLOAD

/* Define if you have usleep */
#undef HAVE_USLEEP

/* Define if you have random */
#undef HAVE_RANDOM

/* Define if you have S_ISSOCK */
#undef HAVE_S_ISSOCK

/* Define the file for utmp entries */
#undef UTMP

/* Define, if you want to use utmp entries */
#undef UTMP_SUPPORT

#undef HAVE_HP_STL

#undef HAVE_SGI_STL

/* Define, if you have setupterm in -l(n)curses */
#undef HAVE_SETUPTERM

/* Define, to enable volume management (Solaris 2.x), if you have -lvolmgt */
#undef HAVE_VOLMGT

#undef KDEMAXPATHLEN


#ifndef HAVE_BOOL
#define HAVE_BOOL
typedef int bool;
#ifdef __cplusplus
const bool false = 0;
const bool true = 1;
#else
#define false (bool)0;
#define true (bool)1;
#endif
#endif

/* this is needed for Solaris and others */
#ifndef HAVE_USLEEP
#ifdef __cplusplus
extern "C"
#endif
void usleep(unsigned int usec);
#endif  

#ifndef HAVE_GETDOMAINNAME
#define HAVE_GETDOMAINNAME
#ifdef __cplusplus  
extern "C" 
#endif
int getdomainname (char *Name, int Namelen);
#endif  

#ifndef HAVE_GETHOSTNAME
#define HAVE_GETHOSTNAME
#ifdef __cplusplus  
extern "C" 
#endif
int gethostname (char *Name, int Namelen);
#endif  

#ifndef HAVE_FUNC_SETENV
#define HAVE_FUNC_SETENV
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
#endif

/*
 * jpeg.h needs HAVE_BOOLEAN, when the system uses boolean in system
 * headers and I'm too lazy to write a configure test as long as only
 * unixware is related
 */
#ifdef _UNIXWARE
#define HAVE_BOOLEAN
#endif

#if !defined(HAVE_SETEUID)
#define seteuid(_eu) setresuid(-1, _eu, -1)
#endif

#ifndef HAVE_RANDOM
#define HAVE_RANDOM
long int random(void); // defined in fakes.cpp
void srandom(unsigned int seed);
#endif

#ifndef HAVE_S_ISSOCK
#define HAVE_S_ISSOCK
#define S_ISSOCK(mode) (1==0)
#endif

