#include "mysql_adapter.h"



bool mysql_adapter::init(const char* sz_db_ip, const char* sz_user, const char* sz_pswd, const char* sz_db_name)
{
	m_str_db_ip = sz_db_ip;
	m_str_user = sz_user;
	m_str_pswd = sz_pswd;
	m_str_db_name = sz_db_name;

	mysql_init(&m_mysql);
	char iAutoReconnect = 1;
	ENSUREf(0 == mysql_options(&m_mysql, MYSQL_OPT_RECONNECT, (char*)&iAutoReconnect));//设置自动重连
	return true;
}

bool mysql_adapter::connect()
{
	/*
		连接msql
	*/
	if( mysql_real_connect( &m_mysql, m_str_db_ip.c_str(), m_str_user.c_str(), m_str_pswd.c_str(), m_str_db_name.c_str(), 0, NULL, 0 ) )
	{
		char sz_set_name[] = "set names gb2312";
		mysql_real_query(&m_mysql, sz_set_name, strlen(sz_set_name));
		m_b_connected = true;
	}
	else
	{
		m_b_connected = false;
	}
	return m_b_connected;
}

bool mysql_adapter::is_connected()
{
	return m_b_connected;
}

bool mysql_adapter::disconnect()
{
	mysql_close( &m_mysql );
	m_b_connected = false;
	return true;
}

bool mysql_adapter::exec_sql( const char* sz_sql )
{
	/*
		执行一条不返回结果的SQL
	*/
	ENSUREf(sz_sql != NULL);
	ENSUREf(is_connected());

	boost::mutex::scoped_lock scopedlock(m_mutex);

	bool result = false;
	do 
	{
		mysql_ping( &m_mysql );
		ENSUREK(0 == mysql_real_query( &m_mysql, sz_sql, strlen( sz_sql)));
		result = true;
	}while (false);
	ENSUREf(result);
	return true;
}


bool mysql_adapter::query_sql(const char* sz_sql, std::vector<std::vector<std::string> >& vec_query_result)
{
/*
		常规查询模式，直接返回结果
	*/
	ENSUREf(sz_sql != NULL);
	ENSUREf(is_connected());

	boost::mutex::scoped_lock scopedlock(m_mutex);
	bool result = false;
	do 
	{
		mysql_ping(&m_mysql);
		ENSUREK(0 == mysql_real_query(&m_mysql, sz_sql, strlen( sz_sql)));

		MYSQL_RES* res = mysql_store_result( &m_mysql );
		ENSUREK(res != NULL);
		unsigned int result_count = mysql_num_fields( res );

		std::vector<MYSQL_ROW> rs_row_list;
		MYSQL_ROW sql_row = NULL;
		while( sql_row = mysql_fetch_row( res ) )
		{
			rs_row_list.push_back( sql_row );
		}
		for (int i = 0; i != rs_row_list.size(); i++)
		{
			std::vector<std::string> vec_strings;
			char** p_result_set = rs_row_list[i];
			for (int j = 0; j != result_count; j++)
			{
				vec_strings.push_back(p_result_set[j]);
			}
			vec_query_result.push_back(vec_strings);
		}
		mysql_free_result(res);
		result = true;
	} while ( false );
	ENSUREf(result);
	return true;
}
