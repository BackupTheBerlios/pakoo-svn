INCLUDES = -I$(top_srcdir)/src/libqortage/core \
	-I$(top_srcdir)/src/libqortage/portageloader $(all_includes)

noinst_LIBRARIES = libportageloader.a
noinst_HEADERS = profileloader.h filemakeconfigloader.h multiplepackageloader.h \
		portageinitialloader.h packageloader.h initialloader.h
libportageloader_a_SOURCES = fileatomloaderbase.cpp filepackagemaskloader.cpp \
		packagescanner.cpp portageml.cpp portagetreescanner.cpp filepackagekeywordsloader.cpp \
		profileloader.cpp filemakeconfigloader.cpp multiplepackageloader.cpp \
		portageinitialloader.cpp packageloader.cpp initialloader.cpp
libportageloader_a_LIBADD = $(top_builddir)/src/libqortage/core/libcore.a
kdelnkdir = $(kde_appsdir)/System
METASOURCES = AUTO

