dnl this is for kdm:

AC_DEFUN(AC_CHECK_KDM,
[
AC_CHECK_FUNCS(getsecretkey)
dnl checks for X server

AC_PATH_PROG(X_SERVER, X)
if test ! -z "$X_SERVER"; then
X_SERVER=`echo $X_SERVER | sed -e 's+/X$++'`
AC_DEFINE_UNQUOTED(XBINDIR,$X_SERVER)
XBINDIR=$X_SERVER
AC_SUBST(XBINDIR)
fi

dnl This one tries to find XDMDIR for config files
AC_ARG_WITH(xdmdir,
	[  --with-xdmdir	          If the xdm config dir can't be found automaticly],
	[ ac_xdmdir=$withval],
	[ ac_xdmdir="no"])

AC_MSG_CHECKING([for xdm configuration dir])
if test "$ac_xdmdir" = "no"; then
    rm -fr conftestdir
    if mkdir conftestdir; then
	cd conftestdir
    cat > Imakefile <<'EOF'
acfindxdm:
	@echo 'ac_xdmdir="$(XDMDIR)";'
EOF
	if (xmkmf) > /dev/null 2> /dev/null && test -f Makefile; then
	    eval `${MAKE-make} acfindxdm 2>/dev/null 2>/dev/null | grep -v make`
	fi
	cd ..
	rm -fr conftestdir
	dnl Check if Imake was right
	if test -f $ac_xdmdir/xdm-config; then
	    AC_MSG_RESULT($ac_xdmdir)
	else
	    dnl Here we must do something else
	    dnl Maybe look for xdm-config in standard places, and
	    dnl if that fails use a fresh copy in $KDEDIR/config/kdm/
	    AC_FIND_FILE(xdm-config,/etc/X11/xdm /var/X11/xdm /usr/openwin/xdm /usr/X11R6/lib/X11/xdm,ac_xdmdir)
	    if test -f $ac_xdmdir/xdm-config; then
                AC_MSG_RESULT($ac_xdmdir)
            else                                 
		if test "${prefix}" = NONE; then
			ac_xdmdir=$ac_default_prefix/config/kdm
		else
			ac_xdmdir=$prefix/config/kdm
		fi
		AC_MSG_RESULT([xdm config dir not found, installing defaults in $ac_xdmdir])
		xdmconfigsubdir=xdmconfig
		AC_SUBST(xdmconfigsubdir)
	    fi
	fi
    fi
else
    if test -f $ac_xdmdir/xdm-config; then
	AC_MSG_RESULT($ac_xdmdir)
    else

	AC_MSG_RESULT([xdm config dir not found, installing defaults in $ac_xdmdir])
	xdmconfigsubdir=xdmconfig
	AC_SUBST(xdmconfigsubdir)
    fi
fi
AC_DEFINE_UNQUOTED(XDMDIR,"$ac_xdmdir")
AC_SUBST(ac_xdmdir)

AC_PATH_PAM
if test "x$no_pam" = "xyes"; then 
	pam_support="no"
else
	pam_support="yes"
        shadow_support="no" # if pam is installed, use it. We can't savely 
	                    # test, if it works *sigh*
fi

AC_ARG_WITH(shadow,
	[  --with-shadow		  If you want shadow password support ],
	[ if test "$withval" = "yes"; then
             shadow_support="yes"
          else
             shadow_support="no"
          fi
	  if test "$pam_support" = "yes" && test "$shadow_support=yes"; then
		AC_MSG_WARN("You can not define both pam AND shadow")
	  fi
	],
	[ if test -z "$shadow_support"; then shadow_support="no"; fi ] )

if test "$pam_support" = "yes"; then
  AC_CHECK_LIB(pam, main, [PASSWDLIB="-lpam -ldl"
  AC_DEFINE_UNQUOTED(HAVE_PAM_LIB)],
  [],-ldl)
fi

if test -z "$PASSWDLIB" && test "$shadow_support" = "yes"; then
  AC_CHECK_LIB(shadow, main,
    [ PASSWDLIB="-lshadow"
      AC_DEFINE_UNQUOTED(HAVE_SHADOW_LIB)
    ])
fi
AC_SUBST(PASSWDLIB)
AC_CHECK_LIB(util, main, [LIBUTIL="-lutil"]) dnl for FreeBSD
AC_SUBST(LIBUTIL)
])

