
data_DATA = $(CONFIG_FILES) xdm-config Xresources Xservers Xaccess Xsession \
	GiveConsole TakeConsole Xsetup_0
datadir = $(ac_xdmdir)

CONFIG_FILES = Xservers.ws xdm-config Xservers Xsetup 
all-local: $(CONFIG_FILES)

CLEANFILES = $(CONFIG_FILES)

Xservers: Xservers.ws
	$(RM) -f Xservers
	$(LN_S) Xservers.ws Xservers

Xservers.ws: $(srcdir)/Xserv.ws.cpp
	BINDIR=$(XBINDIR) ;\
	sed -e "s#BINDIR#$$BINDIR#" $(srcdir)/Xserv.ws.cpp | sed -e '/^#  *[0-9][0-9]*  *.*$$/d' -e '/^XCOMM$$/s//#/' -e '/^XCOMM[^a-zA-Z0-9_]/s/^XCOMM/#/' > Xservers.ws

Xsetup: $(srcdir)/Xsetup_0
	$(RM) -f Xsetup
	$(LN_S) $(srcdir)/Xsetup_0 Xsetup

xdm-config: $(srcdir)/xdm-conf.cpp
	XDMDIR=$(ac_xdmdir) ;\
	sed -e "s#XDMDIR#$$XDMDIR#" $(srcdir)/xdm-conf.cpp |sed -e '/^#  *[0-9][0-9]*  *.*$$/d' -e '/^XCOMM$$/s//#/' -e '/^XCOMM[^a-zA-Z0-9_]/s/^XCOMM/#/'> xdm-config

