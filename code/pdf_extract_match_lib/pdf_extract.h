#pragma once

#include "db_interface.h"
#include "../frame/singleton.h"


struct pdf_annotation 
{
	struct rect
	{
		double _left;
		double _top;
		double _right;
		double _bottom;
	};
	std::wstring _str_subtype;
	std::wstring _str_create_time;
	std::wstring _str_content;
	std::wstring _str_name;
	std::wstring _str_creator;
	rect _rect;
};


class pdf_extract : public singleton<pdf_extract>
{
public:
	bool initialize(const std::wstring& str_xpdf_config_file);
public:
	bool extract_hash(const std::wstring& str_pdf_filename, std::vector<unsigned int>& vec_hash);
	bool extract_pre_hash(const unsigned char* data, unsigned int size, std::vector<unsigned int>& vec_pre_hash);
	bool extract_hash_to_db(const std::wstring& str_pdf_filename, const std::wstring& str_pdf_description, db_interface* p_db);
	bool extract_pre_hash_to_db(const std::wstring& str_pdf_filename, unsigned int pre_hash_size, const std::wstring& str_pdf_description, db_interface* p_db);

	bool extract_text(const std::wstring& str_pdf_filename, const std::wstring& str_text_filename);
	bool extract_annotation(const std::wstring& str_pdf_filename, std::map<unsigned int, std::vector<pdf_annotation> >& map_page_vec_annotation);
private:
	bool m_init;
private:
	pdf_extract()
		: m_init(false)
	{

	}
private:
	SINLETON_CLASS_DECLARE(pdf_extract);
};