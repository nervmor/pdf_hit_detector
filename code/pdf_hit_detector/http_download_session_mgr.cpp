#include "http_download_session_mgr.h"
#include "../frame/util_log.h"
#include "define.h"
#include "../third/arithmetic/blizzard_hash.h"
#include "config.h"

notifyer_result http_download_session_mgr::on_http_session_request_header(boost::shared_ptr<http_session> sp_http_session)
{
	notifyer_result result = notifyer_result_drop;
	do 
	{
		boost::shared_ptr<http_session::http_request::header> sp_request_header = sp_http_session->get_request_header();
		ENSUREK(sp_request_header);
		ENSUREK(!sp_request_header->_str_url.empty());
		result = notifyer_result_keep;
	} while (false);
	return result;
}

notifyer_result http_download_session_mgr::on_http_session_respond_header(boost::shared_ptr<http_session> sp_http_session)
{
	notifyer_result result = notifyer_result_drop;
	do 
	{
		boost::shared_ptr<http_session::http_respond::header> sp_respond_header = sp_http_session->get_respond_header();
		ENSUREK(sp_respond_header);
		ENSUREK(sp_respond_header->_content_length);
		ENSUREK(*sp_respond_header->_content_length != 0);
		if (sp_respond_header->_total_length)
		{
			ENSUREK(*sp_http_session->get_respond_header()->_total_length >= config::instance()->get_min_pdf_file_size());
		}
		else
		{
			ENSUREK(*sp_http_session->get_respond_header()->_content_length >= config::instance()->get_min_pdf_file_size());
		}
		result = notifyer_result_keep;
	} while (false);
	return result;
}

void http_download_session_mgr::on_http_session_error(boost::shared_ptr<http_session> sp_http_session)
{
	do 
	{
		boost::shared_ptr<util_buffer> sp_respond_body = sp_http_session->get_respond_body();
		ENSUREK(sp_respond_body);
		ENSUREK(!sp_respond_body->empty());
		{
			boost::mutex::scoped_lock scopedlock(m_dispatch_mutex);
			m_dispatch_threadpool.schedule(boost::bind(&http_download_session_mgr::_put_error_http_session, this, sp_http_session));
		}
	} while (false);
}

void http_download_session_mgr::on_http_session_complete(boost::shared_ptr<http_session> sp_http_session)
{
	do 
	{
		unsigned int respond_body_size = *sp_http_session->get_respond_header()->_content_length;
		
		boost::shared_ptr<util_buffer> sp_respond_body = sp_http_session->get_respond_body();
		ENSUREK(sp_respond_body);
		ENSUREK(respond_body_size == sp_respond_body->size());
		{
			boost::mutex::scoped_lock scopedlock(m_dispatch_mutex);
			m_dispatch_threadpool.schedule(boost::bind(&http_download_session_mgr::_put_completed_http_session, this, sp_http_session));
		}
	} while (false);
}

bool http_download_session_mgr::initialize()
{
	m_dispatch_threadpool.size_controller().resize(1);
	m_process_threadpool.size_controller().resize(1);
	m_notifyer_threadpool.size_controller().resize(1);
	m_clean_threadpool.size_controller().resize(1);
	m_clean_threadpool.schedule(boost::bind(&http_download_session_mgr::_check_expire_k_http_download_session, this));
	return true;
}

void http_download_session_mgr::uninitialize()
{

}

void http_download_session_mgr::add_notifyer(http_download_session_mgr_notifyer* p_notifyer)
{
	m_set_notifyer.insert(p_notifyer);
}

void http_download_session_mgr::_put_completed_http_session(boost::shared_ptr<http_session> sp_http_session)
{
	bool b_integral = false;
	do 
	{
		ENSUREK(!sp_http_session->get_respond_header()->_total_length);
		b_integral = true;
	} while (false);
	if (b_integral)
	{
		m_process_threadpool.schedule(boost::bind(&http_download_session_mgr::_process_integral_http_session, this, sp_http_session));
	}
	else
	{
		_put_error_http_session(sp_http_session);
	}
}

