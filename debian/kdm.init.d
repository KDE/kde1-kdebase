#!/bin/bash
# /etc/init.d/kdm: start or stop XDM.

test -x /usr/bin/kdm || exit 0

test -f /etc/X11/kdm/config || exit 0

case "$1" in
  start)
    grep -q ^start-kdm /etc/X11/kdm/config || exit 0
    if grep -q ^start-xdm /etc/X11/kdm/config
      then
        echo "WARNING : can only start kdm or xdm, but not both !"
    fi

    echo -n "Starting kde display manager: kdm"    
    start-stop-daemon --start --quiet --exec /usr/bin/kdm
    echo "."
    ;;
  stop)
      echo -n "Stopping kde display manager: kdm"    
      start-stop-daemon --stop --quiet --pid /var/run/xdm.pid || echo " not running"
      echo "."
    ;;
# the last options are taken from kerneld
  restart) 
		$0 stop
		$0 start
    ;; 
  reload)
		start-stop-daemon --stop --signal 1 --q quiet --exec /usr/bin/kdm
 	;;
  force-reload)
		$0 reload 
	;;
  *)
    echo "Usage: /etc/init.d/kdm {start|stop}"
    exit 1
esac

exit 0
