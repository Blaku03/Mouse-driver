#include <linux/uinput.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cmath>

// Ścieżka do oryginalnej myszy
#define ORIGINAL_MOUSE "/dev/input/event18"

// Funkcja akceleracji
int applyAcceleration(int value, double factor) {
  return value * abs(value);
}

// Funkcja konfiguracji wirtualnej myszy
int setupUinputDevice() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Nie można otworzyć /dev/uinput");
        return -1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);

    struct uinput_user_dev uidev = {};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Accelerated Mouse");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if (write(fd, &uidev, sizeof(uidev)) < 0) {
        perror("Błąd zapisu do uinput");
        close(fd);
        return -1;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("Nie można utworzyć urządzenia");
        close(fd);
        return -1;
    }

    return fd;
}

// Funkcja przesyłająca zmodyfikowane zdarzenia do wirtualnego urządzenia
void sendMouseEvent(int fd, int rel_x, int rel_y, int rel_wheel = 0) {
    struct input_event ev = {};

    if (rel_x != 0) {
        ev.type = EV_REL;
        ev.code = REL_X;
        ev.value = rel_x;
        write(fd, &ev, sizeof(ev));
    }

    if (rel_y != 0) {
        ev.type = EV_REL;
        ev.code = REL_Y;
        ev.value = rel_y;
        write(fd, &ev, sizeof(ev));
    }

    if (rel_wheel != 0) {
        ev.type = EV_REL;
        ev.code = REL_WHEEL;
        ev.value = rel_wheel;
        write(fd, &ev, sizeof(ev));
    }

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

int main() {
    // Otwórz oryginalne urządzenie myszy
    int mouse_fd = open(ORIGINAL_MOUSE, O_RDONLY);
    if (mouse_fd < 0) {
        perror("Nie można otworzyć oryginalnego urządzenia myszy");
        return 1;
    }

    // Utwórz wirtualną mysz
    int uinput_fd = setupUinputDevice();
    if (uinput_fd < 0) {
        close(mouse_fd);
        return 1;
    }

    struct input_event ev;
    double accelerationFactor = 2; // Współczynnik akceleracji

    std::cout << "Nakładanie akceleracji na ruch myszy" << std::endl;

    while (true) {
        ssize_t bytes = read(mouse_fd, &ev, sizeof(ev));
        if (bytes < (ssize_t)sizeof(ev)) {
            perror("Błąd odczytu z urządzenia");
            break;
        }

        if (ev.type == EV_REL) {
            if (ev.code == REL_X || ev.code == REL_Y) {
                // Nakładanie akceleracji
                int modified_value = applyAcceleration(ev.value, accelerationFactor);

                if (ev.code == REL_X) {
                    sendMouseEvent(uinput_fd, modified_value, 0);
                } else if (ev.code == REL_Y) {
                    sendMouseEvent(uinput_fd, 0, modified_value);
                }
            }
        } else if (ev.type == EV_KEY) {
            // Przekazywanie kliknięć
            write(uinput_fd, &ev, sizeof(ev));
        }
    }

    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
    close(mouse_fd);

    return 0;
}
