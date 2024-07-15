/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionsManager.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/12 12:23:32 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/15 12:33:21 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <map>
#include <deque>
#include <algorithm>

#include "client.hpp"
#include "../utils/Utility.hpp"
#include "../request/Request.hpp"

class SessionsManager {
	private:
		static std::string _session;
		const static std::string _filename;
		const static size_t _MAX_SESSIONS = 500;
		
		static bool sessionExistsCheck(std::string& sessionData);
		static void generateSession();
		static void checkIfFileExist();
		static void addSessionToFile(std::string& sessionData);
		static void manageSessions(std::deque<std::string>& sessions);
		static void setSessionToResponse(Response* response, std::string& sessionData);
		
	public:
		static void setSession(std::string session);
		static std::string& getSession();
		const std::string getFilename();
		
		static void handleSessions(t_client& client);
};
