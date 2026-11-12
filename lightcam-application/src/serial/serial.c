/*------------------------------------------------------------
Light Camera binary for HI3518EV300 50H20L

Serial communication port managment with Light Card

Contributors:
   Adrien Dirand - initial implementation and documentation.
-------------------------------------------------------------*/
#include "serial.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions          */ 
#include <errno.h>   /* ERROR Number Definitions           */
#include <poll.h>
#include <time.h>

/*
The values for parity are 0 (meaning no parity)
PARENB|PARODD (enable parity and use odd)
PARENB (enable parity and use even)
PARENB|PARODD|CMSPAR (mark parity)
PARENB|CMSPAR (space parity)

speed : B110, B300, B600, B1200, B2400, B4800, B9600, B14400, B19200, B38400, B57600, B115200, B128000 and B256000

"Blocking" sets whether a read() on the port waits for the specified number of characters to arrive.
Setting no blocking means that a read() returns however many characters are available without waiting for more, up to the buffer limit.

*/


/* Usage

write (port_fd, "hello!\n", 7);           // send 7 character greeting

usleep ((7 + 25) * 100);             // sleep enough to transmit the 7 plus
                                     // receive 25:  approx 100 uS per char transmit
char buf [100];
int n = read (port_fd, buf, sizeof buf);  // read up to 100 characters if ready to read

*/

const unsigned char LIGHTCMD_UPDATE            = 0x46;
const unsigned char LIGHTCMD_LIGHTMODE         = 0x4d;
const unsigned char LIGHTCMD_ALARMMODE         = 0x41;
const unsigned char LIGHTCMD_SETPIRSENSIBILITY = 0x44;
// 0x58 ('X'). It was 0x44 before 2026-11, the PIR command: setting the lux
// sensibility changed the PIR distance instead (checked on the MCU status frame).
const unsigned char LIGHTCMD_SETLUXSENSIBILITY = 0x58;
const unsigned char LIGHTCMD_SETHIGHLIGHTLEVEL = 0x48;
const unsigned char LIGHTCMD_ONTEMPORISATION   = 0x54;
const unsigned char LIGHTCMD_LOWLIGHTDURATION  = 0x53;
const unsigned char LIGHTCMD_SETLOWLIGHTLEVEL  = 0x4c;

const unsigned char LIGHTCMD_EXTPARAM_496e63   = 0x01;
const unsigned char LIGHTCMD_EXTPARAM_726765   = 0x02;

// send light mode on command
const unsigned char LIGHTMODE_ON     = 0x64;
const unsigned char LIGHTMODE_DETECT = 0x62;
const unsigned char LIGHTMODE_CONFIG = 0x65;

// send Alarm mode on command
const unsigned char ALARMMODE_ON     = 0x62;
const unsigned char ALARMMODE_OFF    = 0x7A;

// send light mode on command
const unsigned char ONTEMPORISATION_01MIN = 0x62;
const unsigned char ONTEMPORISATION_03MIN = 0x64;
const unsigned char ONTEMPORISATION_10MIN = 0x6B;
const unsigned char ONTEMPORISATION_15MIN = 0x70;


// send light mode on command
const unsigned char LOWLIGHTDURATION_ALLNIGHT = 0x61;
const unsigned char LOWLIGHTDURATION_02H = 0x66;
const unsigned char LOWLIGHTDURATION_04H = 0x6B;
const unsigned char LOWLIGHTDURATION_06H = 0x70;
const unsigned char LOWLIGHTDURATION_10H = 0x7A;

// Port handler
int port_fd = -1;

int set_interface_attributes (int fd, int speed, int parity)
{
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

int set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return -1;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        // non blocking: read() returns at once; light_readstatus() waits with poll()
        tty.c_cc[VTIME] = should_block ? 5 : 0;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
		{
                printf ("error %d setting term attributes", errno);
				return -1;
		}
		
		return 0;
}

