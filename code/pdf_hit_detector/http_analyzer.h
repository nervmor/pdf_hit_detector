#pragma once

#include <boost/thread.hpp>
#include "../frame/util_buffer.h"
#include "../third/http-parser-master/http_parser.h"
#include "../frame/util_cache.h"
#include "http_session.h"
#include "common.h"

class http_analyzer_base : public boost::noncopyable
{
protected:
	void initialize();
public:
	void uninitialize();
	common_result analyze(boost::shared_ptr<util_buffer> sp_data_buffer);
	void breakoff();
private:
	virtual int http_data_cb_on_header_field(http_parser* p_http_parser, const char *at, size_t length);
	virtual int http_data_cb_on_header_value(http_parser* p_http_parser, const char *at, size_t length);
	virtual int http_data_cb_on_body(http_parser* p_http_parser, const char *at, size_t length);
	virtual int http_data_cb_on_headers_complete(http_parser* p_http_parser);
	virtual int http_data_cb_on_message_begin(http_parser* p_http_parser);
	virtual int http_data_cb_on_message_complete(http_parser* p_http_parser);
	virtual int http_data_cb_on_status(http_parser* p_http_parser, const char *at, size_t length);
	virtual int http_data_cb_on_url(http_parser* p_http_parser, const char *at, size_t length);
private:
	virtual enum http_parser_type get_type() = 0;
	virtual bool on_begin() = 0;
	virtual void on_complete() = 0;
	virtual void on_error(boost::shared_ptr<util_buffer> sp_body) = 0;
	virtual bool on_header_field_value(const std::wstring& str_field, const std::wstring& str_value) = 0;
	virtual bool on_headers_complete() = 0;
	virtual void on_body_complete(boost::shared_ptr<util_buffer> sp_body) = 0;
	virtual bool on_url(const std::wstring& str_url) = 0;
	virtual bool on_status(const std::wstring& str_status) = 0;
private:
	static int _http_data_cb_on_body(http_parser* p_http_parser, const char *at, size_t length){return get_this(p_http_parser)->http_data_cb_on_body(p_http_parser, at, length);}
	static int _http_data_cb_on_header_field(http_parser* p_http_parser, const char *at, size_t length){return get_this(p_http_parser)->http_data_cb_on_header_field(p_http_parser, at, length);}
	static int _http_data_cb_on_header_value(http_parser* p_http_parser, const char *at, size_t length){return get_this(p_http_parser)->http_data_cb_on_header_value(p_http_parser, at, length);}
	static int _http_data_cb_on_headers_complete(http_parser* p_http_parser){return get_this(p_http_parser)->http_data_cb_on_headers_complete(p_http_parser);}
	static int _http_data_cb_on_message_begin(http_parser* p_http_parser){return get_this(p_http_parser)->http_data_cb_on_message_begin(p_http_parser);}
	static int _http_data_cb_on_message_complete(http_parser* p_http_parser){return get_this(p_http_parser)->http_data_cb_on_message_complete(p_http_parser);}
	static int _http_data_cb_on_status(http_parser* p_http_parser, const char *at, size_t length){return get_this(p_http_parser)->http_data_cb_on_status(p_http_parser, at, length);}
	static int _http_data_cb_on_url(http_parser* p_http_parser, const char *at, size_t length){return get_this(p_http_parser)->http_data_cb_on_url(p_http_parser, at, length);}
private:
	static http_analyzer_base* get_this(http_parser* p_http_parser);
private:
	void set_this();
	void unset_this();
private:
	struct tls_data
	{
		std::map<http_parser*, http_analyzer_base*> _map_parser_analyzer;
	};
	enum State
	{
		state_analyze_ready,
		state_analyze_header,
		state_analyze_body,
		state_analyze_complete
	};
private:
	static boost::thread_specific_ptr<tls_data> s_tls_data_ptr;
	static http_parser_settings s_parser_setting;
private:
	http_parser m_parser;
	State m_state;
private:
	// 使用缓存的机制 因为http数据流可能有截断造成数据不完整(类似TCP粘包) 
	util_cache m_body_cache;
	std::wstring m_str_field_cache;
	std::wstring m_str_value_cache;
	std::wstring m_str_url_cache;
	std::wstring m_str_status_cache;
	std::map<std::wstring, std::wstring> m_map_field_value;
public:
	http_analyzer_base()
		: m_state(state_analyze_ready)
	{

	}
	virtual ~http_analyzer_base()
	{
		uninitialize();
	}
};

class http_request_analyzer : public http_analyzer_base
{
public:
	class notifyer
	{
	public:
		virtual ~notifyer(){}
		virtual bool on_http_request_header(boost::shared_ptr<http_session::http_request::header> sp_request_header) = 0;
		virtual void on_http_request_complete(const http_session::http_request& request) = 0;
		virtual void on_http_request_error(const http_session::http_request& request) = 0;
	};
public:
	virtual enum http_parser_type get_type();
	virtual bool on_begin();
	virtual void on_complete();
	virtual void on_error(boost::shared_ptr<util_buffer> sp_body);
	virtual bool on_header_field_value(const std::wstring& str_field, const std::wstring& str_value);
	virtual bool on_headers_complete();
	virtual void on_body_complete(boost::shared_ptr<util_buffer> sp_body);
	virtual bool on_url(const std::wstring& str_url);
	virtual bool on_status(const std::wstring& str_status);
public:
	void initialize(notifyer* p_notifyer);
public:
	notifyer* m_p_notifyer;
	http_session::http_request m_http_request;
public:
	http_request_analyzer()
		: http_analyzer_base()
		, m_p_notifyer(NULL)
	{

	}
};


class http_respond_analyzer : public http_analyzer_base
{
public:
	class notifyer
	{
	public:
		virtual ~notifyer(){}
		virtual bool on_http_respond_header(boost::shared_ptr<http_session::http_respond::header> sp_respond_header) = 0;
		virtual void on_http_respond_complete(const http_session::http_respond& respond) = 0;
		virtual void on_http_respond_error(const http_session::http_respond& respond) = 0;
	};
public:
	virtual enum http_parser_type get_type();
	virtual bool on_begin();
	virtual void on_complete();
	virtual void on_error(boost::shared_ptr<util_buffer> sp_body);
	virtual bool on_header_field_value(const std::wstring& str_field, const std::wstring& str_value);
	virtual bool on_headers_complete();
	virtual void on_body_complete(boost::shared_ptr<util_buffer> sp_body);
	virtual bool on_url(const std::wstring& str_url);
	virtual bool on_status(const std::wstring& str_status);
public:
	void initialize(notifyer* p_notifyer);
private:
	notifyer* m_p_notifyer;
	http_session::http_respond m_http_respond;
public:
	http_respond_analyzer()
		: http_analyzer_base()
		, m_p_notifyer(NULL)
	{

	}
};

class http_checker
{
public:
	static bool is_valid_http_request(boost::shared_ptr<util_buffer> sp_data_buffer);
};