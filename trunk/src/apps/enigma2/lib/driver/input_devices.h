#ifndef __input_devices_h
#define __input_devices_h

#define	MAX_BUTTONS		4	/* This is 4 beacuse the average length of button is 40 byte and */
							/* the buffer to read from lircd is 128 byte (128/40 = 3,2 -> 4 */

typedef struct
{
	DFBInputEvent	evt;
	char rc_identifier[50];
}Input_Devices_t;


typedef struct
{
	DFBInputDeviceKeySymbol	symbol;
	char rc_identifier[50];
}Keysymbol_Devices_t;


/* If you use the threads */
int init_input_device(void);
int read_lircd(void);

#endif // __input_devices_h
