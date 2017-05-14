
INCLUDE		= -I../third/boost_1_58_0/ \
          	-I../third/xpdf-3.04 \
          	-I../third/xpdf-3.04/freetype-2.5.5/include \
          	-I../third/xpdf-3.04/goo



CC		= g++

OBJS_LIST		= mysql_adapter.o db_interface.o pdf_extract.o pdf_match.o

TARGET		= libpdf_extract_match_lib.a

CFLAGS		= -g -O2 -D_BSD_SOURCE -W -DLIBNET_VER=1 -DHAVE_ICMPHDR=1 -DHAVE_TCP_STATES=1 -DHAVE_BSD_UDPHDR=1

all:$(TARGET)

$(TARGET):$(OBJS_LIST)
	$(CC) -I. $^ -c -g
	ar -rcs $@ $^

db_interface.o:db_interface.cpp
	$(CC) -fPIC $(CFLAGS) -I. $(INCLUDE) -c db_interface.cpp -o $@ 
mysql_adapter.o:mysql_adapter.cpp
	$(CC) -fPIC $(CFLAGS) -I. $(INCLUDE) -c mysql_adapter.cpp -o $@
pdf_extract.o:pdf_extract.cpp
	$(CC) -fPIC $(CFLAGS) -I. $(INCLUDE) -c pdf_extract.cpp -o $@
pdf_match.o:pdf_match.cpp
	$(CC) -fPIC $(CFLAGS) -I. $(INCLUDE) -c pdf_match.cpp -o $@




clean:
	-rm $(OBJS_LIST) $(TARGET)
