
SUBDIRS = cs da de es fi fr it nl ru is sv hu pt pl el hr ro no ja \
	zh_TW.Big5 zh_CN.GB2312 sk ca sl en_UK mk pt_BR eo ko br tr he et hs

# this is a general list for merging!
POTFILES = kcmbell.pot kcminfo.pot kcminput.pot kcmkeys.pot kcmkfm.pot \
	kcmkpanel.pot kcmkwm.pot kcmlocale.pot kcmsamba.pot kcmsample.pot \
	kcmsyssound.pot kcontrol.pot kdehelp.pot kcmdisplay.pot kdm.pot \
	kdmconfig.pot kfind.pot kfm.pot kfontmanager.pot kikbd.pot \
	klock.pot kmenuedit.pot konsole.pot kpanel.pot krootwm.pot \
	kstart.pot kvt.pot kwm.pot kpager.pot kbgndwm.pot kthememgr.pot

EXTRA_DIST = $(POTFILES)

merge:
	@apps='$(POTFILES)'; \
	langs='$(SUBDIRS)'; \
	for cat in $$apps; do \
	  cat=`echo $$cat | sed -e 's#\.pot$$##'`; \
	  for lang in $$langs; do \
	    if test -r $$lang/$$cat.po; then \
	      echo $$cat; \
	      msgmerge -o $$cat.new $$lang/$$cat.po $$cat.pot; \
	      if test -s $$cat.new && diff $$lang/$$cat.po $$cat.new; then \
	        rm $$cat.new;  \
	      else  \
		mv $$cat.new $$lang/$$cat.po ; \
	    fi; \
	  fi ;\
         done ;\
	done

messages:


