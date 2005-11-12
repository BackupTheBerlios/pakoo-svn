INCLUDES = -I$(top_srcdir)/src/libpakt -I$(top_srcdir)/src/libpakt/core \
	-I$(top_srcdir)/src/libpakt/installer -I$(top_srcdir)/src/libpakt/portageconfig \
	-I$(top_srcdir)/src/libpakt/portageloader $(all_includes)
METASOURCES = AUTO

SUBDIRS = core portageloader installer portageconfig
noinst_HEADERS = libpakt.h backendfactory.h portagebackend.h

libpakt_a_LIBADD = $(top_builddir)/src/libpakt/installer/libinstaller.a \
	$(top_builddir)/src/libpakt/portageloader/libportageloader.a $(top_builddir)/src/libpakt/core/libcore.a
noinst_LIBRARIES = libpakt.a
libpakt_a_SOURCES = backendfactory.cpp portagebackend.cpp backendfactory.cpp\
	portagebackend.cpp

