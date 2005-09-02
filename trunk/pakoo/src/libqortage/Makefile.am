INCLUDES = -I$(top_srcdir)/src/libqortage -I$(top_srcdir)/src/libqortage/core \
	-I$(top_srcdir)/src/libqortage/installer -I$(top_srcdir)/src/libqortage/portageconfig \
	-I$(top_srcdir)/src/libqortage/portageloader $(all_includes)
METASOURCES = AUTO

SUBDIRS = core portageloader installer portageconfig
noinst_HEADERS = libqortage.h backendfactory.h portagebackend.h

libpakt_a_LIBADD = $(top_builddir)/src/libqortage/installer/libinstaller.a \
	$(top_builddir)/src/libqortage/portageloader/libportageloader.a $(top_builddir)/src/libqortage/core/libcore.a
noinst_LIBRARIES = libpakt.a
libpakt_a_SOURCES = backendfactory.cpp portagebackend.cpp backendfactory.cpp\
	portagebackend.cpp