int initialize(char *portname, int baudrate, int parity)
{
	port_fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	
	if (port_fd < 0)
	{
        printf ("error %d opening %s: %s", errno, portname, strerror (errno));
        return -1;
	}
	
	int speed;
	
	 switch(baudrate)
     {
         case 110:
           speed = B110;
		   break;
         case 300:
           speed = B300;
		   break;
         case 600:
           speed = B600;
		   break;
         case 1200:
           speed = B1200;
		   break;
         case 2400:
           speed = B2400;
		   break;
         case 4800:
           speed = B4800;
		   break;
         case 9600:
           speed = B9600;
		   break;
         //case 14400:
         //  speed = B14400;
		 //  break;
         case 19200:
           speed = B19200;
		   break;
         case 38400:
           speed = B38400;
		   break;
         case 57600:
           speed = B57600;
		   break;
         case 115200:
           speed = B115200;
		   break;
        // case 128000:
        //   speed = B128000;
		//   break;
        // case 256000:
        //   speed = B256000;
		//   break;
         default:
           printf ("error: baudrate %d is not allowed", baudrate);
		   return -1;
		   break;
    }
	
	if (set_interface_attributes (port_fd, speed, parity) < 0)
	{
		return -1;
	}
		
	if (set_blocking (port_fd, 0) < 0) // set no blocking
	{
		return -1;
	}

	  // set speed to 115,200 bps, 8n1 (no parity)
	
	return 0;
}


// ==================================================Light Command=============================================================

