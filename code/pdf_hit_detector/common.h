#pragma once


#include "../frame/util_string.h"

#include <map>
enum common_result
{
	common_result_success,
	common_result_complete,
	common_result_error
};


struct network_link_info 
{
	std::wstring _str_client_ip;
	unsigned int _client_port;
	std::wstring _str_server_ip;
	unsigned int _server_port;

	network_link_info(const std::wstring& str_client_ip, unsigned int client_port, const std::wstring& str_server_ip, unsigned int server_port)
		: _str_client_ip(str_client_ip)
		, _client_port(client_port)
		, _str_server_ip(str_server_ip)
		, _server_port(server_port)
	{

	}
	network_link_info(const network_link_info& link_info)
		: _str_client_ip(link_info._str_client_ip)
		, _client_port(link_info._client_port)
		, _str_server_ip(link_info._str_server_ip)
		, _server_port(link_info._server_port)
	{

	}
	std::wstring format_info()
	{
		char szinfo[256];
		sprintf(szinfo, "[%S:%d]--->[%S:%d]", _str_client_ip.c_str(), _client_port, _str_server_ip.c_str(), _server_port);
		return util_string::a2w(szinfo);
	}
};


template <class notifyer_type, typename notifyer_result_type, unsigned int result_keep, unsigned int result_drop>
struct k_session_base
{
	bool is_drop(){return m_b_drop;}
	void set_drop(){m_b_drop = true;}
	void add_notifyer(notifyer_type* p_notifyer){_map_notifyer_result.insert(std::make_pair(p_notifyer, static_cast<notifyer_result_type>(result_keep)));}
	void remove_notifyer(notifyer_type* p_notifyer){typename std::map<notifyer_type*, notifyer_result_type>::iterator it = _map_notifyer_result.find(p_notifyer);if (it != _map_notifyer_result.end()){_map_notifyer_result.erase(it);}}
	void notifyer_set_drop(notifyer_type* p_notifyer){typename std::map<notifyer_type*, notifyer_result_type>::iterator it = _map_notifyer_result.find(p_notifyer);if (it != _map_notifyer_result.end()){it->second = static_cast<notifyer_result_type>(result_drop);}}

	//
	bool m_b_drop;
	std::map<notifyer_type*, notifyer_result_type> _map_notifyer_result;

	k_session_base()
		: m_b_drop(false)
	{

	}
	virtual ~k_session_base()
	{

	}
};