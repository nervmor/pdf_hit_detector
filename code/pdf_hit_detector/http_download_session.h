#pragma once


#include "../frame/include.h"
#include "../frame/util_buffer.h"
#include "common.h"
class http_download_session
{
public:
	static boost::shared_ptr<http_download_session> create(const network_link_info& link_info, const std::wstring& str_filename, size_t file_size);
public:
	std::wstring get_downloader_ip(){return m_str_downloader_ip;}
	std::wstring get_server_ip(){return m_str_server_ip;}
	std::wstring get_file_name(){return m_str_filename;}
	size_t get_file_size(){return m_file_size;}
	boost::shared_ptr<util_buffer> get_file_data(){return m_file_data;}
public:
	void write_file_data(size_t offset, const unsigned char* data, size_t size);
	std::string getinfostring();
private:
	std::wstring m_str_downloader_ip;
	std::wstring m_str_server_ip;
	unsigned int m_server_port;
	std::wstring m_str_filename;
	size_t m_file_size;
	boost::shared_ptr<util_buffer> m_file_data;
private:
	http_download_session()
		: m_server_port(0)
		, m_file_size(0)
	{

	}
};