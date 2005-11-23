INCLUDES = -I$(top_srcdir)/src/libpakt -I$(top_srcdir)/src/libpakt/base/core \
	-I$(top_srcdir)/src/libpakt/portage/core $(all_includes)
METASOURCES = AUTO
noinst_LIBRARIES = libportagecore.a
libportagecore_a_SOURCES = \
	portagecategory.cpp	portagepackage.cpp	portagepackageversion.cpp portagesettings.cpp	portagecategory.cpp portagepackage.cpp \
	portagepackageversion.cpp	portagesettings.cpp dependatom.cpp
libportagecore_a_LIBADD = $(top_builddir)/src/libpakt/base/core/libcore.a
noinst_HEADERS = dependatom.h
