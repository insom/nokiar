#include <error.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

/*

  | LCD  | BCM    | Pi P1  |
  + ---- + ------ + ------ +
  | RST  | GPIO23 | Pin 16 |
  | DC   | GPIO24 | Pin 18 |
  | CE0  | GPIO8  | Pin 24 |
  | DIN  | GPIO10 | Pin 19 |
  | CLK  | GPIO11 | Pin 23 |

 */

void reset(void) {
  system("raspi-gpio set 23 op dl");
  usleep(50);
  system("raspi-gpio set 23 op dh");
  usleep(50);
}

void set_dc(unsigned char p) {
  if (p == 0)
    system("raspi-gpio set 24 op dl");
  else
    system("raspi-gpio set 24 op dh");
}

void lcd_send(int file, char *buf, int len, char dc) {
  struct spi_ioc_transfer xfer = {
      .tx_buf = (__u64)buf,
      .rx_buf = 0,
      .len = len,
      .delay_usecs = 0,
      .speed_hz = 100000,
  };
  set_dc(dc);
  if (ioctl(file, SPI_IOC_MESSAGE(1), &xfer) < 0) {
    perror("ioctl");
    exit(1);
  }
}

int configure_display(void) {
  int file;
  unsigned char buf[512];

  if ((file = open("/dev/spidev0.0", O_RDWR)) < 0) {
    perror("open");
    exit(1);
  }

  reset();

  lcd_send(file, "\x21\xb2\x13\x20", 4, 0);

  for (int i = 0; i < 6 * 84; i++) {
    /* System 6 Wallpaper */
    buf[i] = i % 2 == 0 ? 0x55 : 0xaa;
  }
  lcd_send(file, buf, 6 * 84, 1);

  lcd_send(file, "\x09\x0c\x41\x80", 4, 0);

  return file;
}

int main(void) { int file = configure_display(); }
