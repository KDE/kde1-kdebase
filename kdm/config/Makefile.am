
datadir	= @prefix@/share/config

data_DATA = kdmrc
DISTCLEANFILES = kdmrc

kdmrc: kdmrc.cpp
	sed -e "s#KDEDIR#$(prefix)#" kdmrc.cpp > kdmrc



