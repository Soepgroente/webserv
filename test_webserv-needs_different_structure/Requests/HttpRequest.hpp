/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpRequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/14 16:01:55 by akasiota      #+#    #+#                 */
/*   Updated: 2024/11/14 19:35:51 by akasiota      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>

class HttpRequest
{
	private:
		std::string	method;
		std::string	path;
		std::string	version;
		std::string	host;
		std::map<std::string, std::string>	headers;
		std::string	body;
	public:
		HttpRequest();
		void	parse(std::string& request);
}