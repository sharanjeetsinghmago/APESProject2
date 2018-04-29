#include <stdio.h>
#include <pthread.h>
#include <linux/ioctl.h>
#include "comm_task.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include "socket_task.h"


typedef struct packet
{
  uint8_t log_id;
  uint8_t log_level;
  float data;
  char timestamp[25];
  char c;
}log_packet;

log_packet rec;

float alti=0,humid=0;

float get_altitude();

void *func_comm();

void *func_logger();

void *func_alert();

int startup_test();

int main();