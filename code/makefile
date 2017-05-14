

all:build



build:
	cd third/xpdf-3.04; $(MAKE); cd xpdf; $(MAKE) -f xpdfstaticlib_makefile
	cd third/linux_libnids-1.19; $(MAKE)
	cd pdf_extract_match_lib;$(MAKE)
	cd pdf_hit_detector; $(MAKE)
	-mkdir ./target
	cp -avxrf ./pdf_hit_detector/pdf_hit_detector ./target/pdf_hit_detector
	cp -avxrf ./config/pdf_hit_detector.cfg ./target/pdf_hit_detector.cfg
	cp -avxrf ./config/xpdfrc ./target/xpdfrc
	-mkdir ./target/language_package
	cp -avxrf ./config/language_package/* ./target/language_package
	
clean:
	cd pdf_extract_match_lib;$(MAKE) clean
	cd pdf_hit_detector; $(MAKE) clean
	cd third/xpdf-3.04; $(MAKE) clean; cd xpdf; $(MAKE) clean -f xpdfstaticlib_makefile
	cd third/linux_libnids-1.19; $(MAKE) clean
	-rm -fr ./target
	
	
prebuild:
	cd ../ref; tar -zxvf libpcap-1.7.3.tar.gz; cd libpcap-1.7.3; ./configure; $(MAKE); $(MAKE) install; rm -r ../libpcap-1.7.3
	cd ../ref; tar -zxvf libnet-1.1.2.1.tar.gz; cd libnet; ./configure; $(MAKE); $(MAKE) install; rm -r ../libnet
	cd ../ref; tar -zxvf libpng-1.6.17.tar.gz; cd libpng-1.6.17; ./configure; $(MAKE); $(MAKE) install; rm -r ../libpng-1.6.17
	cd third/boost_1_58_0; ./bootstrap.sh; ./b2 stage link=static runtime-link=static threading=multi release --with-date_time --with-thread --with-filesystem --with-locale
	cd third/xpdf-3.04; ./configure
	cd third/linux_libnids-1.19; ./configure
