INCLUDES = -I$(top_srcdir)/src/libpakt -I$(top_srcdir)/src/libpakt/core \
	-I$(top_srcdir)/src/libpakt/installer $(all_includes)
METASOURCES = AUTO
libportageloader_a_LIBADD = \
	$(top_builddir)/src/libpakt/portage/core/libportagecore.a $(top_builddir)/src/libpakt/core/libcore.a
noinst_LIBRARIES = libportageloader.a
libportageloader_a_SOURCES = filemakeconfigloader.cpp \
	filepackagekeywordsloader.cpp filepackagemaskloader.cpp portageinitialloader.cpp portageml.cpp \
	portagepackageloader.cpp portagetreescanner.cpp profileloader.cpp filemakeconfigloader.cpp \
	filepackagekeywordsloader.cpp filepackagemaskloader.cpp portageinitialloader.cpp portageml.cpp \
	portagepackageloader.cpp portagetreescanner.cpp profileloader.cpp
