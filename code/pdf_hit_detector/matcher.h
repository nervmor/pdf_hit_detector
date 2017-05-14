#pragma once

#include "interface.h"
#include "../pdf_extract_match_lib/db_interface.h"

class pdf_hit_decector_matcher : public http_download_session_mgr_notifyer, public boost::noncopyable
{
public:
	virtual void on_integral_http_download_session(boost::shared_ptr<http_download_session> sp_http_download_session);
	virtual void on_incomplete_http_download_session(boost::shared_ptr<http_download_session> sp_http_download_session);
public:
	bool initialize();
	void uninitialize();
	bool match(const std::wstring& str_file_name, std::wstring& str_pdf_description);
	bool match_pre(boost::shared_ptr<util_buffer> sp_file_data, std::wstring& str_pdf_description);
private:
	bool pdf_weak_check(boost::shared_ptr<util_buffer> sp_file_data);
private:
	db_interface m_db;
};