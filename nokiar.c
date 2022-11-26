#include <error.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

void reset(void) {
  system("raspi-gpio set 23 op dh");
  usleep(50);
  system("raspi-gpio set 23 op dl");
  usleep(50);
}

void dc(unsigned char p) {
  if (p == 0)
    system("raspi-gpio set 24 op dl");
  else
    system("raspi-gpio set 24 op dh");
}

void ce(unsigned char p) {
return;
  if (p == 0)
    system("raspi-gpio set 25 op dl");
  else
    system("raspi-gpio set 25 op dh");
}

#define DATA dc(1)
#define COMMAND dc(0)

/*
   5  reset
   6  dc
   13 lcd
*/

int configure_display(void) {
  int file;

  if ((file = open("/dev/spidev0.0", O_RDWR)) < 0) {
    perror("open");
    exit(1);
  }

 // struct spi_ioc_transfer xfer;
 // memset(&xfer, 0, sizeof xfer);

struct spi_ioc_transfer xfer =
        {
            .tx_buf = 0,
            .rx_buf = 0,
            .len = 0,
            .delay_usecs = 20,
            .speed_hz = 100000,
        };

  unsigned char buf[512];

  memset(buf, 0xaa, sizeof buf);

  static __u32 speed = 10000;
  if (ioctl(file, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
    perror("ioctl");
    exit(1);
  }

  reset();

  xfer.tx_buf = (__u64)buf;
  xfer.len = 4;
  buf[0] = 0x21;
  buf[1] = 0xb2;
  buf[2] = 0x13;
  buf[3] = 0x20;
  COMMAND;
  ce(0);
  if (ioctl(file, SPI_IOC_MESSAGE(1), &xfer) < 0) {
    perror("ioctl");
    exit(1);
  }
  ce(1);

  memset(buf, 0xaa, sizeof buf);
  xfer.tx_buf = (__u64)buf;
  xfer.len = 5 * 84;
  DATA;
  ce(0);
  if (ioctl(file, SPI_IOC_MESSAGE(1), &xfer) < 0) {
    perror("ioctl");
    exit(1);
  }
  ce(1);

  xfer.tx_buf = (__u64)buf;
  xfer.len = 1;
  buf[0] = 0x09;
  buf[1] = 0x0c;
  buf[2] = 0x41;
  buf[3] = 0x80;
  COMMAND;
  ce(0);
  if (ioctl(file, SPI_IOC_MESSAGE(1), &xfer) < 0) {
    perror("ioctl");
    exit(1);
  }
  ce(1);

  return file;
}

int main(void) { int file = configure_display(); }
