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
  system("raspi-gpio set 16 op dh");
  usleep(500);
  system("raspi-gpio set 16 op dl");
  usleep(500);
}

void dc(unsigned char p) {
  if (p == 0)
    system("raspi-gpio set 18 op dl");
  else
    system("raspi-gpio set 18 op dh");
}

void ce(unsigned char p) {
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
  xfer.len = 6;
  buf[0] = 0x21;
  buf[1] = 0x14;
  buf[2] = 0xa8;
  buf[3] = 0x20;
  buf[4] = 0x40;
  buf[5] = 0x80;
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
  buf[0] = 0x0d;
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
