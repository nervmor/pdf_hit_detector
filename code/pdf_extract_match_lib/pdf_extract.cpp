#include "pdf_extract.h"
#include "../third/xpdf-3.04/xpdf/GlobalParams.h"
#include "../third/xpdf-3.04/xpdf/PDFDoc.h"
#include "../third/xpdf-3.04/xpdf/TextOutputDev.h"
#include "../frame/util_string.h"
#include "../third/arithmetic/blizzard_hash.h"
#include "../frame/util_string.h"
#include "../frame/util_buffer.h"

extern GlobalParams* globalParams;



bool pdf_extract::initialize(const std::wstring& str_xpdf_config_file)
{
	globalParams = new GlobalParams(util_string::w2a(str_xpdf_config_file).c_str());
	m_init = true;
	return true;
}

bool pdf_extract::extract_hash(const std::wstring& str_pdf_filename, std::vector<unsigned int>& vec_hash)
{
	ENSUREf(m_init);
	GString* p_file_name = new GString(util_string::w2a(str_pdf_filename).c_str());
	
	boost::shared_ptr<PDFDoc> sp_pdf_doc(new PDFDoc(p_file_name));
	ENSUREf(sp_pdf_doc->isOk());
	unsigned int page_num = sp_pdf_doc->getNumPages();

	std::string str_hash1 = "hash1";
	str_hash1 += util_string::i2a(page_num);
	str_hash1 += util_string::d2a(sp_pdf_doc->getPDFVersion());

	Object doc_info;
	if (sp_pdf_doc->getDocInfo(&doc_info) != NULL)
	{
		Dict* p_dict = doc_info.getDict();
		if (p_dict != NULL)
		{
			static const char* s_info_key[] = 
			{
				"Title",
				"Subject",
				"Keywords",
				"Author",
				"Creator",
				"Producer",
				"CreationDate",
				"ModDate"
			};

			for (int i = 0; i != ArrayCount(s_info_key); i++)
			{
				Object obj;
				if (p_dict->lookup(s_info_key[i], &obj))
				{
					if (obj.isString())
					{
						str_hash1 += obj.getString()->getCString();
					}
					obj.free();
				}
			}
		}
		doc_info.free();
	}

	std::string str_hash2 = "hash2";
	
	if (sp_pdf_doc->getStructTreeRoot()->isDict())
	{
		str_hash2 += "Tagged yes";
	}
	else
	{
		str_hash2 += "Tagged no";
	}
	if (sp_pdf_doc->isEncrypted())
	{
		str_hash2 += "Encrypt yes";
	}
	else
	{
		str_hash2 += "Encrypt no";
	}

	Object* p_acroForm = sp_pdf_doc->getCatalog()->getAcroForm();
	if (p_acroForm != NULL && p_acroForm->isDict())
	{
		Object xfa;
		if (p_acroForm->dictLookup("XFA", &xfa) != NULL)
		{
			if (xfa.isString() || xfa.isArray())
			{
				str_hash2 += "Form:XFA";
			}
			else
			{
				str_hash2 += "Form:AcroForm";
			}
			xfa.free();
		}
	}


	std::string str_hash3 = "hash3";
	if (page_num != 0)
	{
		for (int page = 1; page != page_num; page++)
		{
			str_hash3 += util_string::d2a(sp_pdf_doc->getPageCropHeight(page));
			str_hash3 += util_string::d2a(sp_pdf_doc->getPageCropWidth(page));
			str_hash3 += util_string::d2a(sp_pdf_doc->getPageMediaHeight(page));
			str_hash3 += util_string::d2a(sp_pdf_doc->getPageMediaWidth(page));
			str_hash3 += util_string::i2a(sp_pdf_doc->getPageRotate(page));

			Page* p_page = sp_pdf_doc->getCatalog()->getPage(page);
			std::vector<PDFRectangle*> vec_pdf_rectangle;

			PDFRectangle* p_pdf_rectangle = NULL;

			p_pdf_rectangle = p_page->getMediaBox();
			if (p_pdf_rectangle != NULL)
			{
				vec_pdf_rectangle.push_back(p_pdf_rectangle);
			}
			p_pdf_rectangle = p_page->getCropBox();
			if (p_pdf_rectangle != NULL)
			{
				vec_pdf_rectangle.push_back(p_pdf_rectangle);
			}
			p_pdf_rectangle = p_page->getBleedBox();
			if (p_pdf_rectangle != NULL)
			{
				vec_pdf_rectangle.push_back(p_pdf_rectangle);
			}
			p_pdf_rectangle = p_page->getTrimBox();
			if (p_pdf_rectangle != NULL)
			{
				vec_pdf_rectangle.push_back(p_pdf_rectangle);
			}
			p_pdf_rectangle = p_page->getArtBox();
			if (p_pdf_rectangle != NULL)
			{
				vec_pdf_rectangle.push_back(p_pdf_rectangle);
			}

			for (std::vector<PDFRectangle*>::iterator it = vec_pdf_rectangle.begin(); it != vec_pdf_rectangle.end(); it++)
			{
				PDFRectangle* p_rectangle = *it;
				str_hash3 += util_string::d2a(p_rectangle->x1);
				str_hash3 += util_string::d2a(p_rectangle->x2);
				str_hash3 += util_string::d2a(p_rectangle->y1);
				str_hash3 += util_string::d2a(p_rectangle->y2);
			}
		}
	}
	
	std::string str_hash4 = "hash4";
	std::map<unsigned int, std::vector<pdf_annotation> > map_page_vec_annotation;
	if (extract_annotation(str_pdf_filename, map_page_vec_annotation))
	{
		for (std::map<unsigned int, std::vector<pdf_annotation> >::iterator it = map_page_vec_annotation.begin(); it != map_page_vec_annotation.end(); it++)
		{
			std::vector<pdf_annotation>& vec_annotation = it->second;
			for (std::vector<pdf_annotation>::iterator itx = vec_annotation.begin(); itx != vec_annotation.end(); itx++)
			{
				pdf_annotation& annotation = *itx;
				str_hash4 += util_string::w2a(annotation._str_subtype);
				str_hash4 += util_string::w2a(annotation._str_name);
				str_hash4 += util_string::w2a(annotation._str_create_time);
				str_hash4 += util_string::w2a(annotation._str_creator);
				str_hash4 += util_string::w2a(annotation._str_content);
				str_hash4 += util_string::d2a(annotation._rect._bottom);
				str_hash4 += util_string::d2a(annotation._rect._left);
				str_hash4 += util_string::d2a(annotation._rect._right);
				str_hash4 += util_string::d2a(annotation._rect._top);
			}
		}
	}
	
	unsigned int hash_1 = blizzard_hash::hash((const unsigned char*)str_hash1.c_str(), str_hash1.size());
	unsigned int hash_2 = blizzard_hash::hash((const unsigned char*)str_hash2.c_str(), str_hash2.size());
	unsigned int hash_3 = blizzard_hash::hash((const unsigned char*)str_hash3.c_str(), str_hash3.size());
	unsigned int hash_4 = blizzard_hash::hash((const unsigned char*)str_hash4.c_str(), str_hash4.size());

	boost::shared_ptr<GString> sp_meta(sp_pdf_doc->readMetadata());
	if (sp_meta != NULL)
	{
		unsigned int meta_hash = blizzard_hash::hash((const unsigned char*)sp_meta->getCString(), sp_meta->getLength());
		hash_1 ^= meta_hash;
		hash_2 ^= meta_hash;
		hash_3 ^= meta_hash;
		hash_4 ^= meta_hash;
	}

	vec_hash.push_back(hash_1);
	vec_hash.push_back(hash_2);
	vec_hash.push_back(hash_3);
	vec_hash.push_back(hash_4);

	return true;
}



