/* ----------------------------------------------------------------------
 *
 *          SuspendLocker - Modernizing wrapper for kscreensave
 *
 *                 Copyright (C) 2020 Martin Sandsmark
 *                       martin.sandsmark@kde.org
 *
 * -----------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * -----------------------------------------------------------------------
 * Description:
 *   Simple wrapper thing for kscreensaver that locks the screen on either a
 *   XScreenSaver event (which wasn't available when kde1 was written), or when
 *   the device is going to sleep (dbus and loginmanager was not available when
 *   kde1 was written either).
 *
 */

#include <kscreensave.h>

#include <systemd/sd-bus.h>

#include <thread>
#include <mutex>
#include <functional>

#include <unistd.h>

#include <X11/Xatom.h> // XA_INTEGER
#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

struct Handler {

    static int dbusCallback(sd_bus_message *message, void *userdata, sd_bus_error *ret_error);

    int onSleeping(sd_bus_message *message, sd_bus_error *ret_error);

    bool initX();
    void runXLoop();
    void signalStopXLoop();

    void runDBusLoop();
    void signalStopDBusLoop();

    std::unique_ptr<std::thread> dbusThread;
    std::mutex lockMutex;

    bool running = true;

    // X11 stuff
    int screen = 0;
    Window dummyWindow = 0;
    Display *display = nullptr;
    int ss_event, ss_error;
};

int Handler::onSleeping(sd_bus_message *message, sd_bus_error *ret_error)
{
    int goingToSleep;

    int ret = sd_bus_message_read(message, "b", &goingToSleep);
    if (ret < 0) {
        fprintf(stderr, "Failed to read argument: %s\n", strerror(-ret));
        return 0;
    }
    if (!goingToSleep) {
        puts("Woke from sleep");
        return 0;
    }
    puts("Going to sleep");

    std::lock_guard<std::mutex> lock(lockMutex);
    kForceLocker();

    return 0;
}

int Handler::dbusCallback(sd_bus_message *message, void *userdata, sd_bus_error *ret_error)
{
    Handler *that = reinterpret_cast<Handler*>(userdata);
    return that->onSleeping(message, ret_error);
}

void Handler::runDBusLoop()
{
    sd_bus *bus = nullptr;
    int ret = sd_bus_default_system(&bus);
    if (ret < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-ret));
        return;
    }

    ret = sd_bus_match_signal( bus,
            nullptr, // slot
            "org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            "PrepareForSleep",
            &Handler::dbusCallback,
            this // userdata
        );

    if (ret < 0) {
        fprintf(stderr, "Failed to connect to logind signal: %s\n", strerror(-ret));
        sd_bus_unref(bus);
        return;
    }
    puts("Starting dbus thread");

    while (running) {
        int events = sd_bus_wait(bus, UINT64_MAX);

        {
            std::lock_guard<std::mutex> lock(lockMutex);
            if (!running) {
                break;
            }
        }

        if (events < 0) {
            fprintf(stderr, "Failed to wait for dbus event: %s\n", strerror(-events));
            break;
        }

        do {
            events = sd_bus_process(bus, nullptr);
        } while (events > 0);
        if (events < 0) {
            fprintf(stderr, "Failed to process dbus event: %s\n", strerror(-events));
            break;
        }
    }

    sd_bus_unref(bus);
    puts("DBus thread stopped");
}

static bool s_xError = false;
static int s_xErrorHandler(Display *dpy, XErrorEvent *err)
{
    s_xError = true;

    char buf[256];
    XGetErrorText(dpy, err->error_code, buf, 255);
    printf("\tX error: %s\n", buf);
    return 0;
}


