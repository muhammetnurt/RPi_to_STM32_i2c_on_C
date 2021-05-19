#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <string.h>




u_int8_t get_time[7];
#define DS3231_ADDRESS 0x50
#define DS3231_regAddr_SETTIME 0x00
#define DS3231_regAddr_GETTIME 0x00
#define delay_ms(a) usleep(a*1000)
//#define  DEBUG

int readBytes(u_int8_t devAddr, u_int8_t regAddr, u_int8_t length, u_int8_t *data) {
    int8_t count = 0;
#ifdef DEBUG
    printf("read %#x %#x %u\n",devAddr,regAddr,length);
#endif
    int fd = open("/dev/i2c-1", O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return(-1);
    }
    if (ioctl(fd, I2C_SLAVE, devAddr) < 0) {
        fprintf(stderr, "Failed to select device: %s\n", strerror(errno));
        close(fd);
        return(-1);
    }
    if (write(fd, &regAddr, 1) != 1) {
        fprintf(stderr, "Failed to write reg: %s\n", strerror(errno));
        close(fd);
        return(-1);
    }
    count = read(fd, data, length);
    if (count < 0) {
        fprintf(stderr, "Failed to read device(%d): %s\n", count, strerror(errno));
        close(fd);
        return(-1);
    } else if (count != length) {
        fprintf(stderr, "Short read  from device, expected %d, got %d\n", length, count);
        close(fd);
        return(-1);
    }
    close(fd);

    return count;
}

u_int8_t writeBytes(u_int8_t devAddr, u_int8_t regAddr, u_int8_t length, u_int8_t* data) {
    int8_t count = 0;
    uint8_t buf[128];
    int fd;

#ifdef DEBUG
    printf("write %#x %#x\n",devAddr,regAddr);
#endif
    if (length > 127) {
        fprintf(stderr, "Byte write count (%d) > 127\n", length);
        return -1;
    }

    fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, devAddr) < 0) {
        fprintf(stderr, "Failed to select device: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    buf[0] = regAddr;
    memcpy(buf+1,data,length);
    count = write(fd, buf, length+1);
    if (count < 0) {
        fprintf(stderr, "Failed to write device(%d): %s\n", count, strerror(errno));
        close(fd);
        return -1;
    } else if (count != length+1) {
        fprintf(stderr, "Short write to device, expected %d, got %d\n", length+1, count);
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

int main() {
	u_int8_t seconds=40;
	u_int8_t minutes=44;
	u_int8_t hour=4;
	u_int8_t dayofweek=5;
	u_int8_t dayofmonth=13;
	u_int8_t month=5;
	int year=2021;
	u_int8_t set_time[]={seconds, minutes,hour, dayofweek, dayofmonth, month, year};
	writeBytes(DS3231_ADDRESS, DS3231_regAddr_SETTIME, 7, set_time);
	do{
	    
		readBytes(DS3231_ADDRESS, DS3231_regAddr_GETTIME, 7, get_time);
		seconds = get_time[0];
		minutes = get_time[1];
		hour = get_time[2];
		dayofweek = get_time[3];
		dayofmonth = get_time[4];
		month = get_time[5];
		year = get_time[6];
		delay_ms(1000);
		printf("%x:%x:%x - %x-%x-%x--%x\n" ,hour, minutes, seconds, dayofweek, dayofmonth, month, year);

	
	}while(1);

	return 0;
}



