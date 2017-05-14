#include "http_session_mgr.h"
#include "define.h"
#include "../frame/util_log.h"

notifyer_result http_session_mgr::on_tcp_session_connect(boost::shared_ptr<tcp_session> sp_tcp_session)
{
	return notifyer_result_keep;
}

notifyer_result http_session_mgr::on_tcp_session_data(boost::shared_ptr<tcp_session> sp_tcp_session, tcp_data_type type, boost::shared_ptr<util_buffer> sp_data_buffer)
{
	notifyer_result result = notifyer_result_drop;
	do 
	{
		boost::any context;
		boost::shared_ptr<tcp_http_session_context> sp_context;
		boost::shared_ptr<k_http_session> sp_k_http_session;
		if (type == tcp_session_mgr_notifyer::tcp_data_server)
		{
			if (!sp_tcp_session->get_context(HTTP_CONTEXT_NAME, context))
			{
				ENSUREK(http_checker::is_valid_http_request(sp_data_buffer));
				sp_context = boost::shared_ptr<tcp_http_session_context>(new tcp_http_session_context);
				context = sp_context;
				sp_tcp_session->add_context(HTTP_CONTEXT_NAME, context);
			}
			else
			{
				sp_context = boost::any_cast<boost::shared_ptr<tcp_http_session_context> >(context);
			}
			if (sp_context->_queue_request_k_http_session.empty())
			{
				sp_k_http_session = k_http_session::create(sp_tcp_session->get_network_link_info());
				Assert(sp_k_http_session);
				ENSUREK(sp_k_http_session);
				sp_context->_queue_request_k_http_session.push(sp_k_http_session);
				for (std::set<http_session_mgr_notifyer*>::iterator it = m_set_notifyer.begin(); it != m_set_notifyer.end(); it++)
				{
					http_session_mgr_notifyer* p_notifyer = *it;
					sp_k_http_session->add_notifyer(p_notifyer);
				}
				util_log::logd(HTTP_SESSION_MGR_LOG_TAG, "new http session create...");
			}
			else
			{
				sp_k_http_session = sp_context->_queue_request_k_http_session.front();
			}
			common_result result = sp_k_http_session->on_request_data(sp_data_buffer);
			if (result == common_result_complete)
			{
				// analyze complete, push to respond queue for ready
				sp_context->_queue_request_k_http_session.pop();
				sp_context->_queue_respond_k_http_session.push(sp_k_http_session);
			}
			else if (result == common_result_error)
			{
				// analyze error, remove it
				sp_context->_queue_request_k_http_session.pop();
				sp_k_http_session->destroy();
			}
			else if (result == common_result_success)
			{
				// keep analyze
			}
			else
			{
				Assert(false);
			}
		}
		else if (type == tcp_session_mgr_notifyer::tcp_data_client)
		{
			ENSUREK(sp_tcp_session->get_context(HTTP_CONTEXT_NAME, context));
			sp_context = boost::any_cast<boost::shared_ptr<tcp_http_session_context> >(context);
			ENSUREK(!sp_context->_queue_respond_k_http_session.empty());			
			sp_k_http_session = sp_context->_queue_respond_k_http_session.front();

			common_result result = sp_k_http_session->on_respond_data(sp_data_buffer);
			if (result == common_result_success)
			{

			}
			else if (result == common_result_complete)
			{
				sp_context->_queue_respond_k_http_session.pop();
				sp_k_http_session->destroy();
			}
			else if (result == common_result_error)
			{
				sp_context->_queue_respond_k_http_session.pop();
				sp_k_http_session->destroy();
			}
			else
			{
				Assert(false);
			}
			util_log::logd(HTTP_SESSION_MGR_LOG_TAG, "http session is drop...");
		}
		else
		{
			Assert(false);
		}
		
		result = notifyer_result_keep;
	} while (false);
	return result;
}

void http_session_mgr::on_tcp_session_disconnect(boost::shared_ptr<tcp_session> sp_tcp_session)
{
	do 
	{
		boost::any context;
		boost::shared_ptr<tcp_http_session_context> sp_context;
		boost::shared_ptr<k_http_session> sp_k_http_session;
		ENSUREK(sp_tcp_session->get_context(HTTP_CONTEXT_NAME, context));
		sp_context = boost::any_cast<boost::shared_ptr<tcp_http_session_context> >(context);

		while (!sp_context->_queue_request_k_http_session.empty())
		{
			sp_context->_queue_request_k_http_session.pop();
		}
		while (!sp_context->_queue_respond_k_http_session.empty())
		{
			sp_k_http_session = sp_context->_queue_respond_k_http_session.front();
			sp_context->_queue_respond_k_http_session.pop();

			sp_k_http_session->on_disconnect();
			sp_k_http_session->destroy();

			util_log::logd(HTTP_SESSION_MGR_LOG_TAG, "http session is drop with tcp disconnect...");
		}
		sp_tcp_session->remove_context(HTTP_CONTEXT_NAME);

	} while (false);
}