bool pdf_extract::extract_pre_hash(const unsigned char* data, unsigned int size, std::vector<unsigned int>& vec_pre_hash)
{
	ENSUREf(m_init);
	ENSUREf(data != NULL && size != 0);
	unsigned int hash_1 = blizzard_hash::hash(data, size);
	util_buffer buf;
	ENSUREf(buf.create(size));
	for (int i = size - 1; i >= 0; i--)
	{
		buf.write(&data[i], 1);
	}
	unsigned int hash_2 = blizzard_hash::hash(buf.data(), buf.size());
	vec_pre_hash.push_back(hash_1);
	vec_pre_hash.push_back(hash_2);
	return true;
}

bool pdf_extract::extract_hash_to_db(const std::wstring& str_pdf_filename, const std::wstring& str_pdf_description, db_interface* p_db)
{
	ENSUREf(m_init);
	ENSUREf(p_db != NULL);
	std::vector<unsigned int> vec_hash;
	ENSUREf(extract_hash(str_pdf_filename, vec_hash));
	
	char sz_sql[4096];
	std::vector<std::vector<std::string> > vec_result;

	unsigned int pdf_hash_id = -1;
	do 
	{
		vec_result.clear();
		sprintf(sz_sql, "select id from pdf_hash where hash_1 = %u and hash_2 = %u and hash_3 = %u and hash_4 = %u", vec_hash[0], vec_hash[1], vec_hash[2], vec_hash[3]);
		ENSUREf(p_db->query_sql(sz_sql, vec_result));
		
		if (vec_result.empty())
		{
			vec_result.clear();
			sprintf(sz_sql, "insert into pdf_hash (hash_1, hash_2, hash_3, hash_4) values(%u, %u, %u, %u)", vec_hash[0], vec_hash[1], vec_hash[2], vec_hash[3]);
			ENSUREf(p_db->exec_sql(sz_sql));
		}
		else
		{
			pdf_hash_id = util_string::a2i(vec_result[0][0]);
		}
	} while (pdf_hash_id == -1);
	

	unsigned int pdf_description_id = -1;
	do 
	{
		vec_result.clear();
		sprintf(sz_sql, "select id from pdf_description where description = \'%S\'", str_pdf_description.c_str());
		ENSUREf(p_db->query_sql(sz_sql, vec_result));

		if (vec_result.empty())
		{
			vec_result.clear();
			sprintf(sz_sql, "insert into pdf_description (description) values(\'%S\')", str_pdf_description.c_str());
			ENSUREf(p_db->exec_sql(sz_sql));
		}
		else
		{
			pdf_description_id = util_string::a2i(vec_result[0][0]);
		}
	} while (pdf_description_id == -1);

	vec_result.clear();
	sprintf(sz_sql, "insert into pdf_hash_description (pdf_hash_id, pdf_description_id) values (%u, %u)", pdf_hash_id, pdf_description_id);
	ENSUREf(p_db->exec_sql(sz_sql));

	return true;
}