bool Handler::initX()
{
    display = XOpenDisplay(nullptr);
    if (!display || s_xError) {
        puts("Failed to open display");
        return false;
    }
    screen = DefaultScreen(display);

    if (!XScreenSaverQueryExtension(display, &ss_event, &ss_error) || s_xError) {
      puts("X server does not support MIT-SCREEN-SAVER extension.");
      return false;
    }
    if (!XScreenSaverRegister(display, screen, (XID)getpid(), XA_INTEGER) || s_xError) {
      puts("cannot register screen saver, is another one already running?");
      return false;
    }
    Window root = RootWindow(display, screen);
    XScreenSaverSelectInput(display, root, ScreenSaverNotifyMask);


    Pixmap pmap = XCreateBitmapFromData(display, root, "\0", 1, 1);
    if (s_xError) {
        puts("Failed to create pixmap");
        return false;
    }

    XSetWindowAttributes wa{};
    wa.background_pixel = BlackPixel(display, screen);

    if (!(XFreePixmap(display, pmap)) || s_xError) {
        puts("Failed to free pixmap");
        return false;
    }

    XScreenSaverSetAttributes(display, root,
            -1, -1, // x, y
             1,  1,   // width, height
            0,      // border_width
            CopyFromParent, // depth
            CopyFromParent, // class
            CopyFromParent, // visual
            CWBackPixel, // valuemask
            &wa // attributes
            );
    if (s_xError) {
        puts("Failed to set screensaver attributes");
        return false;
    }
    XScreenSaverInfo info;
    XScreenSaverQueryInfo(display, (Drawable)root, &info);
    if (s_xError) {
        puts("Another screensaver probably already running");
        return false;
    }
    dummyWindow = info.window;

    return true;
}

void Handler::signalStopDBusLoop()
{
    {
        std::lock_guard<std::mutex> lock(lockMutex);
        running = false;
    }
    puts("Stopping dbus");

    // Yeah yeah, worst practice
    pthread_kill(dbusThread->native_handle(), SIGUSR1);
}

void Handler::signalStopXLoop()
{
    puts("Stopping X loop");
    {
        std::lock_guard<std::mutex> lock(lockMutex);
        running = false;
    }

    XClientMessageEvent dummyEvent;
    memset(&dummyEvent, 0, sizeof(XClientMessageEvent));
    dummyEvent.type = ClientMessage;
    dummyEvent.window = dummyWindow;
    dummyEvent.format = 32;
    XSendEvent(display, dummyWindow, 0, 0, (XEvent*)&dummyEvent);
    XFlush(display);
}

void Handler::runXLoop()
{
    XEvent event;
    while (!XNextEvent(display, &event) ) {
        {
            std::lock_guard<std::mutex> lock(lockMutex);
            if (!running) {
                break;
            }
        }

        if (event.type == ss_event) {
            puts("Got screensaver event");

            XScreenSaverNotifyEvent *sevent = (XScreenSaverNotifyEvent *)&event;

            if (sevent->state != ScreenSaverOn) {
                puts("Was not ScreenSaverOn, probably woke.");
                continue;
            }

            std::lock_guard<std::mutex> lock(lockMutex);
            kForceLocker();
            continue;
        }

        if (event.type == ClientMessage) {
            puts("Shutting down X loop");
            break;
        }
    }

    XScreenSaverUnregister(display, screen);
    XCloseDisplay(display);

    puts("X loop stopped");
}


int main(int argc, char *argv[])
{
    Handler handler;

    static int (*oldXErrorHandler) (Display *, XErrorEvent *) =
        XSetErrorHandler(s_xErrorHandler);

    if (!handler.initX()) {
        puts("Initializing X failed");
        return 2;
    }
    XSetErrorHandler(oldXErrorHandler);

    puts("Inited XScreenSaver");

    handler.dbusThread = std::make_unique<std::thread>([&handler]() {
        handler.runDBusLoop();
        handler.signalStopXLoop();
    });

    handler.runXLoop();

    if (handler.dbusThread->joinable()) {
        handler.signalStopDBusLoop();
        handler.dbusThread->join();
    }

    return 0;
}
