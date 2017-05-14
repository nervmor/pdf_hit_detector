#include "db_interface.h"
#include "../frame/util_string.h"
#include "init_table_sql.h"

bool db_interface::initialize(const std::wstring& str_db_ip, const std::wstring& str_db_user, const std::wstring& str_db_pswd, const std::wstring& str_db_name)
{
	ENSUREf(m_mysql.init(util_string::w2a(str_db_ip).c_str(), util_string::w2a(str_db_user).c_str(), util_string::w2a(str_db_pswd).c_str(), util_string::w2a(str_db_name).c_str()));
	ENSUREf(m_mysql.connect());
	return true;
}

void db_interface::uninitialize()
{
	m_mysql.disconnect();
}

bool db_interface::exec_sql(const char* sz_sql)
{
	ENSUREf(m_mysql.exec_sql(sz_sql));
	return true;
}

bool db_interface::query_sql(const char* sz_sql, std::vector<std::vector<std::string> >& vec_query_result)
{
	ENSUREf(m_mysql.query_sql(sz_sql, vec_query_result));
	return true;
}

