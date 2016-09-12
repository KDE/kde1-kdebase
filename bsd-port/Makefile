# New ports collection makefile for:   kdebase
# Version required:    1.1
# Date created:        18 June 1998
# Whom:                Alex Zepeda <garbanzo@hooked.net
#
# $Id: Makefile,v 1.7 1999/02/06 02:43:06 garbanzo Exp $
#
PKGNAME=	kdebase-`date "+%y%m%d"`
CATEGORIES=	x11 kde

MAINTAINER=	garbanzo@hooked.net

BUILD_DEPENDS=	autoconf:${PORTSDIR}/devel/autoconf \
		automake:${PORTSDIR}/devel/automake
INSTALL_ROOT=	${.CURDIR}/${PKGNAME}
USE_QT=		yes
LIB_DEPENDS=    gif.3:${PORTSDIR}/graphics/giflib \
                jpeg.9:${PORTSDIR}/graphics/jpeg \
                kdecore.2:${PORTSDIR}/x11/kdelibs11 \
                Xpm.4:${PORTSDIR}/graphics/xpm
 

LIB_DEPENDS=	gif.3]:${PORTSDIR}/graphics/giflib \
		jpeg.9:${PORTSDIR}/graphics/jpeg \
		kdecore.2:${PORTSDIR}/x11/kdelibs11 \
		kdeui.2:${PORTSDIR}/x11/kdelibs11 \
		jscript.2:${PORTSDIR}/x11/kdelibs11 \
		khtmlw.2:${PORTSDIR}/x11/kdelibs11 \
		kfm.2:${PORTSDIR}/x11/kdelibs11 \
		mediatool.2:${PORTSDIR}/x11/kdelibs11
NO_MTREE=	yup
USE_GMAKE=	"without a doubt"
NO_WRKSUBDIR=	"Cause, you've obviously got the source in the parent directory"
WRKSRC=		../
EXTRACT_ONLY=	"yes, please"
HAS_CONFIGURE=	yes
CONFIGURE_ARGS=	"--prefix=$(PREFIX)" \
		"--x-inc=$(X11BASE)/include" \
		"--x-lib=$(X11BASE)/lib" \
		"--with-install-root=$(INSTALL_ROOT)" \
		"--with-extra-includes=$(PREFIX)/include/giflib"
INSTALL_TARGET=	RUN_KAPPFINDER=no install

.include "kde-snapshot.mk"
.include <bsd.port.mk>
