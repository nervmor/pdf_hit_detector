#pragma once

#include "db_interface.h"
#include "../frame/singleton.h"

class pdf_match : public singleton<pdf_match>
{
public:
	bool initialize(const std::wstring& str_xpdf_config_file_name);
	bool match_from_db(const std::wstring& str_pdf_filename, std::wstring& str_pdf_description, db_interface* p_db);
	bool match_pre_from_db(const unsigned char* data, unsigned int size, std::wstring& str_pdf_description, db_interface* p_db);
private:
	pdf_match()
	{

	}
private:
	SINLETON_CLASS_DECLARE(pdf_match);
};
