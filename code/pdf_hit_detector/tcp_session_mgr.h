#pragma once

#include "../frame/singleton.h"
#include "interface.h"
#include <boost/threadpool/threadpool.hpp>
#ifdef _WIN32
#include "..\third\Libnids-1.19\WIN32-Includes\nids.h"
#else
#include "../third/linux_libnids-1.19/src/nids.h"
#endif




struct k_tcp_session : public k_session_base<tcp_session_mgr_notifyer, notifyer_result, notifyer_result_keep, notifyer_result_drop>, public boost::noncopyable
{
	static boost::shared_ptr<k_tcp_session> create(boost::threadpool::pool* p_threadpool, const std::wstring& str_client_ip, unsigned int client_port, const std::wstring& str_server_ip, unsigned int server_port);
	void on_connect();
	void on_data(tcp_session_mgr_notifyer::tcp_data_type data_type, boost::shared_ptr<util_buffer> sp_data_buffer);
	void on_disconnect();
	//
	boost::shared_ptr<tcp_session> _sp_tcp_session;
	boost::threadpool::pool* _p_threadpool;
private:
	k_tcp_session()
		: _p_threadpool(NULL)

	{

	}
public:
	virtual ~k_tcp_session()
	{

	}
};

class tcp_session_mgr
{
public:
	bool initialize();
	void uninitialize();
	void add_notifyer(tcp_session_mgr_notifyer* p_notifyer);
public:
	void on_libnids_tcp_notify(struct tcp_stream *a_tcp, void ** this_time_not_needed);
private:
	void on_connect(struct tcp_stream *a_tcp);
	void on_data(struct tcp_stream *a_tcp);
	void on_disconnect(struct tcp_stream *a_tcp);
private:
	void _notify_connect(boost::shared_ptr<k_tcp_session> sp_k_tcp_session);
	void _notify_data(boost::shared_ptr<k_tcp_session> sp_k_tcp_session, tcp_session_mgr_notifyer::tcp_data_type data_type, boost::shared_ptr<util_buffer> sp_data_buffer);
	void _notify_disconnect(boost::shared_ptr<k_tcp_session> sp_k_tcp_session);
private:
	boost::threadpool::pool* alloc_threadpool();
	void release_threadpool(boost::threadpool::pool* p_threadpool);
private:
	std::map<struct tcp_stream*, boost::shared_ptr<k_tcp_session> > m_map_tcpstream_ktcpsession;
	std::queue<boost::threadpool::pool*> m_queue_idle_threadpool;
	std::set<tcp_session_mgr_notifyer*> m_set_notifyer;
};