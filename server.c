#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>


void error_handing(char* message);

void do_list(int sock);
void do_post(int sock);
void do_get(int sock);

int main(int argc, char const *argv[])
{
	int serv_sock, conn_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_sz;
	char ch;

	if (argc != 2) {
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1) {
		error_handing("socker() error");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
		error_handing("bind() error");
	}

	if (listen(serv_sock, 5) == -1) {
		error_handing("listen() error");
	}

	while (1) {
		clnt_addr_sz = sizeof(clnt_addr);
		conn_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);
		if (conn_sock == -1) {
			error_handing("accept() error");
		}
		read(conn_sock, &ch, 1);
		printf("%c\n", ch);
		switch (ch) {
			case 'L':
				do_list(conn_sock);
				break;
			case 'P':
				do_post(conn_sock);
				break;
			case 'G':
				do_get(conn_sock);
				break;
			default:
				break;
		}

		close(conn_sock);
		printf("close() success\n");
	}


	close(serv_sock);
	
	return 0;
}

void do_list(int sock)
{
	char msg[80] = "";
	DIR *dir;

  	struct dirent *ptr;

  	dir = opendir(".");

 	while((ptr = readdir(dir)) != NULL) {
 		strcat(msg, ptr->d_name);
 		strcat(msg, "\n");
 	}
 	write(sock, msg, strlen(msg));
		

	closedir(dir);
}

void do_post(int sock)
{
	char name[30];
	int fd;
	int sz;
	char buf[512];
	int n;

	read(sock, name, 30);
	printf("%s\n", name);

	fd = open(name, O_CREAT | O_WRONLY, 0644);

	while ((sz = recv(sock, buf, 512, 0)) > 0) {
		n = write(fd, buf, sz);
		printf("%d\n", sz);
	}
	close(fd);
}

void do_get(int sock)
{
	char name[30];
	char buf[512];
	int fd;
	int sz;
	int n;
	int ch;

	read(sock, name, 30);
	printf("%s\n", name);

	if (access(name, F_OK) == 0) {
		ch = 'Y';
		write(sock, &ch, 1);
	} else {
		ch = 'N';
		write(sock, &ch, 1);
		return;
	}

	fd = open(name, O_RDONLY);
	while ((sz = read(fd, buf, 512)) > 0) {
		n = send(sock, buf, sz, 0);
		printf("%d\n", sz);
	}
	close(fd);
}

void error_handing(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
