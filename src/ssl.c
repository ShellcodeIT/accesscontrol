#include "ssl.h"

SSL_CTX *initialize_ctx(char *keyfile, char *password) {
	SSL_CTX *ctx;

	/* Create our context*/
	ctx=SSL_CTX_new(SSLv23_method());

	return ctx;
}