void http_download_session_mgr::_put_error_http_session(boost::shared_ptr<http_session> sp_http_session)
{
	boost::shared_ptr<k_http_download_session> sp_k_http_download_session;
	unsigned int hash1 = hash_1(sp_http_session);
	unsigned int hash2 = hash_2(sp_http_session);

	std::map<unsigned int, std::map<unsigned int, boost::shared_ptr<k_http_download_session> > >::iterator it1 = m_map_hash_k_http_download_session.find(hash1);
	if (it1 != m_map_hash_k_http_download_session.end())
	{
		std::map<unsigned int, boost::shared_ptr<k_http_download_session> >& map_hash2_k_http_download_session = it1->second;
		std::map<unsigned int, boost::shared_ptr<k_http_download_session> >::iterator it2 = map_hash2_k_http_download_session.find(hash2);
		if (it2 != map_hash2_k_http_download_session.end())
		{
			sp_k_http_download_session = it2->second;
		}
		else
		{
			util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "[Assert]hash2 is not exist happend in error http_session...");
			return;
		}
	}
	else
	{
		sp_k_http_download_session = k_http_download_session::create(sp_http_session);
		ENSURET(sp_k_http_download_session);
		unsigned int hash1 = hash_1(sp_http_session);
		unsigned int hash2 = hash_2(sp_http_session);

		std::map<unsigned int, boost::shared_ptr<k_http_download_session> > map_hash2_k_http_download_session;
		map_hash2_k_http_download_session.insert(std::make_pair(hash2, sp_k_http_download_session));
		m_map_hash_k_http_download_session.insert(std::make_pair(hash1, map_hash2_k_http_download_session));

		m_map_k_http_download_session_hash.insert(std::make_pair(sp_k_http_download_session, std::make_pair(hash1, hash2)));
	}
	m_process_threadpool.schedule(boost::bind(&http_download_session_mgr::_process_incomplete_http_session, this, sp_k_http_download_session, sp_http_session));
}

void http_download_session_mgr::_report_http_download_session(boost::shared_ptr<k_http_download_session> sp_k_http_download_session)
{
	sp_k_http_download_session->update();
}

void http_download_session_mgr::_remove_k_http_download_session(boost::shared_ptr<k_http_download_session> sp_k_http_download_session)
{
	std::map<boost::shared_ptr<k_http_download_session>, std::pair<unsigned int, unsigned int> >::iterator it = m_map_k_http_download_session_hash.find(sp_k_http_download_session);
	ENSURET(it != m_map_k_http_download_session_hash.end());

	std::pair<unsigned int, unsigned int>& hash1_hash2 = it->second;
	unsigned int hash1 = hash1_hash2.first;
	unsigned int hash2 = hash1_hash2.second;
	m_map_k_http_download_session_hash.erase(it);

	std::map<unsigned int, std::map<unsigned int, boost::shared_ptr<k_http_download_session> > >::iterator itx = m_map_hash_k_http_download_session.find(hash1);
	Assert(itx != m_map_hash_k_http_download_session.end());
	std::map<unsigned int, boost::shared_ptr<k_http_download_session> >& map_hash2_k_http_download_session = itx->second;
	std::map<unsigned int, boost::shared_ptr<k_http_download_session> > ::iterator ity = map_hash2_k_http_download_session.find(hash2);
	Assert(ity != map_hash2_k_http_download_session.end());
	map_hash2_k_http_download_session.erase(ity);
	if (map_hash2_k_http_download_session.empty())
	{
		m_map_hash_k_http_download_session.erase(itx);
	}
}

