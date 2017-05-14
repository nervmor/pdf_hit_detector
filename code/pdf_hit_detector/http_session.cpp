#include "http_session.h"

boost::shared_ptr<http_session> http_session::create(const network_link_info& link_info)
{
	boost::shared_ptr<http_session> sp_http_session(new http_session(link_info));
	return sp_http_session;
}
