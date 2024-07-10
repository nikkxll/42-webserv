/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 15:53:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/10 18:23:06 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../request/Request.hpp"
#include "Server.hpp"
#include "client.hpp"
#include "../response/Response.hpp"
#include "../utils/logUtils.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <memory>

#define IN 0
#define OUT 1

#define PYTHON_INTERPRETER "/usr/bin/python3"
#define PHP_INTERPRETER "/usr/bin/php"

class Server;

class CGIServer {
	private:
		static				std::string determineInterpreter(const std::string& filePath);
		static				std::vector<std::string> setEnvironmentVariables(Request* request);
		static void			handleProcesses(t_client& client, Server& server,
								const std::string& interpreter, const std::vector<std::string>& envVars);
		static void			handleChildProcess(Server& server, const std::string& interpreter,
								const std::string& filePath, const std::vector<std::string>& envVars);
		static void			handleParentProcess(Server& server, Response* response, const std::string& method,
								const std::string& body);
		static std::string	readErrorPage(const std::string& errorPagePath);
		static void			checkResponseHeaders(const std::string& result, Response* response);

	public:
		CGIServer()			= delete;
		static void			handleCGI(t_client& client, Server& server);
};