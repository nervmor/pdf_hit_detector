#include "http_download_session.h"


boost::shared_ptr<http_download_session> http_download_session::create(const network_link_info& link_info, const std::wstring& str_filename, size_t file_size)
{
	Assert(!str_filename.empty());
	Assert(file_size != 0);
	boost::shared_ptr<http_download_session> sp_http_download_session(new http_download_session);
	sp_http_download_session->m_str_server_ip = link_info._str_server_ip;
	sp_http_download_session->m_server_port = link_info._server_port;
	sp_http_download_session->m_str_downloader_ip = link_info._str_client_ip;
	sp_http_download_session->m_str_filename = str_filename;
	sp_http_download_session->m_file_size = file_size;
	boost::shared_ptr<util_buffer> sp_file_data(new util_buffer);
	Assert(sp_file_data->create(file_size));
	sp_http_download_session->m_file_data = sp_file_data;
	return sp_http_download_session;
}

void http_download_session::write_file_data(size_t offset, const unsigned char* data, size_t size)
{
	m_file_data->write(data, size, offset);
}

std::string http_download_session::getinfostring()
{
	char info[4096];
	sprintf(info, "[http_download_session] filename:[%S]--filesize[%d]--downloader_ip[%S]--server_ip[%S]", get_file_name().c_str(), get_file_size(), get_downloader_ip().c_str(), get_server_ip().c_str());
	return std::string(info);
}

