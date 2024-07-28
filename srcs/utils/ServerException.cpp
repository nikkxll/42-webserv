/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 00:51:19 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/28 20:53:51 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerException.hpp"

ServerException::ServerException(const std::string message) : _message(message) {
	_errno = errno;
}

const char* ServerException::what() const noexcept
{
	return _message.c_str();
}

int ServerException::getErrno() const
{
	return _errno;
}

ResponseError::ResponseError(int code, std::map<std::string, std::string> optionalHeaders, std::string message) : ServerException("Response error")
{
	_code = code;
	_headers = optionalHeaders;
	_message = message;
}

int ResponseError::getCode() const
{
	return _code;
}

std::map<std::string, std::string> ResponseError::getHeaders() const
{
	return _headers;
}