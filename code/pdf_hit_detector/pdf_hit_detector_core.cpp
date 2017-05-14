#include "pdf_hit_detector_core.h"


bool pdf_hit_detector_core::initialize()
{
	ENSUREf(m_matcher.initialize());
	ENSUREf(m_http_downlad_session_mgr.initialize());
	m_http_downlad_session_mgr.add_notifyer(&m_matcher);
	ENSUREf(m_http_session_mgr.initialize());
	m_http_session_mgr.add_notifyer(&m_http_downlad_session_mgr);
	ENSUREf(m_tcp_session_mgr.initialize());
	m_tcp_session_mgr.add_notifyer(&m_http_session_mgr);

#ifdef _WIN32
	nids_params.device = "3";	// "1"��ʾϵͳ�еĵ�һ������
								// �����ز������ݣ�������windows����������һ������������ʵ�ʵ�������ʶΪ"2"
#else
	nids_params.device = "eth0";
#endif
	nids_params.pcap_filter = "tcp port 80";
	ENSUREf(nids_init());
	nids_register_tcp((void*)&pdf_hit_detector_core::tcp_callback);
	return true;
}

void pdf_hit_detector_core::uninitialize()
{
	m_matcher.uninitialize();
	m_http_downlad_session_mgr.uninitialize();
	m_http_session_mgr.uninitialize();
	m_tcp_session_mgr.uninitialize();
}

bool pdf_hit_detector_core::start()
{
	nids_run();
	return true;
}

void pdf_hit_detector_core::tcp_callback(struct tcp_stream *a_tcp, void ** this_time_not_needed)
{
	pdf_hit_detector_core::instance()->m_tcp_session_mgr.on_libnids_tcp_notify(a_tcp, this_time_not_needed);
}
