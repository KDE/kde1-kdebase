#!/bin/bash
# /etc/init.d/xdm: start or stop XDM.

test -x /usr/bin/X11/xdm || exit 0

test -f /etc/X11/config || exit 0

if grep -q ^xbase-not-configured /etc/X11/config
then
  exit 0
fi

run_kdm=0
if grep -q ^start-kdm /etc/X11/config
then
  run_kdm=1
fi

case "$1" in
  start)
    if [ $run_kdm = 1 ]
    then
      if grep -q ^start-xdm /etc/X11/config
      then
        echo "WARNING : can only start kdm or xdm, but not both !"
      else
	if test /etc/X11/window-managers -nt /etc/kde/kdmrc 
	then
		echo "Updating kdmrc ..."
		(echo -n "s/WINDOWMANAGER/" ; \
		cat /etc/X11/window-managers  |grep -v ^#|while read A ; \
		do echo -n "`basename $A`;" ;  \
		done ; \
		echo "/") > /etc/kde/kdmrc.tmp
		sed `cat /etc/kde/kdmrc.tmp` \
			< /etc/kde/kdmrc.in > /etc/kde/kdmrc
		rm /etc/kde/kdmrc.tmp
	fi
        echo -n "Starting kde display manager: kdm"    
        start-stop-daemon --start --quiet --exec /usr/X11R6/bin/kdm
        echo "."
      fi
    fi
    ;;
  stop)
      echo -n "Stopping kde display manager: kdm"    
      start-stop-daemon --stop --quiet --exec /usr/X11R6/bin/kdm \
		--pidfile /var/run/xdm-pid
      echo "."
    ;;
  *)
    echo "Usage: /etc/init.d/kdm {start|stop}"
    exit 1
esac

exit 0
