# Generated automatically from Makefile.in by configure.

KDEDIR=/home/kulow/prod/kde
mkinstalldirs = $(SHELL) ../../././mkinstalldirs
INSTALL		= /home/kulow/bin/install -c -m 0755 -s
INSTALL_SHELL	= /home/kulow/bin/install -c -m 0755
INSTALL_DATA	= /home/kulow/bin/install -c -m 0644
CONFIGDIR	= /home/kulow/prod/kde/config
CPP		= gcc -E -traditional

default: kdmrc

kdmrc:
	$(CPP) -DKDEDIR=$(KDEDIR) kdmrc.cpp > kdmrc

clean:
	$(RM) kdmrc core *~

install: kdmrc
	$(mkinstalldirs) $(CONFIGDIR)
	$(INSTALL_DATA) kdmrc $(CONFIGDIR)
