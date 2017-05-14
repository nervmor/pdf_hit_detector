#include "tcp_session_mgr.h"
#include "../frame/util_string.h"
#include "../frame/util_log.h"
#include "define.h"
#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

bool tcp_session_mgr::initialize()
{
	return true;
}

void tcp_session_mgr::uninitialize()
{

}

void tcp_session_mgr::add_notifyer(tcp_session_mgr_notifyer* p_notifyer)
{
	m_set_notifyer.insert(p_notifyer);
}

void tcp_session_mgr::on_libnids_tcp_notify(struct tcp_stream *a_tcp, void ** this_time_not_needed)
{
	if (a_tcp->nids_state == NIDS_JUST_EST)
	{
		on_connect(a_tcp);
	}
	else if (a_tcp->nids_state == NIDS_CLOSE)
	{
		on_disconnect(a_tcp);
	}
	else if (a_tcp->nids_state == NIDS_RESET)
	{
		on_disconnect(a_tcp);
	}
	else if (a_tcp->nids_state == NIDS_DATA)
	{
		on_data(a_tcp);
	}
	else
	{

	}
}



void tcp_session_mgr::on_connect(struct tcp_stream *a_tcp)
{
	a_tcp->client.collect++;
	a_tcp->server.collect++;
// 	a_tcp->client.collect_urg++;
// 	a_tcp->server.collect_urg++;

	std::map<struct tcp_stream*, boost::shared_ptr<k_tcp_session> >::iterator it = m_map_tcpstream_ktcpsession.find(a_tcp);
	Assert(it == m_map_tcpstream_ktcpsession.end());

	boost::threadpool::pool* p_threadpool = alloc_threadpool();
	Assert(p_threadpool != NULL);
	std::wstring str_client_ip = util_string::a2w(inet_ntoa(*((struct in_addr *)&a_tcp->addr.saddr)));
	unsigned int client_port = a_tcp->addr.source;
	std::wstring str_server_ip = util_string::a2w(inet_ntoa(*((struct in_addr *)&a_tcp->addr.daddr)));
	unsigned int server_port = a_tcp->addr.dest;
	boost::shared_ptr<k_tcp_session> sp_k_tcp_session = k_tcp_session::create(p_threadpool, str_client_ip, client_port, str_server_ip, server_port);
	Assert(sp_k_tcp_session != NULL);
	
	for (std::set<tcp_session_mgr_notifyer*>::iterator itx = m_set_notifyer.begin(); itx != m_set_notifyer.end(); itx++)
	{
		tcp_session_mgr_notifyer* p_notifyer = *itx;
		sp_k_tcp_session->add_notifyer(p_notifyer);
	}
	m_map_tcpstream_ktcpsession.insert(std::make_pair(a_tcp, sp_k_tcp_session));
	p_threadpool->schedule(boost::bind(&tcp_session_mgr::_notify_connect, this, sp_k_tcp_session));
	util_log::logd(TCP_SESSION_MGR_LOG_TAG, "new tcp session connect.. %S", sp_k_tcp_session->_sp_tcp_session->get_network_link_info().format_info().c_str());
}

void tcp_session_mgr::on_data(struct tcp_stream *a_tcp)
{
	do 
	{
		std::map<struct tcp_stream*, boost::shared_ptr<k_tcp_session> >::iterator it = m_map_tcpstream_ktcpsession.find(a_tcp);
		ENSUREK(it != m_map_tcpstream_ktcpsession.end());
		boost::shared_ptr<k_tcp_session> sp_k_tcp_session = it->second;
		
		boost::threadpool::pool* p_threadpool = sp_k_tcp_session->_p_threadpool;

		if (sp_k_tcp_session->is_drop())
		{
			release_threadpool(p_threadpool);
			m_map_tcpstream_ktcpsession.erase(it);
			util_log::logd(TCP_SESSION_MGR_LOG_TAG, "[on_data]tcp session is droped.. %S", sp_k_tcp_session->_sp_tcp_session->get_network_link_info().format_info().c_str());
			break;
		}
		tcp_session_mgr_notifyer::tcp_data_type data_type = tcp_session_mgr_notifyer::tcp_data_client;
		struct half_stream *hlf = NULL;
		boost::shared_ptr<util_buffer> sp_data_buffer(new util_buffer);
		if (a_tcp->client.count_new)
		{
			hlf = &a_tcp->client; 
			data_type = tcp_session_mgr_notifyer::tcp_data_client;
			ENSUREK(sp_data_buffer->create(hlf->count_new, hlf->data));
		}
		else if (a_tcp->server.count_new)
		{
			hlf = &a_tcp->server;
			data_type = tcp_session_mgr_notifyer::tcp_data_server;
			ENSUREK(sp_data_buffer->create(hlf->count_new, hlf->data));
		}
		else
		{
			break;
		}
		p_threadpool->schedule(boost::bind(&tcp_session_mgr::_notify_data, this, sp_k_tcp_session, data_type, sp_data_buffer));
		util_log::logd(TCP_SESSION_MGR_LOG_TAG, "tcp session data [%s:%d].. %S", 
			data_type == tcp_session_mgr_notifyer::tcp_data_client ? "send" : "recv",
			sp_data_buffer->size(),
			sp_k_tcp_session->_sp_tcp_session->get_network_link_info().format_info().c_str());
	} while (false);
}

