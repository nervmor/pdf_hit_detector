#pragma once

#include "interface.h"
#include "../frame/singleton.h"

#include <boost/thread.hpp>

#include "http_analyzer.h"


struct k_http_session : public k_session_base<http_session_mgr_notifyer, notifyer_result, notifyer_result_keep, notifyer_result_drop>, 
	public boost::noncopyable, public http_request_analyzer::notifyer, public http_respond_analyzer::notifyer
{
	static boost::shared_ptr<k_http_session> create(const network_link_info& link_info);
	void destroy();

	common_result on_respond_data(boost::shared_ptr<util_buffer> sp_data_buffer);
	common_result on_request_data(boost::shared_ptr<util_buffer> sp_data_buffer);
	void on_disconnect();
	
	virtual bool on_http_request_header(boost::shared_ptr<http_session::http_request::header> sp_request_header);
	virtual void on_http_request_complete(const http_session::http_request& request);
	virtual void on_http_request_error(const http_session::http_request& request);


	virtual bool on_http_respond_header(boost::shared_ptr<http_session::http_respond::header> sp_respond_header);
	virtual void on_http_respond_complete(const http_session::http_respond& respond);
	virtual void on_http_respond_error(const http_session::http_respond& respond);

	http_request_analyzer _request_analyzer;
	http_respond_analyzer _respond_analyzer;
	boost::shared_ptr<http_session> _sp_http_session;
private:
	k_http_session()
		: k_session_base()
	{
		
	}
public:
	~k_http_session(){}
};


class http_session_mgr : public tcp_session_mgr_notifyer
{
public:
	virtual notifyer_result on_tcp_session_connect(boost::shared_ptr<tcp_session> sp_tcp_session);
	virtual notifyer_result on_tcp_session_data(boost::shared_ptr<tcp_session> sp_tcp_session, tcp_session_mgr_notifyer::tcp_data_type type, boost::shared_ptr<util_buffer> sp_data_buffer);
	virtual void on_tcp_session_disconnect(boost::shared_ptr<tcp_session> sp_tcp_session);
public:
	bool initialize();
	void uninitialize();
	void add_notifyer(http_session_mgr_notifyer* p_notifyer);
private:
	struct tcp_http_session_context
	{
		std::queue<boost::shared_ptr<k_http_session> > _queue_respond_k_http_session;
		std::queue<boost::shared_ptr<k_http_session> > _queue_request_k_http_session;
	};
private:
	std::set<http_session_mgr_notifyer*> m_set_notifyer;
};