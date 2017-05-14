#include "http_analyzer.h"
#include "../frame/util_string.h"
#include "../frame/util_log.h"
#include "define.h"

void http_analyzer_base::initialize()
{
	http_parser_init(&m_parser, get_type());
	set_this();
}

void http_analyzer_base::uninitialize()
{
	unset_this();
}

common_result http_analyzer_base::analyze(boost::shared_ptr<util_buffer> sp_data_buffer)
{
	Assert(m_state != state_analyze_complete);
	ENSURE(m_state != state_analyze_complete, common_result_error);
	common_result result = common_result_success;
	size_t parser_size = sp_data_buffer->size();
	if (parser_size != http_parser_execute(&m_parser, &s_parser_setting, sp_data_buffer->data<char>(), parser_size))
	{
		on_error(m_body_cache.extract());
		m_state = state_analyze_complete;
		result = common_result_error;
	}
	else
	{
		if (m_state == state_analyze_complete)
		{
			result = common_result_complete;
		}
	}
	return result;
}

void http_analyzer_base::breakoff()
{
	ENSURET(m_state != state_analyze_complete);
	on_error(m_body_cache.extract());
	m_state = state_analyze_complete;
}



void http_analyzer_base::set_this()
{
	if (s_tls_data_ptr.get() == NULL)
	{
		s_tls_data_ptr.reset(new tls_data);
	}
	s_tls_data_ptr->_map_parser_analyzer.insert(std::make_pair(&m_parser, this));
}

http_analyzer_base* http_analyzer_base::get_this(http_parser* p_http_parser)
{
	Assert(s_tls_data_ptr.get() != NULL);
	std::map<http_parser*, http_analyzer_base*>::iterator it = s_tls_data_ptr->_map_parser_analyzer.find(p_http_parser);
	Assert(it != s_tls_data_ptr->_map_parser_analyzer.end());
	http_analyzer_base* p_this = it->second;
	return p_this;
}

void http_analyzer_base::unset_this()
{
	ENSURET(s_tls_data_ptr.get() != NULL);
	std::map<http_parser*, http_analyzer_base*>::iterator it = s_tls_data_ptr->_map_parser_analyzer.find(&m_parser);
	ENSURET(it != s_tls_data_ptr->_map_parser_analyzer.end());
	s_tls_data_ptr->_map_parser_analyzer.erase(it);
}

int http_analyzer_base::http_data_cb_on_header_field(http_parser* p_http_parser, const char *at, size_t length)
{
	if (!m_str_field_cache.empty() && !m_str_value_cache.empty())
	{
		m_map_field_value.insert(std::make_pair(m_str_field_cache, m_str_value_cache));
		if (!on_header_field_value(m_str_field_cache, m_str_value_cache))
		{
			return -1;
		}
		m_str_field_cache.clear();
		m_str_value_cache.clear();
	}
	boost::scoped_array<char> sp_field_name(new char[length + 1]);
	memcpy(sp_field_name.get(), at, length);
	sp_field_name[length] = '\0';
	std::string str_filed_name = sp_field_name.get();
	m_str_field_cache += util_string::a2w(str_filed_name);
	return 0;
}

int http_analyzer_base::http_data_cb_on_header_value(http_parser* p_http_parser, const char *at, size_t length)
{
	boost::scoped_array<char> sp_field_value(new char[length + 1]);
	memcpy(sp_field_value.get(), at, length);
	sp_field_value[length] = '\0';
	m_str_value_cache += util_string::a2w(sp_field_value.get());
	return 0;
}

int http_analyzer_base::http_data_cb_on_body(http_parser* p_http_parser, const char *at, size_t length)
{
	m_state = state_analyze_body;
	if (length != 0)
	{
		boost::shared_ptr<util_buffer> sp_buffer(new util_buffer);
		if (!sp_buffer->create(length, at))
		{
			return -1;
		}
		m_body_cache.put(sp_buffer);
	}
	return 0;
}

int http_analyzer_base::http_data_cb_on_headers_complete(http_parser* p_http_parser)
{
	if (!m_str_field_cache.empty() && !m_str_value_cache.empty())
	{
		m_map_field_value.insert(std::make_pair(m_str_field_cache, m_str_value_cache));
		if (!on_header_field_value(m_str_field_cache, m_str_value_cache))
		{
			return -1;
		}
		m_str_field_cache.clear();
		m_str_value_cache.clear();
	}
	if (!m_str_url_cache.empty())
	{
		if (!on_url(m_str_url_cache))
		{
			return -1;
		}
		m_str_url_cache.clear();
	}
	if (!m_str_status_cache.empty())
	{
		if (!on_status(m_str_status_cache))
		{
			return -1;
		}
		m_str_status_cache.clear();
	}
	if (!on_headers_complete())
	{
		return -1;
	}
	return 0;
}

