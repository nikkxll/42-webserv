#include "Server.hpp"

Server::Server()
{
	initialize();
}

Server::Server(std::string ipAddress, int port)
{
	if (inet_pton(this->domain, ipAddress.c_str(), &(this->address)) != 1)
		throw ServerException("Conversion failed");
	// this->address = inet_addr(ipAddress.c_str());
	this->port = port;
	initialize();
}

Server::~Server()
{
	shutdown();
	close(clientSocket);
}

void Server::initialize()
{
	createSocket();

	this->sockAddress.sin_family = this->domain;
	this->sockAddress.sin_addr.s_addr = this->address;
	this->sockAddress.sin_port = htons(this->port);

	bindSocket();
	listenConnection();
	// while (1)
	// {
	// 	acceptConnection();
	// }
}

void Server::createSocket()
{
	this->serverSocket = socket(this->domain, this->type, this->protocol);
	if (this->serverSocket == 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed");
	}

	// Set socket to non-blocking mode
	int flags = fcntl(this->serverSocket, F_GETFL, 0);
	if (flags == -1)
	{
		close(this->serverSocket);
		throw ServerException("Failed to get socket flags");
	}
	if (fcntl(this->serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		close(this->serverSocket);
		throw ServerException("Failed to set non-blocking mode on socket");
	}
}

void Server::bindSocket()
{
	std::memset(this->sockAddress.sin_zero, '\0', sizeof(this->sockAddress.sin_zero));

	if (bind(this->serverSocket, (struct sockaddr *)&this->sockAddress, sizeof(this->sockAddress)) < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed binding");
	}
}

void Server::listenConnection()
{
	if (listen(this->serverSocket, 10) < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed listening");
	}
}

/* void Server::acceptConnection()
{
	std::cout << "Server listening on port " << this->port << std::endl;
	// char hello[] = "Hello from server";
	char hello[] = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n\r\n<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>Document</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	int addrlen = sizeof(this->sockAddress);
	this->clientSocket = accept(this->serverSocket, (struct sockaddr *)&this->sockAddress, (socklen_t *)&addrlen);
	if (this->clientSocket < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed accepting");
	}

	char buffer[30000] = {0};
	int valread = read(this->clientSocket, buffer, 30000);
	std::cout << valread << std::endl;
	std::cout << buffer << std::endl;
	write(clientSocket, (char *)hello, std::strlen(hello));
	std::cout << "Hello message sent" << std::endl;
	close(this->clientSocket);
	std::cout << "Connection closed" << std::endl;
} */

void Server::handleRequest()
{

	/* Dummy response start */
	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
	// std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world " + std::to_string(time(NULL)) + "\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
	// std::cout << contentLength << std::endl;
	response = response + contentLength + "\r\n" + body;
	/* Dummy response end */


	int bufferSize = 1024;
	char buffer[bufferSize];
	int bytesRead;
	std::string request;
	buffer[bufferSize - 1] = '\0';
	int count = 0;

	while (request.empty())
	{
		if (count == 0)
			std::cout << "=== Waiting for request ===" << std::endl;
		count++;

		// while ((bytesRead = read(this->clientSocket, buffer, 1024) > 0))
		while (1)
		{
			bytesRead = read(this->clientSocket, buffer, bufferSize);
			if (bytesRead > 0)
			{
				std::cout << "=== Input read! ===" << std::endl;
				std::cout << "bytesRead: " << bytesRead << std::endl;
			}
			if (bytesRead <= 0)
				// std::cout << "bytesRead: " << bytesRead << std::endl;
				break;
			request += std::string(buffer);
		}
	}
	std::cout << "Last bytesRead: " << bytesRead << std::endl;
	std::cout << "=== Request read in chunks ===" << std::endl;
	std::cout << TEXT_YELLOW << request << RESET << std::endl;

	Request req(request);

	// Testing request
	std::cout << TEXT_CYAN;
	std::cout << req.getStartLine()["method"] << std::endl;
	std::cout << req.getStartLine()["target"] << std::endl;
	std::cout << req.getStartLine()["version"] << std::endl;
	std::cout << RESET;

	// std::cout << "bytesRead" << bytesRead << std::endl; // message length
	std::cout << "=== Buffer ===" << std::endl;
	std::cout << buffer << std::endl;


/* 	
	// Sending dummy response
	write(clientSocket, response.c_str(), response.length());
	std::cout << "Response message sent: " <<  response.c_str() << std::endl;
 */

	// int fd = open(req.getStartLine()["target"].c_str(), O_RDONLY);
/* 	int fd = open("test.html", O_RDONLY);
	if (fd < 0)
	{
		std::cerr << "Error: " << strerror(errno) << std::endl;
		throw ServerException("Error: " + std::string(strerror(errno)));
	}
	else
	{
		std::cout << "File opened" << std::endl;

		response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
		std::string body = "";

	} */

	// response = "";
	// while (1)
	// {
	// 	bytesRead = read(fd, buffer, bufferSize);
	// 	if (bytesRead > 0)
	// 	{
	// 		std::cout << "=== File read! ===" << std::endl;
	// 		std::cout << "bytesRead: " << bytesRead << std::endl;
	// 	}
	// 	if (bytesRead <= 0)
	// 		// std::cout << "bytesRead: " << bytesRead << std::endl;
	// 		break;
	// 	response += std::string(buffer);
	// }
	std::cout << TEXT_GREEN;
	std::cout << "=== Response message sent ===" << std::endl;
	std::cout << response.c_str() << std::endl;
	std::cout << RESET;
	write(clientSocket, response.c_str(), response.length());

	close(this->clientSocket);
	std::cout << "Connection closed" << std::endl;
}

void Server::shutdown()
{
	close(this->serverSocket);
}

int Server::getSocket() const
{
	return this->serverSocket;
}

struct sockaddr_in &Server::getSockAddress()
{
	return this->sockAddress;
}

int Server::getClientSocket() const
{
	return this->clientSocket;
}

void Server::setClientSocket(int newClientSocket)
{
	this->clientSocket = newClientSocket;
}
