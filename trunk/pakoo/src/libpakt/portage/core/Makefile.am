INCLUDES = -I$(top_srcdir)/src/libpakt -I$(top_srcdir)/src/libpakt/core 	-I$(top_srcdir)/src/libpakt/installer -I$(top_srcdir)/src/libpakt/portageconfig 	-I$(top_srcdir)/src/libpakt/portageloader $(all_includes)
METASOURCES = AUTO
libportagecore_a_LIBADD = $(top_builddir)/src/libpakt/core/libcore.a
noinst_LIBRARIES = libportagecore.a
libportagecore_a_SOURCES = portagecategory.cpp\
	portagepackage.cpp	portagepackageversion.cpp portagesettings.cpp\
	portagecategory.cpp portagepackage.cpp portagepackageversion.cpp\
	portagesettings.cpp
