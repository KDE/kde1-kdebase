# Generated automatically from Makefile.in by configure.

mkinstalldirs = $(SHELL) ../../././mkinstalldirs
INSTALL		= /home/kulow/bin/install -c -m 0755 -s
INSTALL_SHELL	= /home/kulow/bin/install -c -m 0755
INSTALL_DATA	= /home/kulow/bin/install -c -m 0644
CPP		= gcc -E -traditional
XDMDIR		= /home/kulow/prod/kde/config/kdm
BINDIR		= 

all:: Xservers.ws xdm-config Xservers

Xservers: Xservers.ws
	$(RM) -f Xservers
	ln -s Xservers.ws Xservers

Xservers.ws: Xserv.ws.cpp
	$(CPP) -DBINDIR=$(BINDIR) Xserv.ws.cpp | sed -e '/^#  *[0-9][0-9]*  *.*$$/d' -e '/^XCOMM$$/s//#/' -e '/^XCOMM[^a-zA-Z0-9_]/s/^XCOMM/#/' > Xservers.ws

xdm-config: xdm-conf.cpp
	$(CPP) -DXDMDIR=$(XDMDIR) xdm-conf.cpp |sed -e '/^#  *[0-9][0-9]*  *.*$$/d' -e '/^XCOMM$$/s//#/' -e '/^XCOMM[^a-zA-Z0-9_]/s/^XCOMM/#/'> xdm-config

install: all
	$(mkinstalldirs) $(XDMDIR)
	$(INSTALL_DATA) xdm-config $(XDMDIR)
	$(INSTALL_DATA) Xresources $(XDMDIR)
	$(INSTALL_DATA) Xservers $(XDMDIR)
	$(INSTALL_DATA) Xaccess $(XDMDIR)
	$(INSTALL_SHELL) Xsession $(XDMDIR)
	$(INSTALL_SHELL) GiveConsole $(XDMDIR)
	$(INSTALL_SHELL) TakeConsole $(XDMDIR)
	$(INSTALL_SHELL) Xsetup_0 $(XDMDIR)

clean:
	$(RM) core *~ Xservers Xservers.ws xdm-config