void tcp_session_mgr::on_disconnect(struct tcp_stream *a_tcp)
{
	do 
	{
		std::map<struct tcp_stream*, boost::shared_ptr<k_tcp_session> >::iterator it = m_map_tcpstream_ktcpsession.find(a_tcp);
		ENSUREK(it != m_map_tcpstream_ktcpsession.end());
		boost::shared_ptr<k_tcp_session> sp_k_tcp_session = it->second;
		
		boost::threadpool::pool* p_threadpool = sp_k_tcp_session->_p_threadpool;
		
		if (sp_k_tcp_session->is_drop())
		{
			release_threadpool(p_threadpool);
			m_map_tcpstream_ktcpsession.erase(it);
			util_log::logd(TCP_SESSION_MGR_LOG_TAG, "[on_disconnect]tcp session is droped.. %S", sp_k_tcp_session->_sp_tcp_session->get_network_link_info().format_info().c_str());
			break;
		}

		p_threadpool->schedule(boost::bind(&tcp_session_mgr::_notify_disconnect, this, sp_k_tcp_session));

		release_threadpool(p_threadpool);
		m_map_tcpstream_ktcpsession.erase(it);
		util_log::logd(TCP_SESSION_MGR_LOG_TAG, "tcp session disconnect.. %S", sp_k_tcp_session->_sp_tcp_session->get_network_link_info().format_info().c_str());
	} while (false);
}


void tcp_session_mgr::_notify_connect(boost::shared_ptr<k_tcp_session> sp_k_tcp_session)
{
	sp_k_tcp_session->on_connect();
}

void tcp_session_mgr::_notify_data(boost::shared_ptr<k_tcp_session> sp_k_tcp_session, tcp_session_mgr_notifyer::tcp_data_type data_type, boost::shared_ptr<util_buffer> sp_data_buffer)
{
	sp_k_tcp_session->on_data(data_type, sp_data_buffer);
}

void tcp_session_mgr::_notify_disconnect(boost::shared_ptr<k_tcp_session> sp_k_tcp_session)
{
	sp_k_tcp_session->on_disconnect();
}

boost::threadpool::pool* tcp_session_mgr::alloc_threadpool()
{
	boost::threadpool::pool* p_threadpool = NULL;
	if (m_queue_idle_threadpool.empty())
	{
		p_threadpool = new boost::threadpool::pool(1);
		p_threadpool->size_controller().resize(1);
	}
	else
	{
		p_threadpool = m_queue_idle_threadpool.front();
		m_queue_idle_threadpool.pop();
	}
	return p_threadpool;
}

void tcp_session_mgr::release_threadpool(boost::threadpool::pool* p_threadpool)
{
	m_queue_idle_threadpool.push(p_threadpool);
}



boost::shared_ptr<k_tcp_session> k_tcp_session::create(boost::threadpool::pool* p_threadpool, const std::wstring& str_client_ip, unsigned int client_port, const std::wstring& str_server_ip, unsigned int server_port)
{
	boost::shared_ptr<k_tcp_session> sp_k_tcp_session(new k_tcp_session);
	sp_k_tcp_session->_sp_tcp_session = tcp_session::create(str_client_ip, client_port, str_server_ip, server_port);
	Assert(sp_k_tcp_session->_sp_tcp_session != NULL);
	sp_k_tcp_session->_p_threadpool = p_threadpool;
	return sp_k_tcp_session;
}

void k_tcp_session::on_connect()
{
	ENSURET(!is_drop());
	notifyer_result result = notifyer_result_drop;
	for (std::map<tcp_session_mgr_notifyer*, notifyer_result>::iterator it = _map_notifyer_result.begin(); it != _map_notifyer_result.end(); it++)
	{
		tcp_session_mgr_notifyer* p_notifyer = it->first;
		notifyer_result& notifyer_result = it->second;
		if (notifyer_result == notifyer_result_keep)
		{
			notifyer_result = p_notifyer->on_tcp_session_connect(_sp_tcp_session);
			if (notifyer_result == notifyer_result_keep)
			{
				result = notifyer_result_keep;
			}
		}
	}
	if (result == notifyer_result_drop)
	{
		set_drop();
	}
}

void k_tcp_session::on_data(tcp_session_mgr_notifyer::tcp_data_type data_type, boost::shared_ptr<util_buffer> sp_data_buffer)
{
	ENSURET(!is_drop());
	notifyer_result result = notifyer_result_drop;
	for (std::map<tcp_session_mgr_notifyer*, notifyer_result>::iterator it = _map_notifyer_result.begin(); it != _map_notifyer_result.end(); it++)
	{
		tcp_session_mgr_notifyer* p_notifyer = it->first;
		notifyer_result& notifyer_result = it->second;
		if (notifyer_result == notifyer_result_keep)
		{
			notifyer_result = p_notifyer->on_tcp_session_data(_sp_tcp_session, data_type, sp_data_buffer);
			if (notifyer_result == notifyer_result_keep)
			{
				result = notifyer_result_keep;
			}
		}
	}
	if (result == notifyer_result_drop)
	{
		set_drop();
	}
}

void k_tcp_session::on_disconnect()
{
	ENSURET(!is_drop());
	for (std::map<tcp_session_mgr_notifyer*, notifyer_result>::iterator it = _map_notifyer_result.begin(); it != _map_notifyer_result.end(); it++)
	{
		tcp_session_mgr_notifyer* p_notifyer = it->first;
		notifyer_result& notifyer_result = it->second;
		if (notifyer_result == notifyer_result_keep)
		{
			p_notifyer->on_tcp_session_disconnect(_sp_tcp_session);
		}
	}
	set_drop();
}
