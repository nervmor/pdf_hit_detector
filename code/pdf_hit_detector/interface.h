#pragma once

#include "tcp_session.h"
#include "http_session.h"
#include "http_download_session.h"

enum notifyer_result
{
	notifyer_result_keep,
	notifyer_result_drop
};

class tcp_session_mgr_notifyer
{
public:
	enum tcp_data_type
	{
		tcp_data_client,
		tcp_data_server
	};
public:
	virtual ~tcp_session_mgr_notifyer(){}
	virtual notifyer_result on_tcp_session_connect(boost::shared_ptr<tcp_session> sp_tcp_session) = 0;
	virtual notifyer_result on_tcp_session_data(boost::shared_ptr<tcp_session> sp_tcp_session, tcp_data_type type, boost::shared_ptr<util_buffer> sp_data_buffer) = 0;
	virtual void on_tcp_session_disconnect(boost::shared_ptr<tcp_session> sp_tcp_session) = 0;
};



class http_session_mgr_notifyer
{
public:
	virtual ~http_session_mgr_notifyer(){}
	virtual notifyer_result on_http_session_request_header(boost::shared_ptr<http_session> sp_http_session) = 0;
	virtual notifyer_result on_http_session_respond_header(boost::shared_ptr<http_session> sp_http_session) = 0;
	virtual void on_http_session_error(boost::shared_ptr<http_session> sp_http_session) = 0;
	virtual void on_http_session_complete(boost::shared_ptr<http_session> sp_http_session) = 0;
};



class http_download_session_mgr_notifyer
{
public:
	virtual ~http_download_session_mgr_notifyer(){}
	virtual void on_integral_http_download_session(boost::shared_ptr<http_download_session> sp_http_download_session) = 0;
	virtual void on_incomplete_http_download_session(boost::shared_ptr<http_download_session> sp_http_download_session) = 0;
};