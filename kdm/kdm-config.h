#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* xdm stuff, should always defined */
#define UNIXCONN          
#define TCPCONN           
#define GREET_USER_STATIC 

#ifdef XDMBINDIR
# define BINDIR XDMBINDIR
#endif

#define DEF_XDM_CONFIG XDMDIR##"/xdm-config"
#define DEF_AUTH_DIR XDMDIR##"/authdir"

/* Authorization stuff */
/* How do we check for HASXDMAUTH? Use Imake ?? */
#if defined(HAVE_KRB5_KRB5_H)
#endif
#if defined(HAVE_RPC_RPC_H) && defined(HAVE_RPC_KEY_PROT_H)
# define SECURE_RPC 1
#endif

/* Too many systems have trouble with secure rpc,
   so its disabled for now:
*/
#undef SECURE_RPC

#ifdef HAVE_PAM
# define USE_PAM 1
#else
# ifdef HAVE_SHADOW
#  define USESHADOW 1
# endif
#endif

#ifdef sun
#define SVR4 1
#endif

#ifdef __cplusplus
extern "C" {
#endif
int XdmcpAllocARRAY8();
int Debug( char*, ...);
int LogError( char*, ...);
int LogOutOfMem( char*, ...);
#ifdef __cplusplus
}
#endif
