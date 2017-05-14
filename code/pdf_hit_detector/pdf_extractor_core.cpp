#include "pdf_extractor_core.h"
#include "../frame/util_log.h"
#include "config.h"
#include "../pdf_extract_match_lib/pdf_extract.h"
#include "../frame/util_path.h"
#include "define.h"

bool pdf_extractor_core::initialize()
{
	if (!m_db.initialize(config::instance()->get_db_ip(), 
		config::instance()->get_db_user(),
		config::instance()->get_db_pswd(),
		DATABASE_SCHEMA_NAME))
	{
		util_log::log(PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG, "database connect fail..with[ip:%S] [user:%S] [pswd:%S]",
			config::instance()->get_db_ip().c_str(), 
			config::instance()->get_db_user().c_str(),
			config::instance()->get_db_pswd().c_str());
		return false;
	}
	ENSUREf(pdf_extract::instance()->initialize(XPDF_CONFIG_FILE));
	return true;
}

void pdf_extractor_core::uninitialize()
{
	
}


bool pdf_extractor_core::extract_pdf_file(const std::wstring& str_pdf_file_name, const std::wstring& str_pdf_description)
{
	bool result = true;
	if (!pdf_extract::instance()->extract_hash_to_db(str_pdf_file_name, str_pdf_description, &m_db))
	{
		util_log::log(PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG, "extract hash fail...");
		result = false;
	}
	if (!pdf_extract::instance()->extract_pre_hash_to_db(str_pdf_file_name, config::instance()->get_pre_hash_size(), str_pdf_description, &m_db))
	{
		util_log::log(PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG, "extract pre hash fail...");
		result = false;
	}
	std::wstring str_output_dir = util_path::get_image_dir() + EXTRACT_OUTPUT_DIR + L"/";
	util_path::make_sure_dir_exist(str_output_dir);
	std::wstring str_text_file_name = str_output_dir + util_path::get_base_filename_without_ext(str_pdf_file_name) + L"_text.txt";
	if (!pdf_extract::instance()->extract_text(str_pdf_file_name, str_text_file_name))
	{
		util_log::log(PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG, "extract text fail...");
		result = false;
	}
	
	std::map<unsigned int, std::vector<pdf_annotation> > map_page_vec_annotation;
	if (!pdf_extract::instance()->extract_annotation(str_pdf_file_name, map_page_vec_annotation))
	{
		util_log::log(PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG, "extract annotation fail...");
		result = false;
	}
	else
	{
		for (std::map<unsigned int, std::vector<pdf_annotation> >::iterator it = map_page_vec_annotation.begin(); it != map_page_vec_annotation.end(); it++)
		{
			std::vector<pdf_annotation>& vec_annotation = it->second;
			if (!vec_annotation.empty())
			{
				util_log::log(PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG, "--------------µÚ%dÒ³---------------", it->first);
			}
			for (std::vector<pdf_annotation>::iterator itx = vec_annotation.begin(); itx != vec_annotation.end(); itx++)
			{
				pdf_annotation& annotation = *itx;
				util_log::log(PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG, "\n\
																  Subtype[%S]\n\
																  CreateTime[%S]\n\
																  Content[%S]\n\
																  Name[%S]\n\
																  Creator[%S]\n\
																  Rect[%f, %f, %f, %f]\n",
																   annotation._str_subtype.c_str(),
																   annotation._str_create_time.c_str(),
																   annotation._str_content.c_str(),
																   annotation._str_name.c_str(),
																   annotation._str_creator.c_str(),
																   annotation._rect._left, annotation._rect._top, annotation._rect._right, annotation._rect._bottom 
																   );
			}
		}
	}
	ENSUREf(result);
	return true;
}
