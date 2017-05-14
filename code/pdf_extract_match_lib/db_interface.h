#pragma once

#include "../frame/include.h"
#include "mysql_adapter.h"

class db_interface
{
public:
	bool initialize(const std::wstring& str_db_ip, const std::wstring& str_db_user, const std::wstring& str_db_pswd, const std::wstring& str_db_name);
	void uninitialize();

	bool exec_sql(const char* sz_sql);
	bool query_sql(const char* sz_sql, std::vector<std::vector<std::string> >& vec_query_result);
private:
	mysql_adapter m_mysql;
};