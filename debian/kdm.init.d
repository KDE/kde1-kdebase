#!/bin/bash
# /etc/init.d/xdm: start or stop XDM.

test -x /usr/bin/X11/xdm || exit 0

test -f /etc/X11/config || exit 0

grep -q ^xbase-not-configured /etc/X11/config && exit 0

case "$1" in
  start)
    grep -q ^start-kdm /etc/X11/config || exit 0
    if grep -q ^start-xdm /etc/X11/config
      then
        echo "WARNING : can only start kdm or xdm, but not both !"
    fi

    echo -n 's/^SessionTypes=.*/SessionTypes=' > /tmp/kdm.sed
    cat /etc/X11/window-managers |while read A; 
    do
        if test -x "$A"
        then
    	    echo -n "`basename $A`;" >> /tmp/kdm.sed
        fi
    done
    echo -n "failsafe;/" >> /tmp/kdm.sed
    cp /etc/kde/kdmrc /tmp/kdmrc.old
    sed -f /tmp/kdm.sed < /tmp/kdmrc.old > /etc/kde/kdmrc
    rm -f /tmp/kdm.sed /tmp/kdmrc.old

    echo -n "Starting kde display manager: kdm"    
    start-stop-daemon --start --quiet --exec /usr/bin/X11/kdm
    echo "."
    ;;
  stop)
      echo -n "Stopping kde display manager: kdm"    
      start-stop-daemon --stop --quiet --exec /usr/bin/X11/kdm \
		--pidfile /var/run/xdm-pid
      echo "."
    ;;
# the last options are taken from kerneld
  restart) 
		$0 stop
		$0 start
    ;; 
  reload)
		start-stop-daemon --stop --signal 1 --q quiet --exec /usr/bin/X11/kdm
 	;;
  force-reload)
		$0 reload 
	;;
  *)
    echo "Usage: /etc/init.d/kdm {start|stop}"
    exit 1
esac

exit 0
