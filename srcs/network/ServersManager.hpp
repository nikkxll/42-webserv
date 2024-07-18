/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:53 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/18 03:45:00 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"
#include "../config/Config.hpp"
#include "../response/Response.hpp"
#include "../utils/logUtils.hpp"
#include "CGIHandler.hpp"
#include <vector>
#include <poll.h>
#include <csignal>
#include <string>
#include <errno.h>

#include <exception>
#include "../utils/signal.hpp"

#define DEFAULT_CONFIG "default/config.conf"

/* Can be only instatiated once */
class ServersManager
{
	private:
		static ServersManager*		_instance;
		static std::vector<Server*>	_servers;
		static Config*				_webservConfig;
		std::vector<struct pollfd>	_fds;

		ServersManager();
		// ServersManager(Config& webservConfig);
		ServersManager(const ServersManager&) = delete;
		ServersManager& operator=(const ServersManager&) = delete;

		void						handleRead(struct pollfd& pfdReadyForRead);
		void						handleWrite(int fdReadyForWrite);
		void						removeFromPollfd(int fd);
		void						removeClientByFd(int fd);
		bool						ifCGIsFd(t_client& client, int fd);
		pollfd*						findPollfdByFd(int fd);

	public:
		~ServersManager();
		static ServersManager*		getInstance();

		void						run();
		static void					initConfig(const char *fileNameString);
};
