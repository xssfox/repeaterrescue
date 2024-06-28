#include <stdint.h>
#include "Constants.h"

#define FEND 0xC0
#define FESC 0xDB
#define TFEND 0xDC
#define TFESC 0xDD

#define CMD_UNKNOWN 0xFE
#define CMD_DATA 0x00
#define CMD_PREAMBLE 0x01
#define CMD_P 0x02
#define CMD_SLOTTIME 0x03
#define CMD_TXTAIL 0x04
#define CMD_FULLDUPLEX 0x05
#define CMD_SETHARDWARE 0x06

#define MAX_PAYLOAD MTU_MAX

int kiss_serial_read(char sbyte);
int kiss_write_frame(char* buffer, int frame_len);
extern char frame_buffer[MAX_PAYLOAD];
extern char write_buffer[MAX_PAYLOAD*2+3];