# New ports collection makefile for:   kdeutils
# Version required:    Beta4
# Date created:        28 October 1997
# Whom:                Stefan Eßer <se@freebsd.org>
#
# $Id$
#
PKGNAME=	kdeutils-`date "+%y%m%d"`
CATEGORIES=	misc kde

MAINTAINER=	garbanzo@hooked.net

BUILD_DEPENDS=	moc:${PORTSDIR}/x11/qt \
		autoconf:${PORTSDIR}/devel/autoconf \
		automake:${PORTSDIR}/devel/automake
INSTALL_ROOT?=	${.CURDIR}/${PKGNAME}
LIB_DEPENDS=	qt\\.1\\.3[0-9]:${PORTSDIR}/x11/qt \
		gif\\.[23]\\.:${PORTSDIR}/graphics/giflib \
		jpeg\\.8\\.:${PORTSDIR}/graphics/jpeg \
		kdecore\\.0\\.9[1-9]:${PORTSDIR}/x11/kdelibs \
		kdeui\\.0\\.9[1-9]:${PORTSDIR}/x11/kdelibs \
		jscript\\.0\\.9[0-9]:${PORTSDIR}/x11/kdelibs \
		khtmlw\\.0\\.9[1-9]:${PORTSDIR}/x11/kdelibs \
		kfm\\.0\\.9[0-9]:${PORTSDIR}/x11/kdelibs \
		mediatool\\.0\\.9[1-9]:${PORTSDIR}/x11/kdelibs
NO_MTREE=	yup
USE_GMAKE=	"without a doubt"
NO_WRKDIR=	"Cause, you've obviously got the source in the parent directory"
WRKSRC=		../
NO_EXTRACT=	"yes, please"
HAS_CONFIGURE=	yes
CONFIGURE_ARGS=	"--prefix=$(PREFIX)" \
		"--x-inc=$(X11BASE)/include" \
		"--x-lib=$(X11BASE)/lib" \
		"--with-install-root=$(INSTALL_ROOT)" \
		"--with-extra-includes=$(PREFIX)/include/giflib"

CONFIGURE_ENV=	CXXFLAGS="$(CFLAGS)" \
		install_root=$(INSTALL_ROOT) \
		INSTALL_SCRIPT="install -c -m 555"

# Since there's nothing to fetch, we might as well use a dummy target
do-fetch:
		@true
# This should clean the KDE target pretty well
pre-clean:
		cd $(WRKSRC);$(GMAKE) clean   

# We need to go through Makefile.cvs before anything else.
pre-configure:
		cd $(WRKSRC);rm -f config.cache;$(GMAKE) -f Makefile.cvs
		rm -f $(PLIST)

make-plist:
		cd ${INSTALL_ROOT} && \
			find . -type f|sed 's,^\./,,'|sort > $(PLIST)
		cd ${INSTALL_ROOT} && \
			find . -type l|sed 's,^\./,,'|sort > $(PLIST)
		@echo "@exec /sbin/ldconfig -m %B" >> $(PLIST)
		cd ${INSTALL_ROOT} && \
			find . -type d|sed 's,^\./,@dirrm ,'|sort > $(PLIST)
		@echo "@unexec /sbin/ldconfig -R" >> $(PLIST)

.include <bsd.port.mk>
