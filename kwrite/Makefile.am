
INCLUDES = $(all_includes)
LDFLAGS = -lkfile -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 \
	$(all_libraries) $(KDE_RPATH)
XGETTEXT=xgettext

bin_PROGRAMS = kwrite

BUILT_SOURCES =  moc_highlight.cpp moc_kwdoc.cpp moc_kwmainwindow.cpp moc_kwview.cpp moc_search.cpp moc_toplevel.cpp

kwrite_SOURCES = highlight.cpp highlight.h kwdoc.cpp kwdoc.h \
	kwmainwindow.cpp kwmainwindow.h kwview.cpp kwview.h \
	search.cpp search.h toplevel.cpp toplevel.h $(BUILT_SOURCES)
###KMAKE-start (don't edit or delete this block)
moc_highlight.cpp: highlight.h
	$(MOC) highlight.h -o moc_highlight.cpp

moc_kwdoc.cpp: kwdoc.h
	$(MOC) kwdoc.h -o moc_kwdoc.cpp

moc_kwmainwindow.cpp: kwmainwindow.h
	$(MOC) kwmainwindow.h -o moc_kwmainwindow.cpp

moc_kwview.cpp: kwview.h
	$(MOC) kwview.h -o moc_kwview.cpp

moc_search.cpp: search.h
	$(MOC) search.h -o moc_search.cpp

moc_toplevel.cpp: toplevel.h
	$(MOC) toplevel.h -o moc_toplevel.cpp

###KMAKE-end
