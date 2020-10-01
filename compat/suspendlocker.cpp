#include <systemd/sd-bus.h>
#include <kscreensave.h>

static int onSleeping(sd_bus_message *message, void *userdata, sd_bus_error *ret_error)
{
    int goingToSleep;

    int ret = sd_bus_message_read(message, "b", &goingToSleep);
    if (ret < 0) {
        fprintf(stderr, "Failed to read argument: %s\n", strerror(-ret));
        return 0;
    }

    kForceLocker();
    return 0;
}

int main(int argc, char *argv[])
{
    sd_bus *bus = nullptr;
    int ret = sd_bus_default_system(&bus);
    if (ret < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-ret));
        return 1;
    }

    ret = sd_bus_match_signal( bus,
            nullptr, // slot
            "org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            "PrepareForSleep",
            onSleeping,
            nullptr // userdata
        );
    if (ret < 0) {
        fprintf(stderr, "Failed to connect to logind signal: %s\n", strerror(-ret));
        return 1;
    }

    while (true) {
        int events = sd_bus_wait(bus, UINT64_MAX);
        if (events < 0) {
            fprintf(stderr, "Failed to wait for dbus event: %s\n", strerror(-events));
            return 1;
        }

        do {
            events = sd_bus_process(bus, nullptr);
        } while (events > 0);
        if (events < 0) {
            fprintf(stderr, "Failed to process dbus event: %s\n", strerror(-events));
            return 1;
        }
    }

    sd_bus_unref(bus);

    return 0;
}