bool http_session_mgr::initialize()
{
	return true;
}

void http_session_mgr::uninitialize()
{

}

void http_session_mgr::add_notifyer(http_session_mgr_notifyer* p_notifyer)
{
	m_set_notifyer.insert(p_notifyer);
}









boost::shared_ptr<k_http_session> k_http_session::create(const network_link_info& link_info)
{
	boost::shared_ptr<k_http_session> sp_k_http_session(new k_http_session);
	sp_k_http_session->_sp_http_session = http_session::create(link_info);

	sp_k_http_session->_request_analyzer.initialize(sp_k_http_session.get());
	sp_k_http_session->_respond_analyzer.initialize(sp_k_http_session.get());

	return sp_k_http_session;
}

void k_http_session::destroy()
{
	_request_analyzer.uninitialize();
	_respond_analyzer.uninitialize();
}


common_result k_http_session::on_respond_data(boost::shared_ptr<util_buffer> sp_data_buffer)
{
	return _respond_analyzer.analyze(sp_data_buffer);
}

common_result k_http_session::on_request_data(boost::shared_ptr<util_buffer> sp_data_buffer)
{
	return _request_analyzer.analyze(sp_data_buffer);
}

void k_http_session::on_disconnect()
{
	_request_analyzer.breakoff();
	_respond_analyzer.breakoff();
}

bool k_http_session::on_http_request_header(boost::shared_ptr<http_session::http_request::header> sp_request_header)
{
	_sp_http_session->set_request_header(sp_request_header);

	notifyer_result result = notifyer_result_drop;
	for (std::map<http_session_mgr_notifyer*, notifyer_result>::iterator it = _map_notifyer_result.begin(); it != _map_notifyer_result.end(); it++)
	{
		http_session_mgr_notifyer* p_notifyer = it->first;
		if (p_notifyer->on_http_session_request_header(_sp_http_session) == notifyer_result_keep)
		{
			result = notifyer_result_keep;
		}
	}
	ENSUREf(result == notifyer_result_keep);
	return true;
}

void k_http_session::on_http_request_complete(const http_session::http_request& request)
{
	
}

void k_http_session::on_http_request_error(const http_session::http_request& request)
{
	_sp_http_session->set_request_body(request._sp_body);
	for (std::map<http_session_mgr_notifyer*, notifyer_result>::iterator it = _map_notifyer_result.begin(); it != _map_notifyer_result.end(); it++)
	{
		http_session_mgr_notifyer* p_notifyer = it->first;
		p_notifyer->on_http_session_error(_sp_http_session);
	}
}



bool k_http_session::on_http_respond_header(boost::shared_ptr<http_session::http_respond::header> sp_respond_header)
{
	_sp_http_session->set_respond_header(sp_respond_header);

	notifyer_result result = notifyer_result_drop;
	for (std::map<http_session_mgr_notifyer*, notifyer_result>::iterator it = _map_notifyer_result.begin(); it != _map_notifyer_result.end(); it++)
	{
		http_session_mgr_notifyer* p_notifyer = it->first;
		if (p_notifyer->on_http_session_respond_header(_sp_http_session) == notifyer_result_keep)
		{
			result = notifyer_result_keep;
		}
	}
	ENSUREf(result == notifyer_result_keep);
	return true;
}

void k_http_session::on_http_respond_complete(const http_session::http_respond& respond)
{
	_sp_http_session->set_respond_body(respond._sp_body);
	for (std::map<http_session_mgr_notifyer*, notifyer_result>::iterator it = _map_notifyer_result.begin(); it != _map_notifyer_result.end(); it++)
	{
		http_session_mgr_notifyer* p_notifyer = it->first;
		p_notifyer->on_http_session_complete(_sp_http_session);
	}
}

void k_http_session::on_http_respond_error(const http_session::http_respond& respond)
{
	_sp_http_session->set_respond_body(respond._sp_body);
	for (std::map<http_session_mgr_notifyer*, notifyer_result>::iterator it = _map_notifyer_result.begin(); it != _map_notifyer_result.end(); it++)
	{
		http_session_mgr_notifyer* p_notifyer = it->first;
		p_notifyer->on_http_session_error(_sp_http_session);
	}
}
