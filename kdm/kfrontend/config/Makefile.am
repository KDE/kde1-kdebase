
data_DATA = $(BUILT_SOURCES) xdm-config Xresources Xservers Xaccess Xsession \
	GiveConsole TakeConsole Xsetup_0
datadir = $(ac_xdmdir)
BUILT_SOURCES = Xservers.ws xdm-config Xservers Xsetup
CLEANFILES = $(BUILT_SOURCES)

Xservers: Xservers.ws
	$(RM) -f Xservers
	$(LN_S) Xservers.ws Xservers

Xservers.ws: Xserv.ws.cpp
	BINDIR=$(XBINDIR) ;\
	sed -e "s#BINDIR#$$BINDIR#" Xserv.ws.cpp | sed -e '/^#  *[0-9][0-9]*  *.*$$/d' -e '/^XCOMM$$/s//#/' -e '/^XCOMM[^a-zA-Z0-9_]/s/^XCOMM/#/' > Xservers.ws

Xsetup: Xsetup_0
	$(RM) -f Xsetup
	$(LN_S) Xsetup_0 Xsetup

xdm-config: xdm-conf.cpp
	XDMDIR=$(ac_xdmdir) ;\
	sed -e "s#XDMDIR#$$XDMDIR#" xdm-conf.cpp |sed -e '/^#  *[0-9][0-9]*  *.*$$/d' -e '/^XCOMM$$/s//#/' -e '/^XCOMM[^a-zA-Z0-9_]/s/^XCOMM/#/'> xdm-config

