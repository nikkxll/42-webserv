/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 13:17:21 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/17 12:14:14 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

const std::string CGIServer::_python_interpr = "/usr/bin/python3";
const std::string CGIServer::_php_interpr = "/usr/bin/php";

void CGIServer::handleCGI(t_client& client)
{
	LOG_INFO(TEXT_GREEN, "Running CGI", RESET);
	LOG_DEBUG("handleCGI function started");
	std::string interpreter = determineInterpreter(client.request->getStartLine()["path"]);
	std::vector<std::string> envVars = setEnvironmentVariables(client.request);
	LOG_DEBUG("Enviroment has been set");
	handleProcesses(client, interpreter, envVars);
	LOG_DEBUG("handleCGI function ended");
}

std::string CGIServer::readErrorPage(const std::string& errorPagePath)
{
	std::ifstream file(errorPagePath);
	
	if (!file)
	{
		throw ResponseError(404, {}, "Exception has been thrown in readErrorPage() "
			"method of CGIServer class");
	}
	
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

std::string CGIServer::determineInterpreter(const std::string& filePath)
{
	if (filePath.substr(filePath.find_last_of(".") + 1) == "py")
	{
		LOG_DEBUG("Python specificator found");
		return _python_interpr;
	}
	else if (filePath.substr(filePath.find_last_of(".") + 1) == "php")
	{
		LOG_DEBUG("PHP specificator found");
		return _php_interpr;
	}
	else
	{
		throw ResponseError(404, {}, "Exception has been thrown in determineInterpreter() "
			"method of CGIServer class");
	}
}

std::vector<std::string> CGIServer::setEnvironmentVariables(Request* request)
{
	std::vector<std::string> env;

	env.push_back("REQUEST_METHOD=" + request->getStartLine()["method"]);
	env.push_back("QUERY_STRING=" + request->getStartLine()["query"]);
	env.push_back("SCRIPT_NAME=" + request->getStartLine()["path"].erase(0, 1));
	env.push_back("SERVER_PROTOCOL=" + request->getStartLine()["version"]);

	if (request->getStartLine()["method"] == "POST")
	{
		env.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
		env.push_back("CONTENT_LENGTH=" + std::to_string(request->getBody().size()));
	}
	return env;
}

void CGIServer::handleChildProcess(t_client& client, const std::string& interpreter,
	const std::string& filePath, const std::vector<std::string>& envVars)
{
	LOG_DEBUG("Duplicating stdin and stdout");
	if (dup2(client.parentPipe[_in], STDIN_FILENO) < 0 ||
		dup2(client.childPipe[_out], STDOUT_FILENO) < 0)
	{
		closeFds(client);
		throw ResponseError(500, {}, "Exception (dup2) has been thrown in handleChildProcess() "
			"method of CGIServer class");
	}

	close(client.parentPipe[_out]);
	close(client.childPipe[_in]);

	std::vector<char*> args;
	args.push_back(const_cast<char*>(interpreter.c_str()));
	args.push_back(const_cast<char*>(filePath.c_str()));
	args.push_back(nullptr);
	LOG_DEBUG("Agruments set");

	std::vector<char*> envp;
	for (const auto& var : envVars)
	{
		envp.push_back(const_cast<char*>(var.c_str()));
	}
	envp.push_back(nullptr);
	LOG_DEBUG("Environment casted");

	LOG_DEBUG("About to start execve");
	execve(interpreter.c_str(), args.data(), envp.data());
	close(client.parentPipe[_in]);
	close(client.childPipe[_out]);
	throw ResponseError(500, {}, "Exception (execve) has been thrown in handleChildProcess() "
		"method of CGIServer class");
}

void CGIServer::handleParentProcess(t_client& client, Response* response, const std::string& body)
{
	close(client.parentPipe[_in]);
	close(client.childPipe[_out]);

	LOG_DEBUG("Writing body of the request inside the pipe");
	write(client.parentPipe[_out], body.c_str(), body.size());
	close(client.parentPipe[_out]);

	char buffer[1024];
	ssize_t bytesRead;
	std::ostringstream oss;

	while ((bytesRead = read(client.childPipe[_in], buffer, sizeof(buffer))) > 0)
	{
		LOG_DEBUG("Populating response body");
		oss.write(buffer, bytesRead);
	}
	if (bytesRead < 0)
	{
		close(client.childPipe[_in]);
		throw ResponseError(500, {}, "Exception has been thrown in handleParentProcess() "
			"method of CGIServer class");
	}
	checkResponseHeaders(oss.str(), response);
	close(client.childPipe[_in]);
}

void CGIServer::handleProcesses(t_client& client, const std::string& interpreter,
	const std::vector<std::string>& envVars)
{
	if (pipe(client.parentPipe) == -1 || pipe(client.childPipe) == -1)
	{
		throw ResponseError(500, {}, "Exception (pipe) has been thrown in handleParentProcess() "
			"method of CGIServer class");
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		closeFds(client);
		throw ResponseError(500, {}, "Exception (fork) has been thrown in handleParentProcess() "
			"method of CGIServer class");
	}
	else if (pid == 0)
	{
		LOG_DEBUG("Child started");
		handleChildProcess(client, interpreter,
			client.request->getStartLine()["path"].erase(0, 1), envVars);
	}
	else
	{
		LOG_DEBUG("Parent started");
		g_childPids.push_back(pid);
		handleParentProcess(client, client.response, client.request->getBody());
		waitpid(pid, nullptr, 0);
		
		auto it = std::find(g_childPids.begin(), g_childPids.end(), pid);
		if (it != g_childPids.end())
			g_childPids.erase(it);
	}
	LOG_INFO(TEXT_GREEN, "CGI script executed", RESET);
}

void CGIServer::checkResponseHeaders(const std::string& result, Response* response)
{
	LOG_DEBUG("Checking for the headers in CGI output");
	size_t headerEnd = result.find("\n\n");
	
	if (headerEnd != std::string::npos)
	{
		std::string headers = result.substr(0, headerEnd);
		
		response->setBody(result.substr(headerEnd + 2));

		std::istringstream headerStream(headers);
		std::string line;
		
		while (std::getline(headerStream, line))
		{
			size_t separator = line.find(": ");
			if (separator != std::string::npos)
			{
				const std::string& key = line.substr(0, separator);
				std::string value = line.substr(separator + 2);
				if (key == "Content-Type")
				{
					response->setType(value);
				}
				else
				{
					response->setHeader(key, value);
				}
			}
		}
	}
	else
	{
		response->setBody(result);
	}
}

void CGIServer::closeFds(t_client& client)
{
	close(client.childPipe[_in]);
	close(client.childPipe[_out]);
	close(client.parentPipe[_in]);
	close(client.parentPipe[_out]);
}
