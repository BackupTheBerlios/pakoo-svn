INCLUDES = -I$(top_srcdir)/src/libqortage/core $(all_includes)

noinst_LIBRARIES = libportageloader.a
noinst_HEADERS = i18n.h profileloader.h filemakeconfigloader.h
libportageloader_a_SOURCES = fileatomloaderbase.cpp \
			filepackagemaskloader.cpp	loadingevent.cpp packagescanner.cpp portageloaderbase.cpp \
			portageml.cpp	portagetreescanner.cpp filepackagekeywordsloader.cpp profileloader.cpp \
	filemakeconfigloader.cpp
libportageloader_a_LIBADD = $(top_builddir)/src/libqortage/core/libcore.a
kdelnkdir = $(kde_appsdir)/System
METASOURCES = AUTO
