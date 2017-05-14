#pragma once


#include "../frame/util_log.h"

#define CONFIG_FILE_NAME			L"pdf_hit_detector.cfg"



#if 1
#define CONFIG_LOG_TAG		"CONFIG"	
#else
#define CONFIG_LOG_TAG		DISABLE_TAG
#endif

#if 0
#define TCP_SESSION_MGR_LOG_TAG		"TCP_SESSION_MGR"	
#else
#define TCP_SESSION_MGR_LOG_TAG		DISABLE_TAG
#endif



#if 0
#define HTTP_SESSION_MGR_LOG_TAG	"HTTP_SESSION_MGR"	
#else
#define HTTP_SESSION_MGR_LOG_TAG	DISABLE_TAG
#endif



#if 0
#define HTTP_ANALYZER_LOG_TAG		"HTTP_ANALYZER"	
#else
#define HTTP_ANALYZER_LOG_TAG		DISABLE_TAG
#endif



#if 1
#define HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG	"HTTP_DOWNLOAD_MGR"	
#else
#define HTTP_DOWNLOAD_SESSION_MGR_LOG_TAG	DISABLE_TAG
#endif


#if 1
#define PDF_HIT_DECECTOR_MATCHER_LOG_TAG	"MATCHER"	
#else
#define PDF_HIT_DECECTOR_MATCHER_LOG_TAG	DISABLE_TAG
#endif

#if 1
#define PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG	"EXTRACTOR"	
#else
#define PDF_HIT_DECECTOR_EXTRACTOR_LOG_TAG	DISABLE_TAG
#endif


#define HTTP_CONTEXT_NAME								L"http_context"



#define DATABASE_SCHEMA_NAME							L"pdf_hit_detector"

#define HTTP_DOWNLOAD_SESSION_DEFAULT_EXPIRE_TIME				(10 * 60)
#define HTTP_DOWNLOAD_SESSION_DEFAULT_MIN_CONTENT_LENGTH		(1024)

#define PRE_HASH_DEFAULT_SIZE							(512 * 1024)



#define PDF_FORMAT_HEADER_SIGN							"%PDF-1."


#define XPDF_CONFIG_FILE								L"xpdfrc"
#define EXTRACT_OUTPUT_DIR								L"extract_out_put"