void http_download_session_mgr::_clean_expire_k_http_download_session()
{
	std::set<boost::shared_ptr<k_http_download_session> > set_expire_k_http_download_session;
	for (std::map<unsigned int, std::map<unsigned int, boost::shared_ptr<k_http_download_session> > >::iterator it = m_map_hash_k_http_download_session.begin(); it != m_map_hash_k_http_download_session.end(); it++)
	{
		std::map<unsigned int, boost::shared_ptr<k_http_download_session> >& map_hash2_http_download_session = it->second;
		for (std::map<unsigned int, boost::shared_ptr<k_http_download_session> >::iterator itx = map_hash2_http_download_session.begin(); itx != map_hash2_http_download_session.end(); itx++)
		{
			boost::shared_ptr<k_http_download_session> sp_k_http_download_session = itx->second;
			if (sp_k_http_download_session->is_expire())
			{
				set_expire_k_http_download_session.insert(sp_k_http_download_session);
			}
		}
	}
	for (std::set<boost::shared_ptr<k_http_download_session> >::iterator it = set_expire_k_http_download_session.begin(); it != set_expire_k_http_download_session.end(); it++)
	{
		boost::shared_ptr<k_http_download_session> sp_k_http_download_session = *it;
		_remove_k_http_download_session(sp_k_http_download_session);

		boost::shared_ptr<http_download_session> sp_http_download_session = sp_k_http_download_session->get_http_download_session();
		if (sp_http_download_session)
		{
			m_notifyer_threadpool.schedule(boost::bind(&http_download_session_mgr::_notify_incomplete_http_download_session, this, sp_http_download_session));
		}
	}
}

void http_download_session_mgr::_check_expire_k_http_download_session()
{
	boost::this_thread::sleep(boost::posix_time::seconds(config::instance()->get_http_download_session_expire_time() / 4));
	{
		boost::mutex::scoped_lock scopedlock(m_dispatch_mutex);
		m_dispatch_threadpool.schedule(boost::bind(&http_download_session_mgr::_clean_expire_k_http_download_session, this));
	}
	m_clean_threadpool.schedule(boost::bind(&http_download_session_mgr::_check_expire_k_http_download_session, this));
}

void http_download_session_mgr::_process_integral_http_session(boost::shared_ptr<http_session> sp_http_session)
{
	boost::shared_ptr<k_http_download_session> sp_k_http_download_session = k_http_download_session::create(sp_http_session);
	ENSURET(sp_k_http_download_session);
	if (sp_k_http_download_session->process(sp_http_session))
	{
		m_notifyer_threadpool.schedule(boost::bind(&http_download_session_mgr::_notify_integral_http_download_session, this, sp_k_http_download_session->get_http_download_session()));
		{
			boost::mutex::scoped_lock scopedlock(m_dispatch_mutex);
			m_dispatch_threadpool.schedule(boost::bind(&http_download_session_mgr::_remove_k_http_download_session, this, sp_k_http_download_session));
		}
	}
	else
	{
		util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "process integral http session...");
	}
}

void http_download_session_mgr::_process_incomplete_http_session(boost::shared_ptr<k_http_download_session> sp_k_http_download_session, boost::shared_ptr<http_session> sp_http_session)
{
	if (sp_k_http_download_session->process(sp_http_session))
	{
		{
			m_dispatch_threadpool.schedule(boost::bind(&http_download_session_mgr::_remove_k_http_download_session, this, sp_k_http_download_session));
			m_dispatch_threadpool.schedule(boost::bind(&http_download_session_mgr::_remove_k_http_download_session, this, sp_k_http_download_session));
		}
		m_dispatch_threadpool.wait();
		m_notifyer_threadpool.schedule(boost::bind(&http_download_session_mgr::_notify_integral_http_download_session, this, sp_k_http_download_session->get_http_download_session()));
	}
}

void http_download_session_mgr::_notify_integral_http_download_session(boost::shared_ptr<http_download_session> sp_http_session)
{
	for (std::set<http_download_session_mgr_notifyer*>::iterator it = m_set_notifyer.begin(); it != m_set_notifyer.end(); it++)
	{
		http_download_session_mgr_notifyer* p_notifyer = *it;
		p_notifyer->on_integral_http_download_session(sp_http_session);
	}
}

void http_download_session_mgr::_notify_incomplete_http_download_session(boost::shared_ptr<http_download_session> sp_http_session)
{
	for (std::set<http_download_session_mgr_notifyer*>::iterator it = m_set_notifyer.begin(); it != m_set_notifyer.end(); it++)
	{
		http_download_session_mgr_notifyer* p_notifyer = *it;
		p_notifyer->on_incomplete_http_download_session(sp_http_session);
	}
}

