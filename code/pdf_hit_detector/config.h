#pragma once


#include "../frame/singleton.h"
#include "define.h"
#include "../third/ini_config/ini_config.h"
#include "../frame/util_log.h"
#include "../frame/util_string.h"
#include "../frame/util_path.h"

class config : public singleton<config>
{
public:
	bool initialize()
	{
		char sz_config_file[4096];
		std::wstring str_config_file = util_path::get_image_dir() + CONFIG_FILE_NAME;
		strcpy(sz_config_file, util_string::w2a(str_config_file).c_str());

		bool result = false;
		do 
		{
			FILE* p_config_file = fopen(sz_config_file, "r");
			if (p_config_file == NULL)
			{
				util_log::log(CONFIG_LOG_TAG, "config file[%s] is not exist...", sz_config_file);
				break;
			}
			fclose(p_config_file);
			char* p_value = NULL;
			p_value = GetIniKeyString("main", "db_ip", sz_config_file);
			if (p_value == "")
			{
				util_log::log(CONFIG_LOG_TAG, "db_ip is not valid...");
				break;
			}
			m_str_db_ip = util_string::a2w(p_value);
			p_value = GetIniKeyString("main", "db_user", sz_config_file);
			if (p_value == "")
			{
				util_log::log(CONFIG_LOG_TAG, "db_user is not valid...");
				break;
			}
			m_str_db_user = util_string::a2w(p_value);
			p_value = GetIniKeyString("main", "db_pswd", sz_config_file);
			if (p_value == "")
			{
				util_log::log(CONFIG_LOG_TAG, "db_pswd is not valid...");
				break;
			}
			m_str_db_pswd = util_string::a2w(p_value);		
			p_value = GetIniKeyString("main", "cache_expire_time", sz_config_file);
			if (p_value != "")
			{
				m_http_download_session_expire_time = util_string::a2i(p_value);
			}
			p_value = GetIniKeyString("main", "pre_hash_size", sz_config_file);
			if (p_value != "")
			{
				m_pre_hash_size = util_string::a2i(p_value);
			}
			result = true;
		} while (false);
		ENSUREf(result);
		return true;
	}
public:
	size_t get_min_pdf_file_size(){return m_min_pdf_file_size;}
	unsigned int get_http_download_session_expire_time(){return m_http_download_session_expire_time;}
	unsigned int get_pre_hash_size(){return m_pre_hash_size;}
	std::wstring get_db_ip(){return m_str_db_ip;}
	std::wstring get_db_user(){return m_str_db_user;}
	std::wstring get_db_pswd(){return m_str_db_pswd;}
private:
	size_t m_min_pdf_file_size;
	unsigned int m_http_download_session_expire_time;
	unsigned int m_pre_hash_size;
	std::wstring m_str_db_ip;
	std::wstring m_str_db_user;
	std::wstring m_str_db_pswd;
private:
	config()
		: m_min_pdf_file_size(HTTP_DOWNLOAD_SESSION_DEFAULT_MIN_CONTENT_LENGTH)
		, m_http_download_session_expire_time(HTTP_DOWNLOAD_SESSION_DEFAULT_EXPIRE_TIME)
		, m_pre_hash_size(PRE_HASH_DEFAULT_SIZE)
	{

	}
private:
	SINLETON_CLASS_DECLARE(config);
};