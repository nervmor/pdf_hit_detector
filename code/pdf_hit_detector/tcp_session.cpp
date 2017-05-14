#include "tcp_session.h"
#include "../frame/util_string.h"

boost::shared_ptr<tcp_session> tcp_session::create(const std::wstring& str_client_ip, unsigned int client_port, const std::wstring& str_server_ip, unsigned int server_port)
{
	boost::shared_ptr<tcp_session> sp_tcp_session(new tcp_session(str_client_ip, client_port, str_server_ip, server_port));
	return sp_tcp_session;
}
