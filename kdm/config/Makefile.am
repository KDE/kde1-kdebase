# $Id$

confdir	= $(kde_confdir)

conf_DATA = kdmrc
DISTCLEANFILES = kdmrc

kdmrc: kdmrc.cpp
	sed -e "s#KDEDIR#$(prefix)#" $(srcdir)/kdmrc.cpp > kdmrc

EXTRA_DIST = kdmrc.cpp


