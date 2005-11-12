INCLUDES = -I$(top_srcdir)/src/libpakt/core \
	-I$(top_srcdir)/src/libpakt/portageloader $(all_includes)

noinst_LIBRARIES = libportageloader.a
noinst_HEADERS = profileloader.h filemakeconfigloader.h multiplepackageloader.h \
	portageinitialloader.h packageloader.h initialloader.h portagepackageloader.h
libportageloader_a_SOURCES = fileatomloaderbase.cpp filepackagemaskloader.cpp \
	portageml.cpp portagetreescanner.cpp filepackagekeywordsloader.cpp profileloader.cpp \
	filemakeconfigloader.cpp multiplepackageloader.cpp portageinitialloader.cpp packageloader.cpp \
	initialloader.cpp portagepackageloader.cpp
libportageloader_a_LIBADD = $(top_builddir)/src/libpakt/core/libcore.a
kdelnkdir = $(kde_appsdir)/System
METASOURCES = AUTO

