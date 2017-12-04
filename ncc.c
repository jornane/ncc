#include <stdio.h>	//printf
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write, close
#include <stdlib.h>	//atoi

#include <sys/select.h>	//select

#include <netdb.h>	// sockaddr_in (OpenBSD)
#include <netinet/in.h>	// sockaddr_in (FreeBSD)

#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>

#define SOCK_LOCAL 1
#define SOCK_REMOTE 2

int readdata(int fd, char *data, size_t datalen)
{
	int retval = read(fd, data, datalen);
	if (retval < 0)
	{
		perror(fd == STDIN_FILENO ? "read() from stdin" : "");
	}
	return retval;
}

int writedata(int fd, char *data, size_t datalen)
{
	int retval = write(fd, data, datalen);
	if (retval < 0)
	{
		perror(fd == STDOUT_FILENO ? "write() to stdout" : "");
	}
	return retval;
}

int wait_for_data(int sock, fd_set *readfds, fd_set *writefds, int has_data_for_tcp, int has_data_for_stdout, int dead_sock_map)
{
	FD_ZERO(readfds);
	if (!has_data_for_tcp && !(dead_sock_map & SOCK_LOCAL)) FD_SET(STDIN_FILENO, readfds);
	if (!has_data_for_stdout && !(dead_sock_map & SOCK_REMOTE)) FD_SET(sock, readfds);
	FD_ZERO(writefds);
	if (has_data_for_tcp) FD_SET(sock, writefds);
	if (has_data_for_stdout) FD_SET(STDOUT_FILENO, writefds);

	int retval = select(sock+1, readfds, writefds, NULL, NULL);
	if (retval < 0)
	{
		perror("select()");
	}
	return retval;
}

int get_socket(char *host, char *service)
{
	int sfd, s;
	struct addrinfo *result, *rp;
	struct addrinfo hints;
	char addr_str[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	s = getaddrinfo(host, service, &hints, &result);
	if (s != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
		{
			perror("Unable to create socket");
			continue;
		}

		if (rp->ai_family == AF_INET)
		{
			if (!inet_ntop(AF_INET, &(((struct sockaddr_in *) rp->ai_addr)->sin_addr), addr_str, INET_ADDRSTRLEN))
			{
				perror("Unable to read address");
				continue;
			}
			fprintf(stderr, "Trying %s:%s...", addr_str, service);
		}
		else if (rp->ai_family == AF_INET6)
		{
			if (!inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) rp->ai_addr)->sin6_addr), addr_str, INET6_ADDRSTRLEN))
			{
				perror("Unable to read address");
				continue;
			}
			fprintf(stderr, "Trying [%s]:%s...", addr_str, service);
		}
		else
		{
			fprintf(stderr, "Unknown address family\n");
			continue;
		}

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
		{
			fprintf(stderr, " Connected.\n");
			freeaddrinfo(result);
			return sfd;
		}
		else
		{
			fprintf(stderr, " %s\n", strerror(errno));
		}

		close(sfd);
	}

	return -1;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		return 2;
	}

	int sock;
	char message[65535], server_reply[65535];

	//Create socket
	sock = get_socket(argv[1], argv[2]);
	if (sock == -1)
	{
		fputs("Could not create socket\n", stderr);
		return 1;
	}

	fd_set readfds, writefds;
	int len_data_for_tcp = 0, len_data_for_stdout = 0;
	int dead_sock_map = 0;

	while (!dead_sock_map || len_data_for_stdout || len_data_for_tcp)
	{
		if (-1 == wait_for_data(sock, &readfds, &writefds, len_data_for_tcp, len_data_for_stdout, dead_sock_map))
		{
			return 1;
		}

		if (FD_ISSET(STDIN_FILENO, &readfds))
		{
			//fprintf(stderr, "<");
			len_data_for_tcp = readdata(STDIN_FILENO, message, sizeof(message));
			if (len_data_for_tcp == 0)
			{
				shutdown(sock, SHUT_WR);
				dead_sock_map |= SOCK_LOCAL;
			}
		}

		if (FD_ISSET(sock, &readfds))
		{
			//fprintf(stderr, "v");
			len_data_for_stdout = readdata(sock, server_reply, sizeof(server_reply));
			// Socket didn't give us any data; assume FIN,ACK
			if (len_data_for_stdout == 0)
			{
				fprintf(stderr, "\nConnection closed\n");
				// TODO: too harsh, there may still be data in a pipe somewhere
				break;
				dead_sock_map |= SOCK_REMOTE;
			}
		}

		if (FD_ISSET(STDOUT_FILENO, &writefds))
		{
			//fprintf(stderr, ">");
			writedata(STDOUT_FILENO, server_reply, len_data_for_stdout);
			len_data_for_stdout = 0;
		}

		if (FD_ISSET(sock, &writefds))
		{
			//fprintf(stderr, "^");
			writedata(sock, message, len_data_for_tcp);
			len_data_for_tcp = 0;
		}
	}

	close(sock);
	return 0;
}
