#include <stdio.h>	//printf
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write, close

#include <sys/select.h>	//select

#define STDIN 0
#define STDOUT 1

int readdata(int fd, char *data, size_t datalen)
{
	int retval = read(fd, data, datalen);
	if (retval < 0)
	{
		if (fd == STDIN)
		{
			perror("read from stdin");
		}
		else
		{
			perror("read from socket");
		}
	}
	return retval;
}

int writedata(int fd, char *data, size_t datalen)
{
	int retval = write(fd, data, datalen);
	if (retval < 0)
	{
		if (fd == STDOUT)
		{
			perror("write to stdout");
		}
		else
		{
			perror("write to socket");
		}
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
	int sock;
	struct sockaddr_in server;
	char message[65535], server_reply[65535];

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		fputs("Could not create socket\n", stderr);
		return 1;
	}

	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Connection failed");
		return 1;
	}

	fputs("Connected\n", stderr);

	fd_set readfds, writefds;
	int len_data_for_tcp = 0, len_data_for_stdout = 0;
	int terminating = 0;

	//keep communicating with server
	while (!terminating || en_data_for_stdout || len_data_for_tcp)
	{
		if (-1 == selectdata(sock, &readfds, &writefds, len_data_for_tcp, len_data_for_stdout))
		{
			return 1;
		}

		if (FD_ISSET(STDIN, &readfds))
		{
			len_data_for_tcp = readdata(STDIN, message, sizeof(message));
			if (len_data_for_tcp == 0)
			{
				terminating = 1;
			}
		}

		if (FD_ISSET(sock, &readfds))
		{
			len_data_for_stdout = readdata(sock, server_reply, sizeof(server_reply));
			if (len_data_for_stdout == 0)
			{
				terminating = 1;
			}
		}

		if (FD_ISSET(STDOUT, &writefds))
		{
			writedata(STDOUT, server_reply, len_data_for_stdout);
			len_data_for_stdout = 0;
		}

		if (FD_ISSET(sock, &writefds))
		{
			writedata(sock, message, len_data_for_tcp);
			len_data_for_tcp = 0;
		}
	}

	close(sock);
	return 0;
}
