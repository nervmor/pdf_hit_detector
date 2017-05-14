#pragma once

#include "interface.h"
#include "../frame/singleton.h"
#include "http_download_session.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/threadpool/threadpool.hpp>

struct k_http_download_session : public boost::noncopyable
{
	static boost::shared_ptr<k_http_download_session> create(boost::shared_ptr<http_session> sp_incomplete_http_session);

	bool process(boost::shared_ptr<http_session> sp_incomplete_http_session);
	void update();
	bool is_expire();
	boost::shared_ptr<http_download_session> get_http_download_session();
private:
	struct file_data_fragment
	{
		size_t _begin;
		size_t _end;

		size_t _org_begin;
		size_t _org_end;

		file_data_fragment(size_t begin, size_t end)
			: _begin(begin)
			, _end(end)
			, _org_begin(begin)
			, _org_end(end)
		{
		}
	};
	std::map<size_t, boost::shared_ptr<file_data_fragment> > _map_begin_fragment;
	boost::shared_ptr<http_download_session> _sp_http_download_session;
	boost::posix_time::ptime _last_time;
private:
	bool _b_completed;
private:
	k_http_download_session()
		: _last_time(boost::posix_time::microsec_clock::local_time())
		, _b_completed(false){}
private:
	SINLETON_CLASS_DECLARE(k_http_download_session);
};

class http_download_session_mgr : public http_session_mgr_notifyer
{
public:
	virtual notifyer_result on_http_session_request_header(boost::shared_ptr<http_session> sp_http_session);
	virtual notifyer_result on_http_session_respond_header(boost::shared_ptr<http_session> sp_http_session);
	virtual void on_http_session_error(boost::shared_ptr<http_session> sp_http_session);
	virtual void on_http_session_complete(boost::shared_ptr<http_session> sp_http_session);
public:
	bool initialize();
	void uninitialize();
	void add_notifyer(http_download_session_mgr_notifyer* p_notifyer);
private:
	void _put_completed_http_session(boost::shared_ptr<http_session> sp_http_session);
	void _put_error_http_session(boost::shared_ptr<http_session> sp_http_session);

	void _report_http_download_session(boost::shared_ptr<k_http_download_session> sp_k_http_download_session);
	void _remove_k_http_download_session(boost::shared_ptr<k_http_download_session> sp_k_http_download_session);
	void _clean_expire_k_http_download_session();
private:
	void _check_expire_k_http_download_session();
private:
	void _process_integral_http_session(boost::shared_ptr<http_session> sp_http_session);
	void _process_incomplete_http_session(boost::shared_ptr<k_http_download_session> sp_k_http_download_session, boost::shared_ptr<http_session> sp_http_session);
private:
	void _notify_integral_http_download_session(boost::shared_ptr<http_download_session> sp_http_session);
	void _notify_incomplete_http_download_session(boost::shared_ptr<http_download_session> sp_http_session);
private:
	unsigned int hash_1(boost::shared_ptr<http_session> sp_http_session);
	unsigned int hash_2(boost::shared_ptr<http_session> sp_http_session);
private:
	boost::mutex m_dispatch_mutex;
	boost::threadpool::pool m_dispatch_threadpool;
	// 2¼¶hashË÷Òý´æ·Åk_http_download_session
	std::map<unsigned int, std::map<unsigned int, boost::shared_ptr<k_http_download_session> > > m_map_hash_k_http_download_session;
	std::map<boost::shared_ptr<k_http_download_session>, std::pair<unsigned int, unsigned int> > m_map_k_http_download_session_hash;
	unsigned int m_last_clean_time;
private:
	boost::threadpool::pool m_process_threadpool;
	boost::threadpool::pool m_notifyer_threadpool;
	boost::threadpool::pool m_clean_threadpool;
private:
	std::set<http_download_session_mgr_notifyer*> m_set_notifyer;
public:
	http_download_session_mgr()
		: m_dispatch_threadpool(1)
		, m_process_threadpool(1)
		, m_notifyer_threadpool(1)
		, m_clean_threadpool(1)
	{

	}
};