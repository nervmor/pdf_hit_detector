#pragma once

#include "../frame/include.h"
#include "../frame/singleton.h"
#include "tcp_session_mgr.h"
#include "http_session_mgr.h"
#include "http_download_session_mgr.h"
#include "matcher.h"
#ifdef _WIN32
#include "..\third\Libnids-1.19\WIN32-Includes\nids.h"
#else
#include "../third/linux_libnids-1.19/src/nids.h"
#endif

class pdf_hit_detector_core : public singleton<pdf_hit_detector_core>
{
public:
	bool initialize();
	bool start();
	void uninitialize();
private:
	static void tcp_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed);
private:
	tcp_session_mgr m_tcp_session_mgr;
	http_session_mgr m_http_session_mgr;
	http_download_session_mgr m_http_downlad_session_mgr;
	pdf_hit_decector_matcher m_matcher;
private:
	pdf_hit_detector_core()
	{

	}
private:
	SINLETON_CLASS_DECLARE(pdf_hit_detector_core);
};