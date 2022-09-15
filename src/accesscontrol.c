#include "accesscontrol.h"

#define PORT (5001)
#define MAINPROCESS ":: Access Control ::\t"
#define CHILDPROCESS "\t :: Child ::\t"
#define URLSIZE (4096)
#define REQUESTSIZE (4096)
#define RESPONSESIZE (4096)
#define PIDFILE "/var/run/accesscontrol.pid"
#define LOGFILE "/var/log/accesscontrol.log"
#define LOGLINESIZE (1024)
#define MARK "-_.!~*'()\"/"

int daemonizeflag = 0;
int logflag = 0;
int debugflag = 0;
FILE *logfile;

void catch_child(int sig_num)
{
	int child_status;
	wait(&child_status);
}

void finish_main(int sig_num)
{
	printf("Finishing main and childs...\n");
	int child_status;
	wait(&child_status);
	exit(0);
}

char toHexChar(int digitValue)
{
	if (digitValue < 10)
		return (char)('0' + digitValue);
	else
		return (char)('A' + (digitValue - 10));
}
int markIdx(char c)
{
	int len = strlen(MARK);
	int i;
	for (i = 0; i < len; i++)
	{
		if (MARK[i] == c)
			return i;
	}
	return -1;
}

char *encodeURL(char *url)
{
	char *encodedUrl = (char *)malloc((REQUESTSIZE + 1) * sizeof(char));
	unsigned int len = strlen(url);
	unsigned int i, rlen = 0;
	char c;
	bzero(encodedUrl, (REQUESTSIZE + 1));
	for (i = 0; i < len; i++)
	{
		c = url[i];
		if ((c >= '0' && c <= '9') ||
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			markIdx(c) >= 0)
		{
			encodedUrl[rlen] = c;
			rlen++;
		}
		else
		{
			encodedUrl[rlen] = '%';
			rlen++;
			encodedUrl[rlen] = toHexChar((c & 0xF0) >> 4);
			rlen++;
			encodedUrl[rlen] = toHexChar((c & 0x0F));
			rlen++;
		}
	}
	return encodedUrl;
}

char *readline(char *s, int *pos)
{
	int len = strlen(s);
	int i = *pos;
	if (*pos >= len)
		return NULL;
	int e = 0;
	char *ret;
	while (1)
	{
		e = *pos;
		if (*pos >= len)
			break;
		if (*pos < len && s[*pos] == '\r' && s[*pos + 1] == '\n')
		{
			*pos += 2;
			break;
		}
		else if (s[*pos] == '\n')
		{
			*pos += 1;
			break;
		}
		*pos += 1;
	};
	ret = (char *)malloc(((e - i) + 1) * sizeof(char));
	bzero(ret, ((e - i) + 1));
	strncpy(ret, s + i, e - i);
	return ret;
}

void msg(const char *format, ...)
{
	int done;
	va_list arg;
	char *fmt;
	if (logflag || debugflag)
	{
		fmt = (char *)malloc(LOGLINESIZE * sizeof(char));
		bzero(fmt, LOGLINESIZE);
		strncat(fmt, format, (LOGLINESIZE - 1));
		strcat(fmt, "\n");
		va_start(arg, format);
		if (logflag)
		{
			done = vfprintf(logfile, fmt, arg);
		}
		if (debugflag)
		{
			done = vfprintf(stderr, fmt, arg);
		}
		va_end(arg);
	}
}

