#include "pdf_hit_detector_core.h"
#include "pdf_extractor_core.h"
#include "config.h"


int main(int argc,char **argv)
{
	setlocale(LC_CTYPE,"chs");
	if (!config::instance()->initialize())
	{
		printf("config initialize fail...");
		return 0;
	}
	if (argc > 1)
	{
		if (argc < 3)
		{
			printf("useage: param1[pdf_file_name] param2[pdf_description]");
			return 0;
		}
		std::wstring str_pdf_filename = util_string::a2w(argv[1]);
		std::wstring str_pdf_description = util_string::a2w(argv[2]);

		if (pdf_extractor_core::instance()->initialize())
		{
			if (pdf_extractor_core::instance()->extract_pdf_file(str_pdf_filename, str_pdf_description))
			{
				printf("extract pdf_file[%S] success..", str_pdf_filename.c_str());
			}
			else
			{
				//printf("extract pdf_file[%S] fail...", str_pdf_filename.c_str());
			}
		}
		else
		{
			printf("pdf_extractor initialize fail...");
		}

		pdf_extractor_core::instance()->uninitialize();
	}
	else
	{
		if (pdf_hit_detector_core::instance()->initialize())
		{
			if (pdf_hit_detector_core::instance()->start())
			{
				printf("pdf_hit_detector start success...");
			}
			else
			{
				printf("pdf_hit_detector start fail...");
			}
		}
		else
		{
			printf("pdf_hit_detector initialize fail...");
		}
		pdf_hit_detector_core::instance()->uninitialize();
	}
	return 0;
}