int light_sendcommand(unsigned char command, unsigned char param, int extparam)
{
	if (port_fd < 0)
	{
        printf ("serial error: port must be initialized\n");
        return -1;
	}
	
	unsigned char extparam1;
	unsigned char extparam2;
	unsigned char extparam3;
	
	 switch(extparam)
     {
		case 1:
			extparam1 = 0x49;
			extparam2 = 0x6e;
			extparam3 = 0x63;
		   break;
		case 2:
			extparam1 = 0x72;
			extparam2 = 0x67;
			extparam3 = 0x65;
		   break;
         default:
           printf ("serial error: extparam %d is not allowed\n", extparam);
		   return -1;
		   break;
	}

	
	unsigned char bytearray[]={0x42, command, param, 0x55, 0x00, extparam1, extparam2, extparam3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (write (port_fd, bytearray, 16) != 16)
	{
		printf ("serial error: write failed: %s\n", strerror (errno));
		return -1;
	}
	// wait until the 16 bytes are sent (about 17 ms at 9600 bauds)
	tcdrain (port_fd);

	return 0;
}

static long now_ms(void)
{
	struct timespec ts;
	clock_gettime (CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

// Send the update command and read the status frame of the MCU.
// The MCU answers only the update command, in less than 0.5 s, with 12 bytes:
// 0x42, 10 status bytes, 0x55. The status bytes use the command codes:
//   status[0] PIR distance (0x60 + pirsensibility)
//   status[1] on temporisation code
//   status[2] lux sensibility (0x60 + luxsensibility)
//   status[3], status[4] unknown
//   status[5] low light level (0x60 + lowlightlevel)
//   status[6] high light level (0x67 + highlightlevel)
//   status[7] low light duration code
//   status[8], status[9] unknown
// Returns 0 when a frame was read, -1 otherwise.
int light_readstatus(unsigned char status[LIGHT_STATUS_LEN])
{
	if (port_fd < 0)
		return -1;

	unsigned char frame[LIGHT_STATUS_LEN + 2];
	int len = 0;

	tcflush (port_fd, TCIFLUSH);     // drop old bytes (old answers, noise)
	if (light_update () < 0)
		return -1;

	long end = now_ms () + 1000;
	while (now_ms () < end)
	{
		struct pollfd pfd = { port_fd, POLLIN, 0 };
		long left = end - now_ms ();
		if (left <= 0)
			break;
		if (poll (&pfd, 1, (int)left) <= 0)
			continue;

		unsigned char c;
		while (read (port_fd, &c, 1) == 1)
		{
			if (len == 0 && c != 0x42)
				continue;          // wait for the start byte
			frame[len++] = c;
			if (len == LIGHT_STATUS_LEN + 2)
			{
				if (frame[LIGHT_STATUS_LEN + 1] == 0x55)
				{
					memcpy (status, frame + 1, LIGHT_STATUS_LEN);
					return 0;
				}
				// not a frame: look for another start byte in what we got
				int i;
				for (i = 1; i < len && frame[i] != 0x42; i++)
					;
				memmove (frame, frame + i, len - i);
				len -= i;
			}
		}
	}
	return -1;
}

// Compare a status frame with the expected configuration.
// Returns a bit mask of the settings that differ (0: all good).
int light_checkstatus(struct struct_light_config config, const unsigned char status[LIGHT_STATUS_LEN])
{
	int diff = 0;
	if (status[0] != (unsigned char)(0x60 + config.pirsensibility))  diff |= LIGHT_DIFF_PIR;
	if (status[1] != config.ontemporisation)                          diff |= LIGHT_DIFF_ONTEMPO;
	if (status[2] != (unsigned char)(0x60 + config.luxsensibility))  diff |= LIGHT_DIFF_LUX;
	if (status[5] != (unsigned char)(0x60 + config.lowlightlevel))   diff |= LIGHT_DIFF_LOWLEVEL;
	if (status[6] != (unsigned char)(0x67 + config.highlightlevel))  diff |= LIGHT_DIFF_HIGHLEVEL;
	if (status[7] != config.lowlightduration)                         diff |= LIGHT_DIFF_LOWDURATION;
	return diff;
}


// send update command

int light_update()
{

	return light_sendcommand(LIGHTCMD_UPDATE, 0x62, LIGHTCMD_EXTPARAM_496e63);
	
}


// send light mode on command

int light_lightmode(unsigned char lightmode)
{

	return light_sendcommand(LIGHTCMD_LIGHTMODE, lightmode, LIGHTCMD_EXTPARAM_496e63);
}

// send Alarm mode on command

int light_alarmmode(unsigned char alarmmode)
{

	return light_sendcommand(LIGHTCMD_ALARMMODE, alarmmode, LIGHTCMD_EXTPARAM_496e63);

}

// set PIR sensibility
// pirsensibility in [1-25]
int light_pirsensibility(int pirsensibility)
{
	if(pirsensibility >= 1 && pirsensibility <= 25)
	{
		pirsensibility = 0x60 + pirsensibility;
		return light_sendcommand(LIGHTCMD_SETPIRSENSIBILITY, (unsigned char)pirsensibility, LIGHTCMD_EXTPARAM_726765);
	}
	else
	{
        printf ("serial error: pirsensibility must be include in [1-25]\n");
        return -1;
	}

}

// set Lux sensibility
// luxsensibility in [1-23]
int light_luxsensibility(int luxsensibility)
{
	if(luxsensibility >= 1 && luxsensibility <= 23)
	{
		luxsensibility = 0x60 + luxsensibility;
		return light_sendcommand(LIGHTCMD_SETLUXSENSIBILITY, (unsigned char)luxsensibility, LIGHTCMD_EXTPARAM_726765);
	}
	else
	{
        printf ("serial error: luxsensibility must be include in [1-23]\n");
        return -1;
	}

}

// set highlight level
// highlightlevel in [1-19]
int light_highlightlevel(int highlightlevel)
{
	if(highlightlevel >= 1 && highlightlevel <= 19)
	{
		highlightlevel = 0x67 + highlightlevel;
		return light_sendcommand(LIGHTCMD_SETHIGHLIGHTLEVEL, (unsigned char)highlightlevel, LIGHTCMD_EXTPARAM_726765);
	}
	else
	{
        printf ("serial error: highlightlevel must be include in [1-19]\n");
        return -1;
	}

}

// send light on temporisation

int light_ontemporisation(unsigned char ontemporisation)
{

	return light_sendcommand(LIGHTCMD_ONTEMPORISATION, ontemporisation, LIGHTCMD_EXTPARAM_726765);
	
}

// set lowlight level
// lowlightlevel in [1-12]
int light_lowlightlevel(int lowlightlevel)
{
	if(lowlightlevel >= 1 && lowlightlevel <= 12)
	{
		lowlightlevel = 0x60 + lowlightlevel;
		return light_sendcommand(LIGHTCMD_SETLOWLIGHTLEVEL, (unsigned char)lowlightlevel, LIGHTCMD_EXTPARAM_726765);
	}
	else
	{
        printf ("serial error: lowlightlevel must be include in [1-12]\n");
        return -1;
	}

}

// send low light duration

int light_lowlightduration(unsigned char lowlightduration)
{

	return light_sendcommand(LIGHTCMD_LOWLIGHTDURATION, lowlightduration, LIGHTCMD_EXTPARAM_726765);
	
}

// set config

int light_setconfig (struct struct_light_config config)
{
	int retval = 0;
	retval = retval + light_update();
	retval = retval + light_lightmode(config.lightmode); 				// send light mode on command
	retval = retval + light_alarmmode(config.alarmmode); 				// send Alarm mode on command
	retval = retval + light_pirsensibility(config.pirsensibility);	// set PIR sensibility
	retval = retval + light_luxsensibility(config.luxsensibility);	// set Lux sensibility
	retval = retval + light_highlightlevel(config.highlightlevel);	// set highlight level
	retval = retval + light_ontemporisation(config.ontemporisation);	// send light mode on command
	retval = retval + light_lowlightlevel(config.lowlightlevel);		// set lowlight level
	retval = retval + light_lowlightduration(config.lowlightduration);// send light mode on command
	return retval;
}