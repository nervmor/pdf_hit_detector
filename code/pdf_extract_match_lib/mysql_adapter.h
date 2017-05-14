#pragma once

#include "../frame/include.h"
#include <boost/thread/thread.hpp>
#ifdef _WIN32
#include "..\third\mysql\mysql\mysql.h"
#else
#include <mysql/mysql.h>
#endif

class mysql_adapter
{
public:
	bool init(const char* sz_db_ip, const char* sz_user, const char* sz_pswd, const char* sz_db_name);
	bool is_connected();
	bool connect();
	bool disconnect();
	bool ping();

	bool exec_sql(const char* sz_sql);
	bool query_sql(const char* sz_sql, std::vector<std::vector<std::string> >& vec_query_result);
private:
	MYSQL m_mysql;
	bool m_b_connected;
	std::string m_str_db_ip;
	std::string m_str_user;
	std::string m_str_pswd;
	std::string m_str_db_name;
private:
	boost::mutex m_mutex;
public:
	mysql_adapter()
	{

	}
	~mysql_adapter()
	{
		disconnect();
	}
};