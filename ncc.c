/*
	C ECHO client example using sockets
*/
#include<stdio.h>	//printf
#include<sys/socket.h>	//socket
#include<arpa/inet.h>	//inet_addr
#include <unistd.h>	//write, close

#include <sys/select.h>	//select

int main(int argc ,char *argv[])
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
	fputs("Socket created\n", stderr);

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

	//keep communicating with server
	for (;;)
	{
		FD_ZERO(&readfds);
		if (!len_data_for_tcp) FD_SET(0, &readfds);
		if (!len_data_for_stdout) FD_SET(sock, &readfds);
		FD_ZERO(&writefds);
		if (len_data_for_tcp) FD_SET(sock, &writefds);
		if (len_data_for_stdout) FD_SET(1, &writefds);

		if (-1 == select(sock+1, &readfds, &writefds, NULL, NULL)) {
			perror("select()");
			return 1;
		}

		if (FD_ISSET(0, &readfds)) {
			//write(1, "reading\n", 8);
			len_data_for_tcp = read(0, message, sizeof(message));
			if (len_data_for_tcp == 0) {
				break;
			}
		}

		//Receive a reply from the server
		else if (FD_ISSET(sock, &readfds)) {
			len_data_for_stdout = recv(sock, server_reply, sizeof(server_reply), 0);
			if (len_data_for_stdout < 0)
			{
				fputs("recv failed\n", stderr);
				break;
			}
			if (len_data_for_stdout == 0) {
				break;
			}
		}

		else if (FD_ISSET(1, &writefds)) {
			write(1, server_reply, len_data_for_stdout);
			len_data_for_stdout = 0;
		}

		else if (FD_ISSET(sock, &writefds)) {
			if (send(sock, message, len_data_for_tcp, 0) < 0)
			{
				fputs("Send failed\n", stderr);
				return 1;
			}
			len_data_for_tcp = 0;
		}
	}

	close(sock);
	return 0;
}
