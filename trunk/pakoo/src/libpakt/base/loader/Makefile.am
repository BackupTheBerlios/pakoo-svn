INCLUDES = -I$(top_srcdir)/src/libpakt $(all_includes)
METASOURCES = AUTO
noinst_LIBRARIES = libloader.a
noinst_HEADERS = initialloader.h packageloader.h multiplepackageloader.h
libloader_a_SOURCES = initialloader.cpp packageloader.cpp \
	multiplepackageloader.cpp
libloader_a_LIBADD = $(top_builddir)/src/libpakt/base/core/libcore.a
