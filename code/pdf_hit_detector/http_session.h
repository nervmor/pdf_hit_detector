#pragma once

#include "../frame/include.h"
#include "../frame/util_buffer.h"
#include "common.h"


class http_session : public boost::noncopyable
{
public:
	struct http_request : public boost::noncopyable
	{
		struct header 
		{
			std::wstring _str_url;
			std::wstring _str_useragent;
			boost::optional<unsigned int> _range_begin;
			boost::optional<unsigned int> _range_end;
		};

		boost::shared_ptr<header> _sp_header;
		boost::shared_ptr<util_buffer> _sp_body;
	};

	struct http_respond : public boost::noncopyable
	{
		struct header
		{
			std::wstring _str_respond_code;
			boost::optional<unsigned int> _content_length;
			boost::optional<unsigned int> _range_begin;
			boost::optional<unsigned int> _range_end;
			boost::optional<unsigned int> _total_length;
		};
		
		boost::shared_ptr<header> _sp_header;
		boost::shared_ptr<util_buffer> _sp_body;
	};
public:
	static boost::shared_ptr<http_session> create(const network_link_info& link_info);
	network_link_info get_network_link_info(){return m_network_link_info;}
	boost::shared_ptr<http_request::header> get_request_header(){return m_request._sp_header;}
	boost::shared_ptr<util_buffer> get_request_body(){return m_request._sp_body;}
	boost::shared_ptr<http_respond::header> get_respond_header(){return m_respond._sp_header;}
	boost::shared_ptr<util_buffer> get_respond_body(){return m_respond._sp_body;}
public:
	void set_request_header(boost::shared_ptr<http_request::header> sp_header){m_request._sp_header = sp_header;}
	void set_request_body(boost::shared_ptr<util_buffer> sp_body){m_request._sp_body = sp_body;}
	void set_respond_header(boost::shared_ptr<http_respond::header> sp_header){m_respond._sp_header = sp_header;}
	void set_respond_body(boost::shared_ptr<util_buffer> sp_body){m_respond._sp_body = sp_body;}
private:
	network_link_info m_network_link_info;
	http_request m_request;
	http_respond m_respond;
private:
	http_session(const network_link_info& link_info)
		: m_network_link_info(link_info)
	{

	}
};