bool pdf_extract::extract_pre_hash_to_db(const std::wstring& str_pdf_filename, unsigned int pre_hash_size, const std::wstring& str_pdf_description, db_interface* p_db)
{
	ENSUREf(m_init);
	ENSUREf(p_db != NULL);
	util_buffer pre_data;
	ENSUREf(pre_data.create(pre_hash_size));

	bool b_read_res = false;
	FILE* p_file = NULL;
	do 
	{
		p_file = fopen(util_string::w2a(str_pdf_filename).c_str(), "rb");
		ENSUREK(p_file != NULL);
		ENSUREK(pre_hash_size == fread(pre_data._data(), 1, pre_hash_size, p_file));
		b_read_res = true;
	} while (false);
	if (p_file != NULL)
	{
		fclose(p_file);
	}
	ENSUREf(b_read_res);

	std::vector<unsigned int> vec_hash;
	ENSUREf(extract_pre_hash(pre_data.data(), pre_data.size(), vec_hash));

	char sz_sql[4096];
	std::vector<std::vector<std::string> > vec_result;

	unsigned int pdf_pre_hash_id = -1;
	do 
	{
		vec_result.clear();
		sprintf(sz_sql, "select id from pdf_pre_hash where pre_size = %u and hash_1 = %u and hash_2 = %u", pre_hash_size, vec_hash[0], vec_hash[1]);
		ENSUREf(p_db->query_sql(sz_sql, vec_result));

		if (vec_result.empty())
		{
			vec_result.clear();
			sprintf(sz_sql, "insert into pdf_pre_hash (pre_size, hash_1, hash_2) values(%u, %u, %u)", pre_hash_size, vec_hash[0], vec_hash[1]);
			ENSUREf(p_db->exec_sql(sz_sql));
		}
		else
		{
			pdf_pre_hash_id = util_string::a2i(vec_result[0][0]);
		}
	} while (pdf_pre_hash_id == -1);


	unsigned int pdf_description_id = -1;
	do 
	{
		vec_result.clear();
		sprintf(sz_sql, "select id from pdf_description where description = \'%S\'", str_pdf_description.c_str());
		ENSUREf(p_db->query_sql(sz_sql, vec_result));

		if (vec_result.empty())
		{
			vec_result.clear();
			sprintf(sz_sql, "insert into pdf_description (description) values(\'%S\')", str_pdf_description.c_str());
			ENSUREf(p_db->exec_sql(sz_sql));
		}
		else
		{
			pdf_description_id = util_string::a2i(vec_result[0][0]);
		}
	} while (pdf_description_id == -1);

	vec_result.clear();
	sprintf(sz_sql, "insert into pdf_pre_hash_description (pdf_pre_hash_id, pdf_description_id) values (%u, %u)", pdf_pre_hash_id, pdf_description_id);
	ENSUREf(p_db->exec_sql(sz_sql));

	return true;
}





