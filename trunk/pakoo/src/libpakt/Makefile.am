INCLUDES = -I$(top_srcdir)/src/libpakt -I$(top_srcdir)/src/libpakt/base \
	-I$(top_srcdir)/src/libpakt/base/config -I$(top_srcdir)/src/libpakt/base/core \
	-I$(top_srcdir)/src/libpakt/base/installer -I$(top_srcdir)/src/libpakt/base/loader \
	-I$(top_srcdir)/src/libpakt/core -I$(top_srcdir)/src/libpakt/installer \
	-I$(top_srcdir)/src/libpakt/portage/config -I$(top_srcdir)/src/libpakt/portage/core \
	-I$(top_srcdir)/src/libpakt/portage/installer -I$(top_srcdir)/src/libpakt/portage/loader $(all_includes)
METASOURCES = AUTO

SUBDIRS = core base portage installer
noinst_HEADERS = libpakt.h backendfactory.h portagebackend.h

libpakt_a_LIBADD = $(top_builddir)/src/libpakt/installer/libinstaller.a \
	$(top_builddir)/src/libpakt/core/libcore.a $(top_builddir)/src/libpakt/portage/loader/libportageloader.a \
	$(top_builddir)/src/libpakt/portage/core/libportagecore.a $(top_builddir)/src/libpakt/base/loader/libloader.a
noinst_LIBRARIES = libpakt.a
libpakt_a_SOURCES = backendfactory.cpp portagebackend.cpp backendfactory.cpp\
	portagebackend.cpp

