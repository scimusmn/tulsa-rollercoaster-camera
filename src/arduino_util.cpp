#include <libserialport.h>
#include <stdio.h>
#include "arduino_util.h"
#include "logging.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define CHECK_ERR()					\
   do {							\
      if (err != SP_OK) {				\
	 char *last_error = sp_last_error_message();	\
	 log_msg(ERROR, "(%s:%d) %s", __FILE__, __LINE__, last_error);	\
	 sp_free_error_message(last_error);		\
	 return 1;					\
      }							\
   } while(0)

static const char* parity(enum sp_parity p);
static const char* rts(enum sp_rts r);
static const char* cts(enum sp_cts c);
static const char* dtr(enum sp_dtr d);
static const char* dsr(enum sp_dsr d);
static const char* xon_xoff(enum sp_xonxoff x);

int show_config(struct sp_port* port) {
  enum sp_return err;

  // load port configuration
  struct sp_port_config* config;
  sp_new_config(&config);
  sp_get_config(port, config);

  // get configuration variables
  int baudrate;
  int bits;
  enum sp_parity p;
  int stop_bits;
  enum sp_rts r;
  enum sp_cts c;
  enum sp_dtr dt;
  enum sp_dsr ds;
  enum sp_xonxoff x;

  err = sp_get_config_baudrate(config, &baudrate);
  CHECK_ERR();
  err = sp_get_config_bits    (config, &bits);
  CHECK_ERR();
  err = sp_get_config_parity  (config, &p);
  CHECK_ERR();
  err = sp_get_config_stopbits(config, &stop_bits);
  CHECK_ERR();
  err = sp_get_config_rts     (config, &r);
  CHECK_ERR();
  err = sp_get_config_cts     (config, &c);
  CHECK_ERR();
  err = sp_get_config_dtr     (config, &dt);
  CHECK_ERR();
  err = sp_get_config_dsr     (config, &ds);
  CHECK_ERR();
  err = sp_get_config_xon_xoff(config, &x);
  CHECK_ERR();

  // print configuration variables
  log_msg(INFO, "Port configuration:\n"
	  "  Baudrate: %d\n"
	  "  Data bits: %d\n"
	  "  Parity: %s\n"
	  "  Stop bits: %d\n"
	  "  RTS: %s\n"
	  "  CTS: %s\n"
	  "  DTR: %s\n"
	  "  DSR: %s\n"
	  "  XON/XOFF: %s",
	  baudrate, bits,
	  parity(p), stop_bits,
	  rts(r), cts(c),
	  dtr(dt), dsr(ds),
	  xon_xoff(x));
  
  sp_free_config(config);
  return 0;
}

static const char* parity(enum sp_parity p)
{
   switch(p) {
   case SP_PARITY_INVALID:
    return "INVALID";
  case SP_PARITY_NONE:
      return "None";
   case SP_PARITY_ODD:
      return "Odd";
   case SP_PARITY_EVEN:
      return "Even";
   case SP_PARITY_MARK:
      return "Mark";
   case SP_PARITY_SPACE:
      return "Space";
   default:
      return "Unknown";
   } 
}

static const char* rts(enum sp_rts r)
{
   switch(r) {
   case SP_RTS_INVALID:
      return "INVALID";
   case SP_RTS_OFF:
      return "Off";
   case SP_RTS_ON:
      return "On";
   case SP_RTS_FLOW_CONTROL:
      return "Flow control";
   default:
      return "Unknown";
   }
}

static const char* cts(enum sp_cts c)
{
   switch(c) {
   case SP_CTS_INVALID:
      return "INVALID";
   case SP_CTS_IGNORE:
      return "Ignore";
   case SP_CTS_FLOW_CONTROL:
      return "Flow control";
   default:
      return "Unknown";
   }
}

static const char* dtr(enum sp_dtr d)
{
   switch(d) {
   case SP_DTR_INVALID:
      return "INVALID";
   case SP_DTR_OFF:
      return "Off";
   case SP_DTR_ON:
      return "On";
   case SP_DTR_FLOW_CONTROL:
      return "Flow control";
   default:
      return "Unknown";
   }
}

static const char* dsr(enum sp_dsr d)
{
   switch(d) {
   case SP_DSR_INVALID:
      return "INVALID";
   case SP_DSR_IGNORE:
      return "Ignore";
   case SP_DSR_FLOW_CONTROL:
      return "Flow control";
   default:
      return "Unknown";
   }
}

static const char* xon_xoff(enum sp_xonxoff x)
{
   switch(x) {
   case SP_XONXOFF_INVALID:
      return "INVALID";
   case SP_XONXOFF_DISABLED:
      return "Disabled";
   case SP_XONXOFF_IN:
      return "In";
   case SP_XONXOFF_OUT:
      return "Out";
   case SP_XONXOFF_INOUT:
      return "In/Out";
   default:
      return "Unknown";
   }
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


int set_arduino_config(struct sp_port* port, int baudrate) {
  enum sp_return err;

  err = sp_set_baudrate(port, baudrate);
  CHECK_ERR();
  err = sp_set_bits(port, 8);
  CHECK_ERR();
  err = sp_set_parity(port, SP_PARITY_NONE);
  CHECK_ERR();
  err = sp_set_stopbits(port, 1);
  CHECK_ERR();
  err = sp_set_rts(port, SP_RTS_ON);
  CHECK_ERR();
  err = sp_set_cts(port, SP_CTS_IGNORE);
  CHECK_ERR();
  err = sp_set_dtr(port, SP_DTR_ON);
  CHECK_ERR();
  err = sp_set_dsr(port, SP_DSR_IGNORE);
  CHECK_ERR();
  err = sp_set_xon_xoff(port, SP_XONXOFF_DISABLED);
  CHECK_ERR();

  return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


int find_metro_mini(struct sp_port **port)
{
   enum sp_return err;
   struct sp_port **list;
   err = sp_list_ports(&list);
   CHECK_ERR();

   for (int i=0; list[i] != NULL; i++) {
      struct sp_port *p = list[i];
      int vid, pid;
      err = sp_get_port_usb_vid_pid(p, &vid, &pid);
      CHECK_ERR();

      if (vid == METRO_MINI_VID && pid == METRO_MINI_PID) {
	 err = sp_copy_port(p, port);
	 CHECK_ERR();
	 sp_free_port_list(list);
	 return 0;
      }
   }

   log_msg(WARN, "did not find any Metro Mini device!");

   sp_free_port_list(list);
   return 1;
}
