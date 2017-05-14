#pragma once


#include "../frame/include.h"
#include "common.h"
#include <boost/any.hpp>

class tcp_session : public boost::noncopyable
{
public:
	static boost::shared_ptr<tcp_session> create(const std::wstring& str_client_ip, unsigned int client_port, const std::wstring& str_server_ip, unsigned int server_port);
public:
	network_link_info get_network_link_info(){return m_network_link_info;}
	void add_context(const std::wstring& str_context_name, boost::any context){m_map_name_context.insert(std::make_pair(str_context_name, context));}
	bool get_context(const std::wstring& str_context_name, boost::any& context){std::map<std::wstring, boost::any>::iterator it = m_map_name_context.find(str_context_name);ENSUREf(it != m_map_name_context.end()); context = it->second;return true;}
	void remove_context(const std::wstring& str_context_name){std::map<std::wstring, boost::any>::iterator it = m_map_name_context.find(str_context_name);if (it != m_map_name_context.end()){m_map_name_context.erase(it);}}
private:
	network_link_info m_network_link_info;
	bool m_b_drop;
	std::map<std::wstring, boost::any> m_map_name_context;
private:
	tcp_session(const std::wstring& str_client_ip, unsigned int client_port, const std::wstring& str_server_ip, unsigned int server_port)
		: m_network_link_info(str_client_ip, client_port, str_server_ip, server_port)
		, m_b_drop(false)
	{

	}
};