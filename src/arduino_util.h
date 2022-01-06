#include <libserialport.h>

#ifndef INCLUDE_ARDUINO_UTIL_H
#define INCLUDE_ARDUINO_UTIL_H

#define METRO_MINI_VID 0x10c4
#define METRO_MINI_PID 0xea60

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int show_config(struct sp_port *port);
int set_arduino_config(struct sp_port *port, int baudrate);
int find_metro_mini(struct sp_port **port);
void print_error(enum sp_return err);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
