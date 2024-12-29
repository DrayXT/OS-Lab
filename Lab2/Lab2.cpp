#include <vector>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

volatile sig_atomic_t wasSIGHUP = 0;

void sigHupHandler(int r)
{
	wasSIGHUP = 1;
}

int main()
{
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa);
	sa.sa_handler = sigHupHandler;
	sa.sa_flags |= SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);

	sigset_t blockedMask, origMask;
	sigemptyset(&blockedMask);
	sigaddset(&blockedMask, SIGHUP);
	sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(32204);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	if (listen(serverSocket, SOMAXCONN) < 0)
	{
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	printf("Server is running on port 32204 \n");

	std::vector<int> clients;
	int maxFd = serverSocket;
	fd_set fds;

	while (true)
	{
		FD_ZERO(&fds);
		FD_SET(serverSocket, &fds);
		maxFd = serverSocket;

		for (int clientSocket : clients)
		{
			FD_SET(clientSocket, &fds);
			if (clientSocket > maxFd) maxFd = clientSocket;
		}

		if (pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask) == -1)
		{
			if (errno == EINTR)
			{
				if (wasSIGHUP)
				{
					printf("SIGHUP signal received \n");
					wasSIGHUP = 0;
					printf("SIGHUP signal processed \n");
				}
				continue;
			}
			else
			{
				break;
			}
		}

		if (FD_ISSET(serverSocket, &fds))
		{
			int clientSocket = accept(serverSocket, NULL, NULL);
			if (clientSocket != -1)
			{
				printf("New connection: %d\n", clientSocket);
				for (int oldSocket : clients)
				{
					printf("Only one connection is possible, connection %d\n is closing", oldSocket);
					close(oldSocket);
				}
				clients.clear();
				clients.push_back(clientSocket);
			}
		}

		for (auto clientIt = clients.begin(); clientIt != clients.end();)
		{
			int clientSocket = *clientIt;
			if (FD_ISSET(clientSocket, &fds))
			{
				char buffer[1024];
				ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
				if (bytesRead <= 0)
				{
					printf("Connection %d closed \n", clientSocket);
					close(clientSocket);
					clientIt = clients.erase(clientIt);
				}
				else
				{
					printf("%zd bytes received from %d\n", bytesRead, clientSocket);
					++clientIt;
				}
			}
			else
			{
				++clientIt;
			}
		}

	}

	close(serverSocket);
}