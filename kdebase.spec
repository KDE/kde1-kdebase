%define	version Beta3
%define name kdebase
Name: %{name}
Summary: K Desktop Environment - core files
Version: %{version}
Release: 1
Source: ftp.kde.org:/pub/kde/unstable/CVS/snapshots/current/%{name}-%{version}.tar.gz
Group: X11/KDE/Base
Copyright: GPL
BuildRoot: /tmp/realhot_%{name}
Requires: qt >= 1.31
Distribution: KDE
Packager: Magnus Pfeffer <pfeffer@unix-ag.uni-kl.de>
Vendor: The KDE Team

%description
Core applications for the K Desktop Environment.

Included with this package are:
kaudio: audio server
kcontrol: configuration tool
kdehelp: viewer for kde help files, info amd man pages
kdm: replacement for xdm
kfind: find tool
kfm: filemanager, web browser, ftp client, ...
kfontmanager: font selector
kmenuedit: tool to add applications to the panel
kpanel: file starter and desktop pager
kscreensaver: serveral screen savers
kvt: xterm replacement
kwm: kde window manager
kappfinder: non-kde application finder

%prep
rm -rf $RPM_BUILD_ROOT

%setup -n kdebase

%build
export KDEDIR=/opt/kde
./configure --prefix=$KDEDIR --with-install-root=$RPM_BUILD_ROOT --without-gl
make

%install
make RUN_KAPPFINDER=no install

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.%{name}
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.%{name}
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.%{name}

%clean
rm -rf $RPM_BUILD_ROOT

%files -f ../file.list.%{name}
