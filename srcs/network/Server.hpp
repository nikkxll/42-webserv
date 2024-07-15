/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/07/15 21:52:01 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Socket.hpp"
#include "CGIHandler.hpp"
#include "SessionsManager.hpp"
#include "../response/Response.hpp"
#include "../utils/Utility.hpp"
#include "../utils/logUtils.hpp"
#include "../request/Request.hpp"
#include "../utils/ServerException.hpp"
#include "DirLister.hpp"
#include "client.hpp"
#include <vector>
#include <string>
#include <cstring> // memset()
#include <signal.h> // signal()
#include <poll.h> // poll()
#include <unistd.h> // read(), write(), close()

#include <limits> // for max size_t

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <fstream> //open file

#define FDS 2

struct Pipe {
	int input[FDS];
	int output[FDS];
};

class Server
{
	private:
		Socket						_serverSocket;
		struct addrinfo				_hints;
		struct addrinfo*			_res;
		std::vector<t_client>		_clients;
		std::vector<ServerConfig>	_configs;
		Pipe						_CGIpipes;

		int							_port;
		std::string					_ipAddr;

	public:
		Server();
		Server(const char* ipAddr, int port);
		~Server();

		void						setConfig(std::vector<ServerConfig> serverConfigs);
		int							getServerSockfd();
		Pipe&						getPipe();
		std::vector<t_client>&		getClients();
		std::string					getIpAddress();
		int							getPort();
		std::vector<ServerConfig>	getConfigs();

		int							accepter();
		void						handler(Server*& server, t_client& client);
		void						responder(t_client& client, Server &server);

		// Request*					receiveRequest(int clientSockfd);
		bool						receiveRequest(t_client& client);
		bool						sendResponse(t_client& client);
		void						finalizeResponse(t_client& client);

	private:
		std::string					whoAmI() const;
		void						initServer(const char* ipAddr, int port);
		void						removeFromClients(t_client& client);


		void						validateRequest(t_client& client);
		// bool						formRequestErrorResponse(t_client& client);
		bool						formCGIConfigAbsenceResponse(t_client& client, Server &server);
		void						handleCGIResponse(t_client& client, Server &server);
		void						handleUpload(t_client& client, Location& foundLocation);
		void						handleNonCGIResponse(t_client& client, Server &server);
		void						checkIfMethodAllowed(t_client& client, Location& foundLocation);
		void						handleRedirect(t_client& client, Location& foundLocation);
		void						handleStaticFiles(t_client& client, Location& foundLocation);
		Location					findLocation(Request* req);
		
		ServerConfig*				findServerConfig(Request* req);
		size_t						findMaxClientBodyBytes(Request request);

		Response*					createResponse(Request* request, int code, std::map<std::string,
										std::string> optionalHeaders = {});
};
