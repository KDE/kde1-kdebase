 
SUBDIRS = cs da de es fi fr it nl ru is sv hu pt pl el hr ro no \
	zh_TW.Big5 zh_CN.GB2312 sk ca sl en_UK mk pt_BR eo ko

# this is a general list for merging!
APPS = kcmbell kcminfo kcminput kcmkeys kcmkfm kcmkpanel kcmkwm \
	kcmlocale kcmsamba kcmsample kcmsyssound kcontrol kdehelp \
	kdisplay kdm kdmconfig kfind kfm kfontmanager kikbd \
	klock kmenuedit konsole kpanel krootwm kstart kvt kwm \
	kpager

merge:
	@apps='$(APPS)'; \
	langs='$(SUBDIRS)'; \
	for cat in $$apps; do \
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
