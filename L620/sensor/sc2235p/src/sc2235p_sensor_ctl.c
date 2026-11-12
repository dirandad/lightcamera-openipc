/*
 * Copyright (c) Hunan Goke,Chengdu Goke,Shandong Goke. 2021. All rights reserved.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "comm_video.h"
#include "sns_ctrl.h"

#ifdef GPIO_I2C
#include "gpioi2c_ex.h"
#else
#include "i2c.h"
#endif

const unsigned char sc2235p_i2c_addr = 0x60; /* I2C Address of SC2235P */
const unsigned int sc2235p_addr_byte = 2;
const unsigned int sc2235p_data_byte = 1;
static int g_fd[ISP_MAX_PIPE_NUM] = { [0 ...(ISP_MAX_PIPE_NUM - 1)] = -1 };

extern ISP_SNS_STATE_S *g_pastSc2235p[ISP_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSc2235pBusInfo[];

// sensor fps mode
#define SC2235P_SENSOR_1080P_30FPS_LINEAR_MODE (0)

int sc2235p_i2c_init(VI_PIPE ViPipe)
{
	char acDevFile[16] = { 0 };
	GK_U8 u8DevNum;

	if (g_fd[ViPipe] >= 0) {
		return GK_SUCCESS;
	}
#ifdef GPIO_I2C
	int ret;

	g_fd[ViPipe] = open("/dev/gpioi2c_ex", O_RDONLY, S_IRUSR);
	if (g_fd[ViPipe] < 0) {
		ISP_TRACE(MODULE_DBG_ERR, "Open gpioi2c_ex error!\n");
		return GK_FAILURE;
	}
#else
	int ret;

	u8DevNum = g_aunSc2235pBusInfo[ViPipe].s8I2cDev;
	snprintf(acDevFile, sizeof(acDevFile), "/dev/i2c-%u", u8DevNum);

	g_fd[ViPipe] = open(acDevFile, O_RDWR, S_IRUSR | S_IWUSR);

	if (g_fd[ViPipe] < 0) {
		ISP_TRACE(MODULE_DBG_ERR, "Open /dev/i2c_drv-%u error!\n",
			  u8DevNum);
		return GK_FAILURE;
	}

	ret = ioctl(g_fd[ViPipe], I2C_SLAVE_FORCE, (sc2235p_i2c_addr >> 1));
	if (ret < 0) {
		ISP_TRACE(MODULE_DBG_ERR, "I2C_SLAVE_FORCE error!\n");
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return ret;
	}
#endif

	return GK_SUCCESS;
}

int sc2235p_i2c_exit(VI_PIPE ViPipe)
{
	if (g_fd[ViPipe] >= 0) {
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return GK_SUCCESS;
	}
	return GK_FAILURE;
}

int sc2235p_read_register(VI_PIPE ViPipe, int addr)
{
	return GK_SUCCESS;
}

int sc2235p_write_register(VI_PIPE ViPipe, GK_U32 addr, GK_U32 data)
{
	if (g_fd[ViPipe] < 0) {
		return GK_SUCCESS;
	}

#ifdef GPIO_I2C
	i2c_data.dev_addr = sc2235p_i2c_addr;
	i2c_data.reg_addr = addr;
	i2c_data.addr_byte_num = sc2235p_addr_byte;
	i2c_data.data = data;
	i2c_data.data_byte_num = sc2235p_data_byte;

	ret = ioctl(g_fd[ViPipe], GPIO_I2C_WRITE, &i2c_data);

	if (ret) {
		ISP_TRACE(MODULE_DBG_ERR, "GPIO-I2C write faild!\n");
		return ret;
	}
#else
	int idx = 0;
	int ret;
	char buf[8];

	if (sc2235p_addr_byte == 2) {
		buf[idx] = (addr >> 8) & 0xff;
		idx++;
		buf[idx] = addr & 0xff;
		idx++;
	} else {
		buf[idx] = addr & 0xff;
		idx++;
	}

	if (sc2235p_data_byte == 2) {
		buf[idx] = (data >> 8) & 0xff;
		idx++;
		buf[idx] = data & 0xff;
		idx++;
	} else {
		buf[idx] = data & 0xff;
		idx++;
	}

	ret = write(g_fd[ViPipe], buf, sc2235p_addr_byte + sc2235p_data_byte);
	if (ret < 0) {
		ISP_TRACE(MODULE_DBG_ERR, "I2C_WRITE error!\n");
		return GK_FAILURE;
	}

#endif
	return GK_SUCCESS;
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc2235p_standby(VI_PIPE ViPipe)
{
	return;
}

void sc2235p_restart(VI_PIPE ViPipe)
{
	return;
}

void sc2235p_linear_1080p30_init(VI_PIPE ViPipe);

void sc2235p_init(VI_PIPE ViPipe)
{
	GK_U32 i;
	GK_U8 u8ImgMode;
	u8ImgMode = g_pastSc2235p[ViPipe]->u8ImgMode;
	sc2235p_i2c_init(ViPipe);
	switch (u8ImgMode) {
	case 0:
		sc2235p_linear_1080p30_init(ViPipe);
		break;
	default:
		printf("SC2235P_SENSOR_CTL_Not support this mode\n");
		g_pastSc2235p[ViPipe]->bInit = GK_FALSE;
		break;
	}

	for (i = 0; i < g_pastSc2235p[ViPipe]->astRegsInfo[0].u32RegNum; i++) {
		sc2235p_write_register(ViPipe,
				      g_pastSc2235p[ViPipe]
					      ->astRegsInfo[0]
					      .astI2cData[i]
					      .u32RegAddr,
				      g_pastSc2235p[ViPipe]
					      ->astRegsInfo[0]
					      .astI2cData[i]
					      .u32Data);
	}

	g_pastSc2235p[ViPipe]->bInit = GK_TRUE;

	return;
}

void sc2235p_exit(VI_PIPE ViPipe)
{
	sc2235p_i2c_exit(ViPipe);

	return;
}

void sc2235p_linear_1080p30_init(VI_PIPE ViPipe)
{
	// sc2235p_write_register(ViPipe, 0x0103, 0x01);
	delay_ms(20);
	sc2235p_write_register(ViPipe, 0x0100, 0x00);


	sc2235p_write_register(ViPipe, 0x0100, 0x00);
	sc2235p_write_register(ViPipe, 0x3001, 0xfe);
	sc2235p_write_register(ViPipe, 0x3018, 0x33);
	sc2235p_write_register(ViPipe, 0x3031, 0x0a);
	sc2235p_write_register(ViPipe, 0x3037, 0x20);
	sc2235p_write_register(ViPipe, 0x3038, 0xff);
	sc2235p_write_register(ViPipe, 0x3039, 0x22);
	sc2235p_write_register(ViPipe, 0x303b, 0x06);
	sc2235p_write_register(ViPipe, 0x303c, 0x0e);
	sc2235p_write_register(ViPipe, 0x3200, 0x00);
	sc2235p_write_register(ViPipe, 0x3201, 0x00);
	sc2235p_write_register(ViPipe, 0x3202, 0x00);
	sc2235p_write_register(ViPipe, 0x3203, 0x00);
	sc2235p_write_register(ViPipe, 0x3204, 0x07);
	sc2235p_write_register(ViPipe, 0x3205, 0x8f);
	sc2235p_write_register(ViPipe, 0x3206, 0x04);
	sc2235p_write_register(ViPipe, 0x3207, 0x47);
	sc2235p_write_register(ViPipe, 0x3208, 0x07);
	sc2235p_write_register(ViPipe, 0x3209, 0x88);
	sc2235p_write_register(ViPipe, 0x320a, 0x04);
	sc2235p_write_register(ViPipe, 0x320b, 0x40);
	sc2235p_write_register(ViPipe, 0x320c, 0x0a);
	sc2235p_write_register(ViPipe, 0x320d, 0x50);
	sc2235p_write_register(ViPipe, 0x320e, 0x04);
	sc2235p_write_register(ViPipe, 0x320f, 0x65);
	sc2235p_write_register(ViPipe, 0x3211, 0x04);
	sc2235p_write_register(ViPipe, 0x3213, 0x04);
	sc2235p_write_register(ViPipe, 0x3222, 0x29);
	sc2235p_write_register(ViPipe, 0x3235, 0x08);
	sc2235p_write_register(ViPipe, 0x3236, 0xc8);
	sc2235p_write_register(ViPipe, 0x3301, 0x06);
	sc2235p_write_register(ViPipe, 0x3302, 0x1f);
	sc2235p_write_register(ViPipe, 0x3303, 0x20);
	sc2235p_write_register(ViPipe, 0x3306, 0x48);
	sc2235p_write_register(ViPipe, 0x3308, 0x10);
	sc2235p_write_register(ViPipe, 0x3309, 0x60);
	sc2235p_write_register(ViPipe, 0x330b, 0xd3);
	sc2235p_write_register(ViPipe, 0x330e, 0x30);
	sc2235p_write_register(ViPipe, 0x3314, 0x08);
	sc2235p_write_register(ViPipe, 0x331b, 0x83);
	sc2235p_write_register(ViPipe, 0x331e, 0x19);
	sc2235p_write_register(ViPipe, 0x331f, 0x59);
	sc2235p_write_register(ViPipe, 0x3320, 0x01);
	sc2235p_write_register(ViPipe, 0x3326, 0x00);
	sc2235p_write_register(ViPipe, 0x3333, 0x30);
	sc2235p_write_register(ViPipe, 0x335e, 0x01);
	sc2235p_write_register(ViPipe, 0x335f, 0x03);
	sc2235p_write_register(ViPipe, 0x3366, 0x7c);
	sc2235p_write_register(ViPipe, 0x3367, 0x08);
	sc2235p_write_register(ViPipe, 0x3368, 0x04);
	sc2235p_write_register(ViPipe, 0x3369, 0x00);
	sc2235p_write_register(ViPipe, 0x336a, 0x00);
	sc2235p_write_register(ViPipe, 0x336b, 0x00);
	sc2235p_write_register(ViPipe, 0x337c, 0x04);
	sc2235p_write_register(ViPipe, 0x337d, 0x06);
	sc2235p_write_register(ViPipe, 0x337f, 0x03);
	sc2235p_write_register(ViPipe, 0x33a0, 0x05);
	sc2235p_write_register(ViPipe, 0x33aa, 0x10);
	sc2235p_write_register(ViPipe, 0x360f, 0x01);
	sc2235p_write_register(ViPipe, 0x3620, 0x28);
	sc2235p_write_register(ViPipe, 0x3621, 0x28);
	sc2235p_write_register(ViPipe, 0x3622, 0x06);
	sc2235p_write_register(ViPipe, 0x3624, 0x08);
	sc2235p_write_register(ViPipe, 0x3625, 0x02);
	sc2235p_write_register(ViPipe, 0x3630, 0x1c);
	sc2235p_write_register(ViPipe, 0x3631, 0x84);
	sc2235p_write_register(ViPipe, 0x3632, 0x08);
	sc2235p_write_register(ViPipe, 0x3633, 0x4f);
	sc2235p_write_register(ViPipe, 0x3635, 0xa0);
	sc2235p_write_register(ViPipe, 0x3636, 0x25);
	sc2235p_write_register(ViPipe, 0x3637, 0x59);
	sc2235p_write_register(ViPipe, 0x3638, 0x1f);
	sc2235p_write_register(ViPipe, 0x3639, 0x09);
	sc2235p_write_register(ViPipe, 0x363a, 0x9f);
	sc2235p_write_register(ViPipe, 0x363b, 0x0b);
	sc2235p_write_register(ViPipe, 0x363c, 0x05);
	sc2235p_write_register(ViPipe, 0x366e, 0x08);
	sc2235p_write_register(ViPipe, 0x366f, 0x2f);
	sc2235p_write_register(ViPipe, 0x3670, 0x0c);
	sc2235p_write_register(ViPipe, 0x3671, 0xc6);
	sc2235p_write_register(ViPipe, 0x3672, 0x06);
	sc2235p_write_register(ViPipe, 0x3673, 0x16);
	sc2235p_write_register(ViPipe, 0x3677, 0x84);
	sc2235p_write_register(ViPipe, 0x3678, 0x88);
	sc2235p_write_register(ViPipe, 0x3679, 0x88);
	sc2235p_write_register(ViPipe, 0x367a, 0x28);
	sc2235p_write_register(ViPipe, 0x367b, 0x3f);
	sc2235p_write_register(ViPipe, 0x367e, 0x08);
	sc2235p_write_register(ViPipe, 0x367f, 0x28);
	sc2235p_write_register(ViPipe, 0x3690, 0x34);
	sc2235p_write_register(ViPipe, 0x3691, 0x11);
	sc2235p_write_register(ViPipe, 0x3692, 0x42);
	sc2235p_write_register(ViPipe, 0x369c, 0x08);
	sc2235p_write_register(ViPipe, 0x369d, 0x28);
	sc2235p_write_register(ViPipe, 0x3802, 0x01);
	sc2235p_write_register(ViPipe, 0x3901, 0x02);
	sc2235p_write_register(ViPipe, 0x3902, 0xC5); // 0x45
    sc2235p_write_register(ViPipe, 0x3905, 0x98);
	sc2235p_write_register(ViPipe, 0x3907, 0x00);
	sc2235p_write_register(ViPipe, 0x3908, 0x11);
	sc2235p_write_register(ViPipe, 0x391b, 0x80);
	sc2235p_write_register(ViPipe, 0x3e00, 0x00);
	sc2235p_write_register(ViPipe, 0x3e01, 0x8c);
	sc2235p_write_register(ViPipe, 0x3e02, 0x60);
	sc2235p_write_register(ViPipe, 0x3e03, 0x0b);
	sc2235p_write_register(ViPipe, 0x3e06, 0x00);
	sc2235p_write_register(ViPipe, 0x3e07, 0x80);
	sc2235p_write_register(ViPipe, 0x3e08, 0x03);
	sc2235p_write_register(ViPipe, 0x3e09, 0x10);
	sc2235p_write_register(ViPipe, 0x3e1e, 0x34);
	sc2235p_write_register(ViPipe, 0x3f00, 0x07);
	sc2235p_write_register(ViPipe, 0x3f04, 0x04);
	sc2235p_write_register(ViPipe, 0x3f05, 0x28);
	sc2235p_write_register(ViPipe, 0x4603, 0x00);
	sc2235p_write_register(ViPipe, 0x4827, 0x48);
	sc2235p_write_register(ViPipe, 0x4837, 0x35);
	sc2235p_write_register(ViPipe, 0x5000, 0x06);
	sc2235p_write_register(ViPipe, 0x5780, 0xff);
	sc2235p_write_register(ViPipe, 0x5781, 0x04);
	sc2235p_write_register(ViPipe, 0x5785, 0x18);
	delay_ms(20);
	sc2235p_write_register(ViPipe, 0x0100, 0x01);


    /*{0xF5, 0xA5}, // SleepMask1
    {0x5A, 0x5F}, // SleepMask2
    {0x00, 0x64}, // 100ms*/


	printf("=== Smart SC2235P_1080P_30FPS_10BIT_LINE_Init_OK!===\n");

	return;
}










