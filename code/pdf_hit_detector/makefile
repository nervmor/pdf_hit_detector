


INCLUDE_DIR		= -I../frame \
			-I../third/boost_1_58_0

LIB_LIST		= -L../third/linux_libnids-1.19/src -lnids -lpcap -lnet \
			-L../pdf_extract_match_lib \
			-lpdf_extract_match_lib \
			-L../third/boost_1_58_0/stage/lib \
			-lboost_date_time \
			-lboost_system \
			-lboost_locale \
			-lboost_filesystem \
			-lboost_thread \
			-L../third/xpdf-3.04/xpdf \
			-lxpdfstaticlib \
			-lpthread \
			-lmysqlclient \
			-ldl \
			-lz



OBJ_LIST		= http_parser.o \
			http_analyzer.o \
			http_download_session.o \
			http_download_session_mgr.o \
			http_session.o \
			http_session_mgr.o \
			tcp_session.o \
			tcp_session_mgr.o \
			matcher.o \
			pdf_extractor_core.o \
			pdf_hit_detector_core.o \
			main.o

CC			= g++


CFLAGS          = -g -O2 -D_BSD_SOURCE -W -DLIBNET_VER=1 -DHAVE_ICMPHDR=1 -DHAVE_TCP_STATES=1 -DHAVE_BSD_UDPHDR=1


TARGET			= pdf_hit_detector



all:$(TARGET)


$(TARGET):$(OBJ_LIST)
	$(CC) $^ -o $@ $(LIB_LIST) -g


http_analyzer.o:http_analyzer.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@

http_download_session.o:http_download_session.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@
http_download_session_mgr.o:http_download_session_mgr.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@

http_session.o:http_session.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@

http_session_mgr.o:http_session_mgr.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@

main.o:main.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@

matcher.o:matcher.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@

pdf_extractor_core.o:pdf_extractor_core.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@

pdf_hit_detector_core.o:pdf_hit_detector_core.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@
tcp_session.o:tcp_session.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@
tcp_session_mgr.o:tcp_session_mgr.cpp
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@

http_parser.o:../third/http-parser-master/http_parser.c
	$(CC) $^ -c -fPIC $(CFLAGS) -I. $(INCLUDE_DIR) -o $@




clean:
	-rm $(OBJ_LIST) ../$(TARGET)


