
confdir	= $(kde_confdir)

conf_DATA = kdmrc
DISTCLEANFILES = kdmrc

kdmrc: kdmrc.cpp
	sed -e "s#KDEDIR#$(prefix)#" kdmrc.cpp > kdmrc



