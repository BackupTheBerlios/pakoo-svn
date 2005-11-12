## Makefile.am for pakoo

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = pakoo pakoo_client

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/src/libpakt -I$(top_srcdir)/src/portagewidgets \
	$(all_includes)

# the library search path.
pakoo_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
pakoo_LDADD = $(top_builddir)/src/portagewidgets/libportagewidgets.a \
	$(top_builddir)/src/libpakt/libpakt.a $(top_builddir)/src/libpakt/portageloader/libportageloader.a \
	$(top_builddir)/src/libpakt/installer/libinstaller.a $(top_builddir)/src/libpakt/core/libcore.a $(LIB_KFILE) $(LIB_KHTML)

# which sources should be compiled for pakoo
pakoo_SOURCES = main.cpp pakoo.cpp pakooview.cpp pref.cpp pakooiface.skel \
	pakooconfig.kcfgc actionview.cpp installeractionview.cpp

# these are the headers for your project
noinst_HEADERS = pakoo.h pakooview.h pref.h i18n.h actionview.h \
	installeractionview.h

# client stuff
pakoo_client_LDFLAGS = $(KDE_RPATH) $(all_libraries)
pakoo_client_LDADD = $(LIB_KDECORE)
pakoo_client_SOURCES = pakoo_client.cpp

# let automoc handle all of the meta source files (moc)


messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/pakoo.pot

KDE_ICON = pakoo

# this is where the kdelnk file will go
kdelnkdir = $(kde_appsdir)/System
kdelnk_DATA = pakoo.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/pakoo
rc_DATA = pakooui.rc

# KConfigXT
kde_kcfg_DATA = pakooconfig.kcfg








SUBDIRS = libpakt portagewidgets

METASOURCES = AUTO
