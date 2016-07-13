#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>



void do_list(int sock);
void do_post(int sock, char *cmd);
void do_get(int sock, char *cmd);

void error_handing(char* message);

int main(int argc, char const *argv[])
{	
	int sock;
	struct sockaddr_in serv_addr;

	char cmd[30];

	if (argc != 3) {
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	}

	while (1) {
		printf(">>>>");
		fgets(cmd, 30, stdin);
		cmd[strlen(cmd) - 1] = 0;

		sock = socket(PF_INET, SOCK_STREAM, 0);
		if (sock == -1) {
			error_handing("socker() error");
		}

		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
		serv_addr.sin_port = htons(atoi(argv[2]));

		if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
			error_handing("connect error");
		}

		if (strcmp(cmd, "list") == 0) {
			do_list(sock);
		} else if (strncmp(cmd, "upload ", 7) == 0) {
			do_post(sock, cmd);
		} else if (strncmp(cmd, "download ", 9) == 0) {
			do_get(sock, cmd);
		}

		close(sock);
	}
	

	return 0;
}

void do_list(int sock)
{
	char ch = 'L';
	char msg[80];
	int sz;

	write(sock, &ch, 1);
	sz = read(sock, msg, 79);
	msg[sz] = 0;
	printf("%s\n", msg);
}

void do_post(int sock, char *cmd)
{
	char msg[30] = "P";
	int fd;
	char* name = cmd + 7;
	int sz;
	char buf[512];

	strcat(msg, name);
	write(sock, msg, 30);
	
	fd = open(name, O_RDONLY);
	while ((sz = read(fd, buf, 512)) > 0) {
		send(sock, buf, sz, 0);
	}
	close(fd);
	printf("Upload Success!\n");
}

void do_get(int sock, char* cmd)
{
	char msg[30] = "G";
	int fd;
	char *name = cmd + 9;
	int sz;
	char buf[512];
	int ch;

	strcat(msg, name);
	write(sock, msg, 30);

	read(sock, &ch, 1);
	printf("%d\n", ch);

	if (ch == 32590) {
		printf("file is not exist\n");
		return;
	}

	fd = open(name, O_CREAT | O_WRONLY, 0644);
	while ((sz = recv(sock, buf, 512, 0)) > 0) {
		write(fd, buf, sz);
	}
	close(fd);
	printf("Download Success!\n");
}

void error_handing(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}