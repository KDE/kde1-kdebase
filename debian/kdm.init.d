#!/bin/bash
# /etc/init.d/kdm: start or stop XDM.

test -x /usr/bin/kdm || exit 0

test -f /etc/X11/kdm/config || exit 0

grep -q ^start-kdm /etc/X11/kdm/config || exit 0
if grep -q ^start-xdm /etc/X11/kdm/config
then
  echo "WARNING : can only start xdm or kdm, but not both !"
fi

if grep -qs ^check-local-xserver /etc/X11/xdm/xdm.options; then
  if head -1 /etc/X11/Xserver 2> /dev/null | grep -q Xsun; then
    # the Xsun X servers do not use XF86Config
    CHECK_LOCAL_XSERVER=
  else
    CHECK_LOCAL_XSERVER=yes
  fi
fi

case "$1" in
  start)
    if [ "$CHECK_LOCAL_XSERVER" ]; then
      problem=yes
      echo -n "Checking for valid XFree86 server configuration..."
      if [ -e /etc/X11/XF86Config ]; then
        if [ -x /usr/sbin/parse-xf86config ]; then
          if parse-xf86config --quiet --nowarning --noadvisory /etc/X11/XF86Config; then
            problem=
          else
            echo "error in configuration file."
          fi
        else
          echo "unable to check."
        fi
      else
        echo "file not found."
      fi
      if [ "$problem" ]; then
        echo "Not starting X display manager."
        exit 1
      else
        echo "done."
      fi
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
