#include "ssl.h"
#include <stdio.h>
#include <unistd.h>

static char *pass;
SSL_CTX *initialize_ctx()
{
	SSL_CTX *ctx;

	/* Create our context*/
	ctx = SSL_CTX_new(SSLv23_method());
	if (!ctx)
	{
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	return ctx;
}

void configure_context(SSL_CTX *ctx, char *cert_path, char *pkey_path, char *pkey_passwd)
{
	pass = pkey_passwd;
	/* Set the key and cert */
	if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	if (pass != NULL)
	{
		SSL_CTX_set_default_passwd_cb(ctx, password_cb);
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, pkey_path, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

static int password_cb(char *buf, int num,
					   int rwflag, void *userdata)
{
	if (num < strlen(pass) + 1)
		return (0);

	strcpy(buf, pass);
	return (strlen(pass));
}