int http_analyzer_base::http_data_cb_on_message_begin(http_parser* p_http_parser)
{
	m_state = state_analyze_header;
	if (!on_begin())
	{
		return -1;
	}
	return 0;
}


int http_analyzer_base::http_data_cb_on_message_complete(http_parser* p_http_parser)
{
	m_state = state_analyze_complete;

	if (!m_body_cache.empty())
	{
		on_body_complete(m_body_cache.extract());
	}
	on_complete();
	return 0;
}

int http_analyzer_base::http_data_cb_on_status(http_parser* p_http_parser, const char *at, size_t length)
{
	boost::scoped_array<char> sp_status(new char[length + 1]);
	memcpy(sp_status.get(), at, length);
	sp_status[length] = '\0';

	m_str_status_cache += util_string::a2w(sp_status.get());
	return 0;
}

int http_analyzer_base::http_data_cb_on_url(http_parser* p_http_parser, const char *at, size_t length)
{
	boost::scoped_array<char> sp_url(new char[length + 1]);
	memcpy(sp_url.get(), at, length);
	sp_url[length] = '\0';
	m_str_url_cache += util_string::a2w(sp_url.get());
	return 0;
}






boost::thread_specific_ptr<http_analyzer_base::tls_data> http_analyzer_base::s_tls_data_ptr;

http_parser_settings http_analyzer_base::s_parser_setting = 
{
	&http_analyzer_base::_http_data_cb_on_message_begin,
	&http_analyzer_base::_http_data_cb_on_url,
	&http_analyzer_base::_http_data_cb_on_status,
	&http_analyzer_base::_http_data_cb_on_header_field,
	&http_analyzer_base::_http_data_cb_on_header_value,
	&http_analyzer_base::_http_data_cb_on_headers_complete,
	&http_analyzer_base::_http_data_cb_on_body,
	&http_analyzer_base::_http_data_cb_on_message_complete
};










void http_request_analyzer::initialize(notifyer* p_notifyer)
{
	m_p_notifyer = p_notifyer;
	http_analyzer_base::initialize();
}

enum http_parser_type http_request_analyzer::get_type()
{
	return HTTP_REQUEST;
}

bool http_request_analyzer::on_begin()
{
	m_http_request._sp_header = boost::shared_ptr<http_session::http_request::header>(new http_session::http_request::header);
	return true;
}

void http_request_analyzer::on_complete()
{
	m_p_notifyer->on_http_request_complete(m_http_request);
}

bool http_request_analyzer::on_header_field_value(const std::wstring& str_field, const std::wstring& str_value)
{
	util_log::logd(HTTP_ANALYZER_LOG_TAG, "request header[%S--%S]", str_field.c_str(), str_value.c_str());
	std::wstring str_filed_low = util_string::low(str_field);
	std::wstring str_value_low = util_string::low(str_value);
	if (str_filed_low == L"range")
	{
		std::wstring str_sign = L"bytes=";
		ENSUREf(0 == str_value_low.compare(0, str_sign.size(), str_sign));
		size_t pos = str_value_low.find(L"-");
		ENSUREf(pos != std::wstring::npos);
		unsigned int range_begin = util_string::w2i(str_value_low.substr(str_sign.size(), pos - str_sign.size()));
		m_http_request._sp_header->_range_begin = boost::make_optional(range_begin);
		if (pos != str_value_low.size() - 1)
		{
			unsigned int range_end = util_string::w2i(str_value_low.substr(pos + 1, str_value_low.size() - pos));
			m_http_request._sp_header->_range_end = boost::make_optional(range_end);
		}
	}
	else if (str_filed_low == L"user-agent")
	{
		m_http_request._sp_header->_str_useragent = str_value_low;
	}
	else
	{

	}
	return true;
}

bool http_request_analyzer::on_headers_complete()
{
	ENSUREf(m_p_notifyer->on_http_request_header(m_http_request._sp_header));
	return true;
}

void http_request_analyzer::on_body_complete(boost::shared_ptr<util_buffer> sp_body)
{
	util_log::logd(HTTP_ANALYZER_LOG_TAG, "request body_size[%d]", sp_body->size());
	m_http_request._sp_body = sp_body;
}

