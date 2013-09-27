#include "common.h"
#include "netw.h"

#ifndef ___AC_SSLH
#define ___AC_SSLH

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct {
    int socket;
    SSL *sslHandle;
    SSL_CTX *sslContext;
} secconnection;

SSL_CTX *initialize_ctx(char *keyfile, char *password);

#endif
