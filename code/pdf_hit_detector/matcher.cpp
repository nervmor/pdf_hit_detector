#include "matcher.h"
#include "define.h"
#include "../frame/util_string.h"
#include "../frame/util_path.h"
#include "../third/arithmetic/blizzard_hash.h"
#include "../pdf_extract_match_lib/pdf_match.h"
#include "config.h"

void pdf_hit_decector_matcher::on_integral_http_download_session(boost::shared_ptr<http_download_session> sp_http_download_session)
{
	util_log::logd(PDF_HIT_DECECTOR_MATCHER_LOG_TAG, "match http download session...\n\
													 downloader_ip[%S]\n\
													 server_ip[%S]\n\
													 file_name[%S]\n\
													 file_size[%d]",
													 sp_http_download_session->get_downloader_ip().c_str(),
													 sp_http_download_session->get_server_ip().c_str(),
													 sp_http_download_session->get_file_name().c_str(), 
													 sp_http_download_session->get_file_size());

	do 
	{
		ENSUREK(pdf_weak_check(sp_http_download_session->get_file_data()));
		std::wstring str_cache_dir = util_path::get_image_dir() + L"match_cache/";
		util_path::make_sure_dir_exist(str_cache_dir);
		std::wstring str_file = str_cache_dir + 
			util_string::i2w(blizzard_hash::hash((const unsigned char*)sp_http_download_session->get_file_name().c_str(), sp_http_download_session->get_file_name().size() * sizeof(wchar_t)))
			+ L".cache";

		FILE* p_file = fopen(util_string::w2a(str_file).c_str(), "wb");
		if (p_file == NULL)
		{
			util_log::log(PDF_HIT_DECECTOR_MATCHER_LOG_TAG, "create cache file to match fail...cache_file[%S] pdf_file[]", str_file.c_str(), sp_http_download_session->get_file_name().c_str());
			break;
		}
		fwrite(sp_http_download_session->get_file_data()->data(), 1, sp_http_download_session->get_file_size(), p_file);
		fclose(p_file);
		
		std::wstring str_pdf_description;
		if (!match(str_file, str_pdf_description))
		{
			util_log::log(PDF_HIT_DECECTOR_MATCHER_LOG_TAG, "match fail...pdf_file[%S]", sp_http_download_session->get_file_name().c_str());
		}
		else
		{
			util_log::log(PDF_HIT_DECECTOR_MATCHER_LOG_TAG, "match success...pdf_file[%S] pdf_description[%S]", sp_http_download_session->get_file_name().c_str(), str_pdf_description.c_str());
		}
		util_path::delete_file(str_file);
	} while (false);
}

void pdf_hit_decector_matcher::on_incomplete_http_download_session(boost::shared_ptr<http_download_session> sp_http_download_session)
{
	do 
	{
		ENSUREK(pdf_weak_check(sp_http_download_session->get_file_data()));
		std::wstring str_pdf_description;
		if (match_pre(sp_http_download_session->get_file_data(), str_pdf_description))
		{
			util_log::log(PDF_HIT_DECECTOR_MATCHER_LOG_TAG, "match sucess by pre hash...pdf_file[%S] pdf_description[%S]", sp_http_download_session->get_file_name().c_str(), str_pdf_description.c_str());
			break;
		}
		on_integral_http_download_session(sp_http_download_session);
	} while (false);
}

bool pdf_hit_decector_matcher::initialize()
{
	if (!m_db.initialize(config::instance()->get_db_ip(), 
					config::instance()->get_db_user(),
					config::instance()->get_db_pswd(),
					DATABASE_SCHEMA_NAME))
	{
		util_log::log(PDF_HIT_DECECTOR_MATCHER_LOG_TAG, "database connect fail..with[ip:%S] [user:%S] [pswd:%S]",
			config::instance()->get_db_ip().c_str(), 
			config::instance()->get_db_user().c_str(),
			config::instance()->get_db_pswd().c_str());
		return false;
	}
	ENSUREf(pdf_match::instance()->initialize(XPDF_CONFIG_FILE));
	return true;
}

void pdf_hit_decector_matcher::uninitialize()
{

}

bool pdf_hit_decector_matcher::pdf_weak_check(boost::shared_ptr<util_buffer> sp_file_data)
{
	char pdf_header_sign[] = PDF_FORMAT_HEADER_SIGN;
	ENSUREf(sp_file_data->size() > strlen(PDF_FORMAT_HEADER_SIGN));
	ENSUREf(0 == memcmp((void*)sp_file_data->data(), pdf_header_sign, strlen(PDF_FORMAT_HEADER_SIGN)));
	return true;
}

bool pdf_hit_decector_matcher::match(const std::wstring& str_file_name, std::wstring& str_pdf_description)
{
	ENSUREf(pdf_match::instance()->match_from_db(str_file_name, str_pdf_description, &m_db));
	return true;
}

bool pdf_hit_decector_matcher::match_pre(boost::shared_ptr<util_buffer> sp_file_data, std::wstring& str_pdf_description)
{
	ENSUREf(pdf_match::instance()->match_pre_from_db(sp_file_data->data(), sp_file_data->size(), str_pdf_description, &m_db));
	return true;
}
