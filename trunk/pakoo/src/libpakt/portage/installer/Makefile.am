INCLUDES = -I$(top_srcdir)/src/libpakt \
	-I$(top_srcdir)/src/libpakt/base/installer -I$(top_srcdir)/src/libpakt/portage/installer $(all_includes)
METASOURCES = AUTO
noinst_LIBRARIES = libportageinstaller.a
libportageinstaller_a_SOURCES = emergeprocess.cpp syncprocess.cpp
noinst_HEADERS = emergeprocess.h syncprocess.h
libportageinstaller_a_LIBADD = \
	$(top_builddir)/src/libpakt/portage/core/libportagecore.a $(top_builddir)/src/libpakt/base/core/libcore.a
