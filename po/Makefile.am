 
SUBDIRS = cs da de es fi fr it nl ru sv hu pt pl el hr ro no zh_TW_Big5 zh_GB2312 sk ca sl

# this is a general list for merging!
APPS = kdehelp kdisplay kdm kfind kfm kfontmanager kpanel kvt kwm klock \
	krootwm kcmbell kcminfo kcminput kcmkwm kcmsamba kcmsample kcontrol \
	kmenuedit kcmkpanel kcmlocale kcmsyssound kstartondesk

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
