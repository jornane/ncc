/*
	C ECHO client example using sockets
*/
#include<stdio.h>	//printf
#include<string.h>	//strlen
#include<sys/socket.h>	//socket
#include<arpa/inet.h>	//inet_addr
#include <unistd.h>	//write, close

#include <sys/select.h>	//select

int main(int argc ,char *argv[])
{
	int sock;
	struct sockaddr_in server;
	char message[1000], server_reply[2000];

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

	fd_set readfds;
	int retval;
	int recvval;

	//keep communicating with server
	for (;;)
	{
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		FD_SET(sock, &readfds);

		retval = select(sock+1, &readfds, NULL, NULL, NULL);
		if (retval == -1) {
			perror("select()");
			return 1;
		}

		if (FD_ISSET(0, &readfds)) {
			//write(1, "reading\n", 8);
			read(0, message, sizeof(message));
			//Send some data
			if (send(sock, message, strlen(message) , 0) < 0)
			{
				fputs("Send failed\n", stderr);
				return 1;
			}
		}

		//Receive a reply from the server
		if (FD_ISSET(sock, &readfds)) {
			recvval = recv(sock, server_reply, sizeof(server_reply), 0);
			if (recvval < 0)
			{
				fputs("recv failed\n", stderr);
				break;
			}
			if (recvval == 0) {
				break;
			}
			write(1, server_reply, strnlen(server_reply, sizeof(server_reply)));
		}
	}

	close(sock);
	return 0;
}