bool pdf_extract::extract_text(const std::wstring& str_pdf_filename, const std::wstring& str_text_filename)
{
	globalParams->setTextEncoding("GBK");
	
	GString* p_file_name = new GString(util_string::w2a(str_pdf_filename).c_str());

	boost::shared_ptr<PDFDoc> sp_pdf_doc(new PDFDoc(p_file_name));
	ENSUREf(sp_pdf_doc->isOk());
	ENSUREf(sp_pdf_doc->okToCopy());

	unsigned int page_count = sp_pdf_doc->getNumPages();
	ENSUREf(page_count != 0);

	TextOutputControl text_out_control;
	text_out_control.mode = textOutReadingOrder;
	text_out_control.clipText = gFalse;

	TextOutputDev text_output_dev(const_cast<char*>(util_string::w2a(str_text_filename).c_str()), &text_out_control, gFalse);
	ENSUREf(text_output_dev.isOk());

	sp_pdf_doc->displayPages(&text_output_dev, 1, page_count, 72, 72, 0, gFalse, gTrue, gFalse);
	return true;
}

bool pdf_extract::extract_annotation(const std::wstring& str_pdf_filename, std::map<unsigned int, std::vector<pdf_annotation> >& map_page_vec_annotation)
{
	GString* p_file_name = new GString(util_string::w2a(str_pdf_filename).c_str());

	boost::shared_ptr<PDFDoc> sp_pdf_doc(new PDFDoc(p_file_name));
	ENSUREf(sp_pdf_doc->isOk());

	unsigned int page_count = sp_pdf_doc->getNumPages();
	ENSUREf(page_count != 0);

	for (int page = 1; page != page_count; page++)
	{
		Page* p_page = sp_pdf_doc->getCatalog()->getPage(page);

		std::vector<pdf_annotation> vec_annotation;
		Object annots_array;
		do 
		{
			ENSUREK(NULL != p_page->getAnnots(&annots_array));
			ENSUREK(annots_array.isArray());
			for (int n = 0; n != annots_array.arrayGetLength(); n++)
			{
				pdf_annotation annotation;
				Object annots_dict;
				Object annot_attribute;
				do 
				{
					annots_array.arrayGet(n, &annots_dict);
					ENSUREK(annots_dict.isDict());
					
					ENSUREK(NULL != annots_dict.dictLookup("Subtype", &annot_attribute));
					ENSUREK(annot_attribute.isName());
					std::wstring str_subtype = util_string::a2w(annot_attribute.getName());
					ENSUREK(str_subtype != L"Popup");
					annotation._str_subtype = str_subtype;
					annot_attribute.free();

					ENSUREK(NULL != annots_dict.dictLookup("T", &annot_attribute));
					ENSUREK(annot_attribute.isString());
					annotation._str_creator = util_string::a2w(annot_attribute.getString()->getCString());
					annot_attribute.free();
					
					ENSUREK(NULL != annots_dict.dictLookup("Rect", &annot_attribute));
					ENSUREK(annot_attribute.isArray());
					ENSUREK(4 == annot_attribute.arrayGetLength());

					double rect_x[4];
					for (int y = 0; y != 4; y++)
					{
						Object x;
						if (NULL != annot_attribute.arrayGet(y, &x) && x.isReal())
						{
							rect_x[y] = x.getReal();
						}
						x.free();
					}
					annotation._rect._left = rect_x[0];
					annotation._rect._top = rect_x[1];
					annotation._rect._right = rect_x[2];
					annotation._rect._bottom = rect_x[3];
					annot_attribute.free();
					
					ENSUREK(NULL != annots_dict.dictLookup("CreationDate", &annot_attribute));
					ENSUREK(annot_attribute.isString());
					annotation._str_create_time = util_string::a2w(annot_attribute.getString()->getCString());
					annot_attribute.free();

					if (NULL != annots_dict.dictLookup("Name", &annot_attribute) && annot_attribute.isName())
					{
						annotation._str_name = util_string::a2w(annot_attribute.getName());
						annot_attribute.free();
					}

					if (NULL != annots_dict.dictLookup("Contents", &annot_attribute) && annot_attribute.isString())
					{
						const char* p_content = annot_attribute.getString()->getCString();
						if ((unsigned char)p_content[0] == 0xFE)
						{
							annotation._str_content = util_string::usc2_to_utf8((wchar_t*)p_content, annot_attribute.getString()->getLength());
						}
						else
						{
							annotation._str_content = util_string::a2w(p_content);
						}
						annot_attribute.free();
					}
					vec_annotation.push_back(annotation);
				}while (false);
				annot_attribute.free();
				annots_dict.free();
			}
		} while (false);
		annots_array.free();
		if (!vec_annotation.empty())
		{
			map_page_vec_annotation.insert(std::make_pair(page, vec_annotation));
		}
	}
	return true;
}