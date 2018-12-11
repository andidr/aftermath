ACLOCAL_AMFLAGS=-I m4
AM_CFLAGS=-Wall -Werror

bin_PROGRAMS = aftermath-dump

aftermath_dump_SOURCES = src/main.c

aftermath_dump_CFLAGS = @AFTERMATH_CORE_INCLUDES@ $(AM_CFLAGS)
aftermath_dump_LDADD = @AFTERMATH_CORE_LIBS@

UPSTREAM_VERSION=@PACKAGE_VERSION@

clean-local:
	rm -rf tmp/packaging

deb:
	$(MAKE) dist-gzip
	rm -rf tmp/packaging/debian
	$(MKDIR_P) tmp/packaging/debian
	mv aftermath-dump-$(UPSTREAM_VERSION).tar.gz tmp/packaging/debian/aftermath-dump_$(UPSTREAM_VERSION).orig.tar.gz
	cd tmp/packaging/debian ; tar xf aftermath-dump_$(UPSTREAM_VERSION).orig.tar.gz
	cp -ar $(srcdir)/distros/debian/build/debian tmp/packaging/debian/aftermath-dump-$(UPSTREAM_VERSION)
	cd tmp/packaging/debian/aftermath-dump-$(UPSTREAM_VERSION) ; debuild -us -uc