int main(int argc, char **argv)
{
	int sockfd, newsockfd, portno, clilen;
	int child_pid = -1;
	char *request = (char *)malloc(REQUESTSIZE * sizeof(char));
	char *response = (char *)malloc(RESPONSESIZE * sizeof(char));
	char *page = (char *)malloc(URLSIZE * sizeof(char));
	struct sockaddr_in serv_addr, cli_addr;
	struct sockaddr_in *pV4Addr;
	int ipAddr;
	char *buffer, *get, *host, *parsebuffer;
	char *clientip;
	char *token;
	FILE *pidfile = fopen(PIDFILE, "ab");
	int i, n;
	int pos = 0;

	/**
	 * SSL
	 */
	SSL *ssl;
	SSL_CTX *ctx = initialize_ctx();
	// THEPASSWD
	configure_context(ctx, "../cert.pem", "../key.pem", "THEPASSWD");

	/* initialize parameters */
	for (i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-d") == 0)
		{
			daemonizeflag = 1;
		}
		else if (strcmp(argv[i], "-l") == 0)
		{
			logflag = 1;
			logfile = fopen(LOGFILE, "ab+");
		}
		else if (strcmp(argv[i], "-v") == 0)
		{
			debugflag = 1;
		}
	}
	signal(SIGCHLD, catch_child);
	signal(SIGUSR1, finish_main);

	if (!daemonizeflag)
	{
		debugflag = 1;
	}
	if (daemonizeflag)
	{
		msg("%sDaemonizing...", MAINPROCESS);
		int daemon_pid = fork();
		if (daemon_pid)
		{
			exit(EXIT_SUCCESS);
		}
	}
	msg("%sInitializing... PID: %d", MAINPROCESS, getpid());

	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR opening socket");
		exit(1);
	}
	/* Initialize socket structure */
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	// lose the pesky "Address already in use" error message
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		exit(1);
	}

	int mainPID = getpid();
	/* Now start listening for the clients, here process will
	 * go in sleep mode and will wait for the incoming connection
	 */
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	while (1)
	{
		msg("%sWaiting connection from new client on port %d.", MAINPROCESS, PORT);
		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
						   &clilen);
		pV4Addr = (struct sockaddr_in *)&cli_addr;
		ipAddr = pV4Addr->sin_addr.s_addr;
		clientip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
		inet_ntop(AF_INET, &ipAddr, clientip, INET_ADDRSTRLEN);
		msg("%sConection received from %s -- fork()", MAINPROCESS, clientip);

		/**
		 * SSL
		 */
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, newsockfd);
		if (SSL_accept(ssl) <= 0)
		{
			ERR_print_errors_fp(stderr);
		}

		child_pid = fork();

		/* child process */
		if (!child_pid)
		{

			/* check accept socket */
			if (newsockfd < 0)
			{
				perror("ERROR on accept");
				exit(1);
			}

			/* closing listener socket */
			if (close(sockfd) < 0)
			{
				perror("ERROR closing listener socket");
			}

			/* child process pid */
			msg("%s%d", CHILDPROCESS, getpid());

			// /* read request */
			// bzero(request, REQUESTSIZE);
			// n = read(newsockfd, request, REQUESTSIZE);
			// if (n < 0)
			// {
			// 	perror("ERROR reading from socket");
			// 	exit(1);
			// }
			/* child process pid */
			// msg("READED DATA!");
			const char reply[] = "Access Control V1.0\n";

			if (SSL_accept(ssl) <= 0)
			{
				ERR_print_errors_fp(stderr);
			}
			else
			{
				SSL_write(ssl, reply, strlen(reply));
			}
			// char buf[4096];

			int stop_loop = 0;
			// first read data
			int r;

			while (stop_loop == 0)
			{

				r = SSL_read(ssl, request, REQUESTSIZE);

				switch (SSL_get_error(ssl, r))
				{
				case SSL_ERROR_NONE:
					if (request[r - 2] == '\r')
					{
						request[r - 2] = '\0';
					}
					else if (request[r - 1] == '\n')
					{
						request[r - 1] = '\0';
					}
					break;
				case SSL_ERROR_ZERO_RETURN:
					printf("SSL zero return");
					stop_loop = 1;
				default:
					printf("SSL read problem");
					stop_loop = 1;
				}

				if (stop_loop == 0)
				{
					msg("%s%d > %s", CHILDPROCESS, getpid(), request);
					if (strcmp(request, "CLOSE") == 0 || strcmp(request, "FINISH") == 0)
					{

						SSL_shutdown(ssl);
						SSL_free(ssl);
						close(newsockfd);
						if (strcmp(request, "FINISH") == 0)
							kill(0, SIGUSR1);
						exit(EXIT_SUCCESS);
					}
				}
			}

			msg("%s%d > ENDING CHILD PROCESS", CHILDPROCESS, getpid(), request);
			SSL_shutdown(ssl);
			SSL_free(ssl);
			close(newsockfd);
			exit(EXIT_SUCCESS);
		}
		else
		{
			close(newsockfd);
		}
	}
	// close server port
	close(sockfd);
	SSL_CTX_free(ctx);
	return EXIT_FAILURE;
}
