#include <kurl.h>
#include "protocol.h"

int ProtocolSupported(const char *url);
KProtocol *CreateProtocol(const char *url);
const char *TopLevelURL(const char *url);
int HaveProto(const char *url);
char *ReformatURL(const char *url);