bool http_request_analyzer::on_url(const std::wstring& str_url)
{
	util_log::logd(HTTP_ANALYZER_LOG_TAG, "request url[%S]", str_url.c_str());
	m_http_request._sp_header->_str_url = str_url;
	return true;
}

bool http_request_analyzer::on_status(const std::wstring& str_status)
{
	util_log::logd(HTTP_ANALYZER_LOG_TAG, "request status[%S]", str_status.c_str());
	return true;
}

void http_request_analyzer::on_error(boost::shared_ptr<util_buffer> sp_body)
{
	m_http_request._sp_body = sp_body;
	m_p_notifyer->on_http_request_error(m_http_request);
}




void http_respond_analyzer::initialize(notifyer* p_notifyer)
{
	m_p_notifyer = p_notifyer;
	http_analyzer_base::initialize();
}


enum http_parser_type http_respond_analyzer::get_type()
{
	return HTTP_RESPONSE;
}

bool http_respond_analyzer::on_begin()
{
	m_http_respond._sp_header = boost::shared_ptr<http_session::http_respond::header>(new http_session::http_respond::header);
	return true;
}

void http_respond_analyzer::on_complete()
{
	m_p_notifyer->on_http_respond_complete(m_http_respond);
}

bool http_respond_analyzer::on_header_field_value(const std::wstring& str_field, const std::wstring& str_value)
{
	util_log::logd(HTTP_ANALYZER_LOG_TAG, "respond header[%S--%S]", str_field.c_str(), str_value.c_str());
	std::wstring str_filed_low = util_string::low(str_field);
	std::wstring str_value_low = util_string::low(str_value);
	if (str_filed_low == L"content-length")
	{
		m_http_respond._sp_header->_content_length = boost::make_optional(util_string::w2i(str_value_low));
	}
	else if (str_filed_low == L"content-range")
	{
		std::wstring str_sign = L"bytes ";
		ENSUREf(0 == str_value_low.compare(0, str_sign.size(), str_sign));
		size_t pos_1 = str_value_low.find(L"-");
		ENSUREf(pos_1 != std::wstring::npos);
		size_t pos_2 = str_value_low.find(L"/");
		ENSUREf(pos_2 != std::wstring::npos);

		unsigned int range_begin = util_string::w2i(str_value_low.substr(str_sign.size(), pos_1 - str_sign.size()));
		m_http_respond._sp_header->_range_begin = boost::make_optional(range_begin);
	
		unsigned int range_end = util_string::w2i(str_value_low.substr(pos_1 + 1, pos_2 - pos_1 - 1));
		m_http_respond._sp_header->_range_end = boost::make_optional(range_end);

		unsigned int total_length = util_string::w2i(str_value_low.substr(pos_2 + 1, str_value_low.size() - pos_2 - 1));
		m_http_respond._sp_header->_total_length = boost::make_optional(total_length);
	}
	else
	{

	}
	return true;
}

bool http_respond_analyzer::on_headers_complete()
{
	ENSUREf(m_p_notifyer->on_http_respond_header(m_http_respond._sp_header));
	return true;
}

void http_respond_analyzer::on_body_complete(boost::shared_ptr<util_buffer> sp_body)
{
	util_log::logd(HTTP_ANALYZER_LOG_TAG, "respond body_size[%d]", sp_body->size());
	m_http_respond._sp_body = sp_body;
}

bool http_respond_analyzer::on_url(const std::wstring& str_url)
{
	util_log::logd(HTTP_ANALYZER_LOG_TAG, "respond url[%S]", str_url.c_str());
	return true;
}

bool http_respond_analyzer::on_status(const std::wstring& str_status)
{
	util_log::logd(HTTP_ANALYZER_LOG_TAG, "respond status[%S]", str_status.c_str());
	m_http_respond._sp_header->_str_respond_code = str_status;
	return true;
}

void http_respond_analyzer::on_error(boost::shared_ptr<util_buffer> sp_body)
{
	m_http_respond._sp_body = sp_body;
	m_p_notifyer->on_http_respond_error(m_http_respond);
}













bool http_checker::is_valid_http_request(boost::shared_ptr<util_buffer> sp_data_buffer)
{
	http_parser request_parser;
	http_parser_init(&request_parser, HTTP_REQUEST);
	size_t parser_size = sp_data_buffer->size();
	http_parser_settings settings = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	ENSUREf(parser_size == http_parser_execute(&request_parser, &settings, sp_data_buffer->data<char>(), parser_size));
	return true;
}
