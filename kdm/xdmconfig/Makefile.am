
data_DATA = $(BUILT_SOURCES) xdm-config Xresources Xservers Xaccess Xsession \
	GiveConsole TakeConsole Xsetup_0
datadir = $(ac_xdmdir)
BUILT_SOURCES = Xservers.ws xdm-config Xservers
CLEANFILES = $(BUILT_SOURCES)

Xservers: Xservers.ws
	$(RM) -f Xservers
	$(LN_S) Xservers.ws Xservers

Xservers.ws: Xserv.ws.cpp
	sed -e "s#BINDIR#$(XBINDIR)#" Xserv.ws.cpp | sed -e '/^#  *[0-9][0-9]*  *.*$$/d' -e '/^XCOMM$$/s//#/' -e '/^XCOMM[^a-zA-Z0-9_]/s/^XCOMM/#/' > Xservers.ws

xdm-config: xdm-conf.cpp
	sed -e "s#XDMDIR#$(XDMDIR)#" xdm-conf.cpp |sed -e '/^#  *[0-9][0-9]*  *.*$$/d' -e '/^XCOMM$$/s//#/' -e '/^XCOMM[^a-zA-Z0-9_]/s/^XCOMM/#/'> xdm-config

