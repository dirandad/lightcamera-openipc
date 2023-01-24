#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

extern const unsigned char LIGHTCMD_UPDATE;
extern const unsigned char LIGHTCMD_LIGHTMODE;
extern const unsigned char LIGHTCMD_ALARMMODE;
extern const unsigned char LIGHTCMD_SETPIRSENSIBILITY;
extern const unsigned char LIGHTCMD_SETLUXSENSIBILITY;
extern const unsigned char LIGHTCMD_SETHIGHLIGHTLEVEL;
extern const unsigned char LIGHTCMD_ONTEMPORISATION;
extern const unsigned char LIGHTCMD_LOWLIGHTDURATION;
extern const unsigned char LIGHTCMD_SETLOWLIGHTLEVEL;

extern const unsigned char LIGHTCMD_EXTPARAM_496e63;
extern const unsigned char LIGHTCMD_EXTPARAM_726765;

// send light mode on command
extern const unsigned char LIGHTMODE_ON;
extern const unsigned char LIGHTMODE_DETECT;
extern const unsigned char LIGHTMODE_CONFIG;

// send Alarm mode on command
extern const unsigned char ALARMMODE_ON;
extern const unsigned char ALARMMODE_OFF;

// send light mode on command
extern const unsigned char ONTEMPORISATION_01MIN;
extern const unsigned char ONTEMPORISATION_03MIN;
extern const unsigned char ONTEMPORISATION_10MIN;
extern const unsigned char ONTEMPORISATION_15MIN;


// send light mode on command
extern const unsigned char LOWLIGHTDURATION_ALLNIGHT;
extern const unsigned char LOWLIGHTDURATION_02H;
extern const unsigned char LOWLIGHTDURATION_04H;
extern const unsigned char LOWLIGHTDURATION_06H;
extern const unsigned char LOWLIGHTDURATION_10H;


int set_interface_attributes (int fd, int speed, int parity);

int set_blocking (int fd, int should_block);

int initialize(char *portname, int baudrate, int parity);

int light_sendcommand(unsigned char command, unsigned char param, int extparam);

int light_update();

int light_lightmode(unsigned char lightmode);

int light_alarmmode(unsigned char alarmmode);

int light_pirsensibility(int pirsensibility);

int light_luxsensibility(int luxsensibility);

int light_highlightlevel(int highlightlevel);

int light_ontemporisation(unsigned char ontemporisation);

int light_lowlightlevel(int lowlightlevel);

int light_lowlightduration(unsigned char lowlightduration);


#endif