unsigned int http_download_session_mgr::hash_1(boost::shared_ptr<http_session> sp_http_session)
{
	std::wstring str_key;
	str_key += sp_http_session->get_network_link_info()._str_client_ip;
	str_key += sp_http_session->get_network_link_info()._str_server_ip;
	str_key += util_string::i2w(sp_http_session->get_network_link_info()._server_port);
	str_key += sp_http_session->get_request_header()->_str_url;
	str_key += sp_http_session->get_request_header()->_str_useragent;
	return blizzard_hash::hash((const unsigned char*)str_key.c_str(), str_key.size() * sizeof(wchar_t));
}

unsigned int http_download_session_mgr::hash_2(boost::shared_ptr<http_session> sp_http_session)
{
	std::wstring str_key;
	str_key += sp_http_session->get_request_header()->_str_useragent;
	str_key += sp_http_session->get_request_header()->_str_url;
	str_key += util_string::i2w(sp_http_session->get_network_link_info()._server_port);
	str_key += sp_http_session->get_network_link_info()._str_server_ip;
	str_key += sp_http_session->get_network_link_info()._str_client_ip;
	return blizzard_hash::hash((const unsigned char*)str_key.c_str(), str_key.size() * sizeof(wchar_t));
}













boost::shared_ptr<k_http_download_session> k_http_download_session::create(boost::shared_ptr<http_session> sp_http_session)
{
	boost::shared_ptr<k_http_download_session> sp_k_http_download_session;
	do 
	{
		std::wstring str_url = sp_http_session->get_request_header()->_str_url;
		ENSUREK(!str_url.empty());
		std::wstring str_file_name = str_url.substr(1, str_url.size() - 1);
		ENSUREK(!str_file_name.empty());

		size_t file_size = 0;
		if (sp_http_session->get_respond_header()->_total_length)
		{
			file_size = *sp_http_session->get_respond_header()->_total_length;
		}
		else
		{
			if (sp_http_session->get_respond_header()->_content_length)
			{
				file_size = *sp_http_session->get_respond_header()->_content_length;
			}
		}
		ENSUREK(file_size != 0);
		boost::shared_ptr<http_download_session> sp_http_download_session = http_download_session::create(sp_http_session->get_network_link_info(), str_file_name, file_size);
		ENSUREK(sp_http_download_session);
		sp_k_http_download_session = boost::shared_ptr<k_http_download_session>(new k_http_download_session);
		sp_k_http_download_session->_sp_http_download_session = sp_http_download_session;
		sp_k_http_download_session->_last_time = boost::posix_time::microsec_clock::local_time();
	} while (false);
	return sp_k_http_download_session;
}

