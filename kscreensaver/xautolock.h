
typedef enum { IGNORE, FORCE_SAVE, FORCE_LOCK } CornerAction;

void setCorners( const char * );
void forceTimeout();
void initAutoLock();
void cleanupAutoLock();
int  waitTimeout( int );

