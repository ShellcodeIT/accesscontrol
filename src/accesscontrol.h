#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct {
    int socket;
    SSL *sslHandle;
    SSL_CTX *sslContext;
} secconnection;

struct requestStr {
	char *get;
	char *host;
	char *body;
};

typedef struct requestStr requestType;


