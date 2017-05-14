#include "pdf_match.h"
#include "pdf_extract.h"
#include "../frame/util_string.h"


bool pdf_match::initialize(const std::wstring& str_xpdf_config_file_name)
{
	ENSUREf(pdf_extract::instance()->initialize(str_xpdf_config_file_name));
	return true;
}


bool pdf_match::match_from_db(const std::wstring& str_pdf_filename, std::wstring& str_pdf_description, db_interface* p_db)
{
	ENSUREf(p_db != NULL);

	std::vector<unsigned int> vec_hash;
	ENSUREf(pdf_extract::instance()->extract_hash(str_pdf_filename, vec_hash));

	char sz_sql[4096];
	std::vector<std::vector<std::string> > vec_result;

	unsigned int pdf_hash_id = -1;

	vec_result.clear();
	sprintf(sz_sql, "select id from pdf_hash where hash_1 = %u and hash_2 = %u and hash_3 = %u and hash_4 = %u", vec_hash[0], vec_hash[1], vec_hash[2], vec_hash[3]);
	ENSUREf(p_db->query_sql(sz_sql, vec_result));

	ENSUREf(!vec_result.empty());

	pdf_hash_id = util_string::a2i(vec_result[0][0]);

	unsigned int pdf_description_id = -1;

	vec_result.clear();
	sprintf(sz_sql, "select pdf_description_id from pdf_hash_description where pdf_hash_id = %u", pdf_hash_id);
	ENSUREf(p_db->query_sql(sz_sql, vec_result));
	ENSUREf(!vec_result.empty());

	pdf_description_id = util_string::a2i(vec_result[0][0]);

	vec_result.clear();
	sprintf(sz_sql, "select description from pdf_description where id = %u", pdf_description_id);
	ENSUREf(p_db->query_sql(sz_sql, vec_result));
	ENSUREf(!vec_result.empty());
	str_pdf_description = util_string::a2w(vec_result[0][0]);

	return true;
}

bool pdf_match::match_pre_from_db(const unsigned char* data, unsigned int size, std::wstring& str_pdf_description, db_interface* p_db)
{
	ENSUREf(p_db != NULL);
	
	std::set<unsigned int> set_db_pre_size;

	char sz_sql[4096];
	std::vector<std::vector<std::string> > vec_result;

	sprintf(sz_sql, "select pre_size from pdf_pre_hash");
	ENSUREf(p_db->query_sql(sz_sql, vec_result));
	ENSUREf(!vec_result.empty());
	for (int i = 0; i != vec_result.size(); i++)
	{
		set_db_pre_size.insert(util_string::a2i(vec_result[i][0]));
	}

	std::set<unsigned int> set_pre_size;
	for (std::set<unsigned int>::iterator it = set_db_pre_size.begin(); it != set_db_pre_size.end(); it++)
	{
		unsigned int db_pre_size = *it;
		if (db_pre_size <= size)
		{
			set_pre_size.insert(db_pre_size);
		}
	}

	bool b_match_res = false;
	for (std::set<unsigned int>::iterator it = set_pre_size.begin(); it != set_pre_size.end(); it++)
	{
		unsigned int pre_size = *it;
		do 
		{
			std::vector<unsigned int> vec_pre_hash;
			ENSUREK(pdf_extract::instance()->extract_pre_hash(data, pre_size, vec_pre_hash));

			unsigned int pdf_pre_hash_id = -1;

			vec_result.clear();
			sprintf(sz_sql, "select id from pdf_pre_hash where pre_size = %u and hash_1 = %u and hash_2 = %u", pre_size, vec_pre_hash[0], vec_pre_hash[1]);
			ENSUREf(p_db->query_sql(sz_sql, vec_result));

			ENSUREK(!vec_result.empty());

			pdf_pre_hash_id = util_string::a2i(vec_result[0][0]);

			unsigned int pdf_description_id = -1;

			vec_result.clear();
			sprintf(sz_sql, "select pdf_description_id from pdf_pre_hash_description where pdf_pre_hash_id = %u", pdf_pre_hash_id);
			ENSUREf(p_db->query_sql(sz_sql, vec_result));
			ENSUREK(!vec_result.empty());

			pdf_description_id = util_string::a2i(vec_result[0][0]);

			vec_result.clear();
			sprintf(sz_sql, "select description from pdf_description where id = %u", pdf_description_id);
			ENSUREf(p_db->query_sql(sz_sql, vec_result));
			ENSUREK(!vec_result.empty());
			str_pdf_description = util_string::a2w(vec_result[0][0]);
			b_match_res = true;
		} while (false);
		if (b_match_res)
		{
			break;
		}
	}
	ENSUREf(b_match_res);
	return true;
}

