#$Id$

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
post-install:
		${MAKE} PREFIX=${PREFIX} make-plist
		cp -R ${INSTALL_ROOT}/* /
		rm -rf ${INSTALL_ROOT}

# This is still imperfect, as it _still_ 
# adds a null item to the plist.  *sigh* Some things don't change.
make-plist:
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type f|sed 's,^\./,,'|sort > $(PLIST)
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type l|sed 's,^\./,,'|sort >> $(PLIST)
		@echo "@exec /sbin/ldconfig -m %B" >> $(PLIST)
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type d|sed 's,^\./,@dirrm ,'|sort >> $(PLIST)
		@echo "@unexec /sbin/ldconfig -R" >> $(PLIST)
