#pragma once

#include "../frame/singleton.h"
#include "../pdf_extract_match_lib/db_interface.h"

class pdf_extractor_core : public singleton<pdf_extractor_core>
{
public:
	bool initialize();
	void uninitialize();
public:
	bool extract_pdf_file(const std::wstring& str_pdf_file_name, const std::wstring& str_pdf_description);
private:
	db_interface m_db;
private:
	SINLETON_CLASS_DECLARE(pdf_extractor_core);
};