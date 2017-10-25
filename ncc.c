#include <stdio.h>	//printf
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write, close
#include <stdlib.h>	//atoi

#include <sys/select.h>	//select

#define STDIN 0
#define STDOUT 1

int readdata(int fd, char *data, size_t datalen)
{
	int retval = read(fd, data, datalen);
	if (retval < 0)
	{
		perror(fd == STDIN ? "read() from stdin" : "");
	}
	return retval;
}

int writedata(int fd, char *data, size_t datalen)
{
	int retval = write(fd, data, datalen);
	if (retval < 0)
	{
		perror(fd == STDOUT ? "write() to stdout" : "");
	}
	return retval;
}

int selectdata(int sock, fd_set *readfds, fd_set *writefds, int has_data_for_tcp, int has_data_for_stdout)
{
	FD_ZERO(readfds);
	if (!has_data_for_tcp) FD_SET(STDIN, readfds);
	if (!has_data_for_stdout) FD_SET(sock, readfds);
	FD_ZERO(writefds);
	if (has_data_for_tcp) FD_SET(sock, writefds);
	if (has_data_for_stdout) FD_SET(STDOUT, writefds);

	int retval = select(sock+1, readfds, writefds, NULL, NULL);
	if (retval < 0)
	{
		perror("select()");
	}
	return retval;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		return 2;
	}

	int sock;
	struct sockaddr_in server;
	char message[65535], server_reply[65535];

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1)
	{
		fputs("Could not create socket\n", stderr);
		return 1;
	}

	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));

	//Connect to remote server
	fprintf(stderr, "Trying %s...", argv[1]);
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("\nConnection failed");
		return 1;
	}

	fprintf(stderr, " Connected.\n");

	fd_set readfds, writefds;
	int len_data_for_tcp = 0, len_data_for_stdout = 0;
	int terminating = 0;

	//keep communicating with server
	while (terminating != 3 || len_data_for_stdout || len_data_for_tcp)
	{
		if (-1 == selectdata(sock, &readfds, &writefds, len_data_for_tcp, len_data_for_stdout))
		{
			return 1;
		}

		if (FD_ISSET(STDIN, &readfds))
		{
			//fprintf(stderr, "<");
			len_data_for_tcp = readdata(STDIN, message, sizeof(message));
			if (len_data_for_tcp == 0)
			{
				terminating |= 1;
				shutdown(sock, SHUT_WR);
			}
		}

		if (FD_ISSET(sock, &readfds))
		{
			//fprintf(stderr, "v");
			len_data_for_stdout = readdata(sock, server_reply, sizeof(server_reply));
			// Socket didn't give us any data; assume FIN,ACK
			if (len_data_for_stdout == 0)
			{
				terminating |= 2;
				shutdown(sock, SHUT_RD);
				fprintf(stderr, "\nConnection closed\n");
			}
		}

		if (FD_ISSET(STDOUT, &writefds))
		{
			//fprintf(stderr, ">");
			writedata(STDOUT, server_reply, len_data_for_stdout);
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