bool k_http_download_session::process(boost::shared_ptr<http_session> sp_incomplete_http_session)
{
	ENSUREf(!_b_completed);
	size_t data_begin = 0;
	
	if (sp_incomplete_http_session->get_respond_header()->_range_begin)
	{
		data_begin = *sp_incomplete_http_session->get_respond_header()->_range_begin;
	}
	size_t data_end = data_begin + sp_incomplete_http_session->get_respond_body()->size();
	
	util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "fragment come[%d-%d] <%s>", data_begin, data_end, _sp_http_download_session->getinfostring().c_str());

	boost::shared_ptr<file_data_fragment> sp_left_fragment;
	boost::shared_ptr<file_data_fragment> sp_right_fragment;
	std::map<size_t, boost::shared_ptr<file_data_fragment> > map_between_fragment;
	for (std::map<size_t, boost::shared_ptr<file_data_fragment> >::iterator it = _map_begin_fragment.begin(); it != _map_begin_fragment.end(); it++)
	{
		boost::shared_ptr<file_data_fragment> sp_fragment = it->second;
		if (data_begin >= sp_fragment->_begin && data_begin <= sp_fragment->_end)
		{
			sp_left_fragment = sp_fragment;
		}
		if (data_end >= sp_fragment->_begin && data_end <= sp_fragment->_end)
		{
			sp_right_fragment = sp_fragment;
			break;
		}
		if (sp_fragment->_begin >= data_begin && sp_fragment->_end <= data_end)
		{
			if (sp_left_fragment != sp_fragment)
			{
				map_between_fragment.insert(std::make_pair(it->first, it->second));
			}
		}
	}

	if (sp_left_fragment && sp_right_fragment)
	{
		if (sp_left_fragment != sp_right_fragment)
		{
			util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "fragment fix[%d-%d]--->[%d-%d] <%s>", sp_left_fragment->_begin, sp_left_fragment->_end, sp_left_fragment->_begin, sp_right_fragment->_end, _sp_http_download_session->getinfostring().c_str());
			sp_left_fragment->_end = sp_right_fragment->_end;
			
			map_between_fragment.insert(std::make_pair(sp_right_fragment->_org_begin, sp_right_fragment));
		}
		else
		{
			util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "fragment exist[%d-%d] in [%d-%d] <%s>", data_begin, data_end, sp_left_fragment->_begin, sp_left_fragment->_end, _sp_http_download_session->getinfostring().c_str());
		}
	}
	else if (!sp_left_fragment && !sp_right_fragment)
	{
		_map_begin_fragment.insert(std::make_pair(data_begin, boost::shared_ptr<file_data_fragment>(new file_data_fragment(data_begin, data_end))));
		util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "fragment insert[%d-%d] <%s>", data_begin, data_end, _sp_http_download_session->getinfostring().c_str());
	}
	else if (sp_left_fragment && !sp_right_fragment)
	{
		util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "fragment fix[%d-%d]--->[%d-%d] <%s>", sp_left_fragment->_begin, sp_left_fragment->_end, sp_left_fragment->_begin, data_end, _sp_http_download_session->getinfostring().c_str());
		sp_left_fragment->_end = data_end;
	}
	else if (!sp_left_fragment && sp_right_fragment)
	{
		util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "fragment fix[%d-%d]--->[%d-%d] <%s>", sp_right_fragment->_begin, sp_right_fragment->_end, data_begin, sp_right_fragment->_end, _sp_http_download_session->getinfostring().c_str());
		sp_right_fragment->_begin = data_begin;
	}
	else
	{
		Assert(false);
	}
	_sp_http_download_session->write_file_data(data_begin, sp_incomplete_http_session->get_respond_body()->data(), sp_incomplete_http_session->get_respond_body()->size());
	for (std::map<size_t, boost::shared_ptr<file_data_fragment> >::iterator it = map_between_fragment.begin(); it != map_between_fragment.end(); it++)
	{
		boost::shared_ptr<file_data_fragment> sp_fragment = it->second;
		std::map<size_t, boost::shared_ptr<file_data_fragment> >::iterator it_erase = _map_begin_fragment.find(it->first);
		Assert(it_erase != _map_begin_fragment.end());
		_map_begin_fragment.erase(it_erase);
		util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "fragment remove[%d-%d] <%s>", sp_fragment->_begin, sp_fragment->_end, _sp_http_download_session->getinfostring().c_str());
	}
	util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "-----------current fragment list begin--------------");
	for (std::map<size_t, boost::shared_ptr<file_data_fragment> >::iterator it = _map_begin_fragment.begin(); it != _map_begin_fragment.end(); it++)
	{
		boost::shared_ptr<file_data_fragment> sp_fragment = it->second;
		util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "[%d-%d] <%s>", sp_fragment->_begin, sp_fragment->_end, _sp_http_download_session->getinfostring().c_str());
	}
	util_log::logd(HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG, "-----------current fragment list end--------------\n");
	ENSUREf(_map_begin_fragment.size() == 1);
	boost::shared_ptr<file_data_fragment> sp_the_fragment = _map_begin_fragment.begin()->second;
	ENSUREf(sp_the_fragment->_begin == 0 && sp_the_fragment->_end == _sp_http_download_session->get_file_size());
	_b_completed = true;
	return true;
}

void k_http_download_session::update()
{
	_last_time = boost::posix_time::microsec_clock::local_time();
}

bool k_http_download_session::is_expire()
{
	boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time();
	std::string str_elapsed_time = boost::posix_time::to_iso_string(current_time - _last_time);
	double d = util_string::a2d(str_elapsed_time);
	unsigned int elapsed_second = (unsigned int)d;
	ENSUREf(elapsed_second > config::instance()->get_http_download_session_expire_time());
	return true;
}

boost::shared_ptr<http_download_session> k_http_download_session::get_http_download_session()
{
	return _sp_http_download_session;
}
