ACLOCAL_AMFLAGS=-I m4
AM_CFLAGS=-Wall -Werror

bin_PROGRAMS = aftermath-convert

aftermath_convert_SOURCES = \
	src/main.c \
	src/ost/ost.c \
	src/ost/ost.h \
	src/ost/v16.c \
	src/ost/v16.h \
	src/ost/v16_structs.h


aftermath_convert_CFLAGS = @AFTERMATH_CORE_INCLUDES@ $(AM_CFLAGS)
aftermath_convert_LDADD = @AFTERMATH_CORE_LIBS@

UPSTREAM_VERSION=@PACKAGE_VERSION@

clean-local:
	rm -rf tmp/packaging

deb:
	$(MAKE) dist-gzip
	rm -rf tmp/packaging/debian
	$(MKDIR_P) tmp/packaging/debian
	mv aftermath-convert-$(UPSTREAM_VERSION).tar.gz tmp/packaging/debian/aftermath-convert_$(UPSTREAM_VERSION).orig.tar.gz
	cd tmp/packaging/debian ; tar xf aftermath-convert_$(UPSTREAM_VERSION).orig.tar.gz
	cp -ar $(srcdir)/distros/debian/build/debian tmp/packaging/debian/aftermath-convert-$(UPSTREAM_VERSION)
	cd tmp/packaging/debian/aftermath-convert-$(UPSTREAM_VERSION) ; debuild -us -uc
