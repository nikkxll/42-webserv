/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:46 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/10 18:34:56 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "../config/Config.hpp"

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
static const std::string serverName = "webserv";
static const std::string charset = "; charset=UTF-8";
static const std::map<int, std::string> statusMessages = {
	{200, "OK"},
	{307, "Temporary Redirect"},
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"}, // if the location does not allowes method in request. Then put "Allowed: GET, POST" in response header
	{413, "Request Entity Too Large"}, // if the request body size exceeds the clientMaxBodySize
	{500, "Internal Server Error"} // can be used when the server runs into unexpected issues processing the request, including memory allocation failures
};

static const std::map<std::string, std::string> mimeTypes = {
	{"html", "text/html" + charset},
	{"htm", "text/html" + charset},
	{"css", "text/css" + charset},
	{"js", "application/javascript" + charset},

	{"ttf", "font/ttf"},
	{"woff", "font/woff"},
	{"woff2", "font/woff2"},

	{"jpg", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"png", "image/png"},
	{"gif", "image/gif"},
	{"svg", "image/svg+xml"},
	{"ico",	"image/x-icon"},
	{"bmp", "image/bmp"},
	{"tiff", "image/tiff"},
	{"webp", "image/webp"},

	{"mp3", "audio/mpeg"},
	{"wav", "audio/wav"},
	{"ogg", "audio/ogg"},

	{"mp4", "video/mp4"},
	{"webm", "video/webm"},
	{"ogg", "video/ogg"},

	{"txt", "text/plain" + charset},
	{"sitemap", "application/xml" + charset},
	{"json", "application/json" + charset},
	{"xml", "application/xml" + charset},
	{"csv", "text/csv" + charset},
	{"markdown", "text/markdown" + charset},
	{"pdf", "application/pdf"},
	{"zip", "application/zip"},
	{"gzip", "application/gzip"},

	// https://stackoverflow.com/questions/20508788/do-i-need-content-type-application-octet-stream-for-file-download
	{"default", "application/octet-stream"}
};

// HTTP/1.1 404 Not Found
// Date: Sun, 18 Oct 2012 10:36:20 GMT
// Server: Apache/2.2.14 (Win32)
// Content-Length: 230
// Connection: Closed
// Content-Type: text/html; charset=iso-8859-1

Response::Response() {}

/**
 * optionalHeaders: {{ "headerKey1": "headerValue1" }, { "headerKey2": "headerValue2}}
*/
Response::Response(int code, ServerConfig* serverConfig, std::map<std::string, std::string> optionalHeaders)
{
	setStatusFromCode(code);
	if (optionalHeaders.size() > 0)
		_headers.insert(optionalHeaders.begin(), optionalHeaders.end());

	if (code >= 400 && code <= 599)
	{
		std::string errorPagePath = serverConfig->defaultErrorPages[404];
		auto errorIt = serverConfig->errorPages.find(code);
		auto defaultErrorIt = serverConfig->defaultErrorPages.find(code);
		if (errorIt != serverConfig->errorPages.end())
			errorPagePath = serverConfig->errorPages[code];
		else if (defaultErrorIt != serverConfig->defaultErrorPages.end())
			errorPagePath = serverConfig->defaultErrorPages[code];
		*this = Response(code, errorPagePath);
	}
}

Response::Response(int code, std::string filePath)
{
	auto [file, size] = Utility::readBinaryFile(filePath);
	setStatusFromCode(code);

	setContentLength(size);

	std::string fileContent(file.begin(), file.end());

	size_t dotPos = filePath.find_last_of(".");

	if (dotPos != std::string::npos && mimeTypes.find(filePath.substr(filePath.find_last_of(".") + 1)) != mimeTypes.end())
	{
		setType(mimeTypes.at(filePath.substr(filePath.find_last_of(".") + 1)));
		setBody(fileContent);
	}
	else
	{
		setType(mimeTypes.at("default"));
		setBody(fileContent);
	}
}

std::string& Response::getBody()
{
	return _body;
}

std::string& Response::getStatus()
{
	return _status;
}

std::string& Response::getType()
{
	return _type;
}

int Response::getContentLength() const
{
	return _contentLength;
}

std::string Response::getHeader(std::string key)
{
	return _headers[key];
}

void Response::setBody(std::string body)
{
	_body = body;
}

void Response::setStatus(std::string status)
{
	_status = status;
}

void Response::setStatusFromCode(int code)
{
	_status = std::to_string(code) + " " + statusMessages.at(code);
}

void Response::setType(std::string type)
{
	_type = type;
}

void Response::setTypeFromFormat(std::string format)
{
	try
	{
		_type = mimeTypes.at(format);
	}
	catch (const std::exception& e)
	{
		_type = mimeTypes.at("default");
	}
}

void Response::setContentLength(int contentLength)
{
	_contentLength = contentLength;
}

void Response::setHeader(std::string& key, std::string& value)
{
	_headers[key] = value;
}

void Response::appendToBody(char* data, std::size_t length)
{
	_body.append(data, length);
}

std::string Response::buildResponse(Response& response)
{
	std::stringstream responseNew;

	if (response.getStatus().empty())
	{
		response._status = "200 OK";
	}
	responseNew << "HTTP/1.1 " << response.getStatus() << "\r\n";
	responseNew << "Date: " << Utility::getDate() << "\r\n";
	responseNew << "Server: webserv" << "\r\n";
	responseNew << "Connection: close" << "\r\n";
	responseNew << "Content-Length: " << response.getBody().size() << "\r\n";

	if (!response.getType().empty())
		responseNew << "Content-Type: " << response.getType() << "\r\n";

	/* Add optional headers*/
	for (auto& [headerKey, headerValue] : response._headers)
		responseNew << headerKey << ": " << headerValue << "\r\n";

	responseNew << "\r\n";

	/* Not adding extra line if body is empty*/
	if (response.getBody().size() != 0)
		responseNew << response.getBody() << "\r\n";
	return responseNew.str();
}
