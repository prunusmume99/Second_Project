// i2c_lcd.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>
#include "i2c_lcd.h"

static int lcd_fd;
static int lcd_addr;

void lcd_send_cmd(char cmd) {
    char buffer[2];
    buffer[0] = 0x00;  // 명령 전송
    buffer[1] = cmd;
    write(lcd_fd, buffer, 2);
    usleep(1000);
}

void lcd_send_data(char data) {
    char buffer[2];
    buffer[0] = 0x40;  // 데이터 전송
    buffer[1] = data;
    write(lcd_fd, buffer, 2);
    usleep(1000);
}

void lcd_init(int i2c_address) {
    lcd_addr = i2c_address;
    char *i2c_file = "/dev/i2c-1";

    lcd_fd = open(i2c_file, O_RDWR);
    if (lcd_fd < 0) {
        perror("I2C open 실패");
        exit(1);
    }

    if (ioctl(lcd_fd, I2C_SLAVE, lcd_addr) < 0) {
        perror("I2C address 설정 실패");
        exit(1);
    }

    // 초기화 시퀀스
    lcd_send_cmd(0x38); // Function set
    lcd_send_cmd(0x39); // Extended command set
    lcd_send_cmd(0x14); // Internal OSC freq
    lcd_send_cmd(0x70); // Contrast set
    lcd_send_cmd(0x56); // Power/Icon/Contrast
    lcd_send_cmd(0x6C); // Follower control
    usleep(200000);
    lcd_send_cmd(0x38);
    lcd_send_cmd(0x0C); // Display ON
    lcd_send_cmd(0x01); // Clear display
    usleep(2000);
}

void lcd_clear() {
    lcd_send_cmd(0x01);  // Clear display
    usleep(2000);
}

void lcd_set_cursor(int row, int col) {
    char pos = 0x80 + (row == 0 ? 0x00 : 0x40) + col;
    lcd_send_cmd(pos);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}
