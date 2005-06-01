INCLUDES = -I$(top_srcdir)/src/libqortage/core $(all_includes)

noinst_LIBRARIES = libportageloader.a
noinst_HEADERS = fileatomloaderbase.h filepackagemaskloader.h loadingevent.h \
		packagescanner.h portageloaderbase.h portageml.h portagetreescanner.h \
		fileatomloaderbase.h filepackagemaskloader.h loadingevent.h packagescanner.h \
		portageloaderbase.h portageml.h portagetreescanner.h filepackagekeywordsloader.h \
		fileatomloaderbase.h filepackagekeywordsloader.h filepackagemaskloader.h loadingevent.h \
		packagescanner.h portageloaderbase.h portageml.h portagetreescanner.h i18n.h
libportageloader_a_SOURCES = fileatomloaderbase.cpp \
	filepackagemaskloader.cpp	loadingevent.cpp packagescanner.cpp portageloaderbase.cpp \
	portageml.cpp	portagetreescanner.cpp filepackagekeywordsloader.cpp
libportageloader_a_LIBADD = $(top_builddir)/src/libqortage/core/libcore.a
kdelnkdir = $(kde_appsdir)/System
METASOURCES = AUTO
