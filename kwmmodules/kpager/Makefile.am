
INCLUDES = $(all_includes)

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD = -lkfile -lkfm -lkdeui -lkdecore $(LIB_KIMGIO) -lqt -lXext -lX11 \
	$(LIBSOCKET)

kpager_SOURCES = windowdrag.cpp ktrianglebutton.cpp desktop.cpp kpagerclient.cpp kpager.cpp main.cpp

noinst_HEADERS = *.h

SRCMETA = ktrianglebutton.moc kpagerclient.moc kpager.moc desktop.moc

DISTCLEANFILES = $(SRCMETA) ./.deps/* ./.deps/.P

bin_PROGRAMS = kpager

%.moc: %.h
	$(MOC) $< -o $@

kpager.cpp:kpager.moc

kpagerclient.cpp:kpagerclient.moc

desktop.cpp:desktop.moc

ktrianglebutton.cpp:ktrianglebutton.moc

applnk_DATA = kpager.kdelnk
applnkdir = $(kde_appsdir)/System

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kpager_SOURCES) && mv messages.po ../../po/kpager.pot
