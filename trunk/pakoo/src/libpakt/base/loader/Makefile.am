INCLUDES = -I$(top_srcdir)/src/libpakt -I$(top_srcdir)/src/libpakt/core \
	-I$(top_srcdir)/src/libpakt/installer $(all_includes)
METASOURCES = AUTO
noinst_LIBRARIES = libloader.a
noinst_HEADERS = initialloader.h packageloader.h multiplepackageloader.h \
	fileatomloaderbase.h
libloader_a_SOURCES = initialloader.cpp packageloader.cpp \
		multiplepackageloader.cpp fileatomloaderbase.cpp
libloader_a_LIBADD = $(top_builddir)/src/libpakt/core/libcore.a
