
#ifdef QBOXHD
#include <lib/gdi/glcddc.h>
#endif

#include <lib/driver/rcdfbinput.h>

#include <lib/base/eerror.h>

#include <sys/ioctl.h>
#include <linux/input.h>
#include <sys/stat.h>

#include <lib/base/ebase.h>
#include <lib/base/init.h>
#include <lib/base/init_num.h>
#include <lib/driver/input_fake.h>

#include "input_devices.h"

#include <dfb++.h>

DFBEnumerationResult
DirectFBShowInputDevices( DFBInputDeviceID    device_id,
                   DFBInputDeviceDescription  desc,
                   void                      *data )
{
      	printf( "[DFBInputDevice] Found Device name: %s vendor: %s\n", desc.name, desc.vendor );
      	return DFENUM_OK;
}

int eDirectFBInputDev::convertKeySymbol(DFBInputEvent *ev)
{
	switch(ev->device_id)
	{
		case DIDID_REMOTE:      /* Remote control Mapping */
			//printf("Event from DIDID_REMOTE\n");

			switch( ev->key_symbol )
			{
				case DIKS_MUTE: 	return KEY_MUTE;
				case DIKS_POWER: 	return KEY_POWER;
				case DIKS_0: 		return KEY_0;
				case DIKS_1: 		return KEY_1;
				case DIKS_2: 		return KEY_2;
				case DIKS_3: 		return KEY_3;
				case DIKS_4: 		return KEY_4;
				case DIKS_5: 		return KEY_5;
				case DIKS_6: 		return KEY_6;
				case DIKS_7: 		return KEY_7;
				case DIKS_8: 		return KEY_8;
				case DIKS_9: 		return KEY_9;
				case DIKS_PREVIOUS: 	return KEY_PREVIOUS;
				case DIKS_NEXT: 	return KEY_NEXT;
				case DIKS_INFO: 	return KEY_INFO;
				case DIKS_PAGE_UP: 	return KEY_CHANNELUP;
				case DIKS_PAGE_DOWN: 	return KEY_CHANNELDOWN;
				case DIKS_CURSOR_UP: 	return KEY_UP;
				case DIKS_CURSOR_DOWN: 	return KEY_DOWN;
				case DIKS_CURSOR_LEFT: 	return KEY_LEFT;
				case DIKS_CURSOR_RIGHT:	return KEY_RIGHT;
				case DIKS_MENU: 	return KEY_MENU;
				case DIKS_RETURN:	return KEY_OK;
				case DIKS_AUDIO: 	return KEY_AUDIO;
				case DIKS_VOLUME_UP: 	return KEY_VOLUMEUP;
				case DIKS_VOLUME_DOWN: 	return KEY_VOLUMEDOWN;
				case DIKS_VIDEO: 	return KEY_VIDEO;

				/* New code for QBoxHD_mini_rc */
				case DIKS_PLAY:		return KEY_PLAY;
				case DIKS_REWIND:	return KEY_REWIND;
				case DIKS_FORWARD:	return KEY_FORWARD;
				case DIKS_PAUSE:	return KEY_PAUSE;
				case DIKS_F11:		return KEY_PREVIOUSSONG;
				case DIKS_F12:		return KEY_NEXTSONG;

				case DIKS_STOP: 	return KEY_STOP;
				case DIKS_EXIT: 	return KEY_EXIT;
				case DIKS_RECORD: 	return KEY_RECORD;
				case DIKS_RED: 		return KEY_RED;
				case DIKS_GREEN: 	return KEY_GREEN;
				case DIKS_YELLOW: 	return KEY_YELLOW;
				case DIKS_BLUE: 	return KEY_BLUE;
				case DIKS_TV: 		return KEY_TV;
				case DIKS_RADIO: 	return KEY_RADIO;
				case DIKS_TEXT: 	return KEY_TEXT;
				case DIKS_HELP: 	return KEY_HELP;
				case DIKS_CUSTOM0: 	return KEY_OPTION;
				case DIKS_PLAYER: 	return KEY_MEDIA;

				default:
					return -1;
			}

		break;

		case DIDID_KEYBOARD:     /* Keyboard Mapping */
			//printf("Event from DIDID_KEYBOARD\n");

			switch( ev->key_symbol )
			{
				case DIKS_BACKSPACE 		  : return KEY_BACKSPACE;
				case DIKS_TAB                     : return KEY_TAB;
				case DIKS_RETURN                  : return KEY_OK;
				case DIKS_CANCEL                  : return KEY_CANCEL;
				case DIKS_ESCAPE                  : return KEY_ESC;
				case DIKS_SPACE                   : return KEY_SPACE;
				case DIKS_EXCLAMATION_MARK        : return -1;
				case DIKS_QUOTATION               : return -1;
				case DIKS_NUMBER_SIGN             : return -1;
				case DIKS_DOLLAR_SIGN             : return -1;
				case DIKS_PERCENT_SIGN            : return -1;
				case DIKS_AMPERSAND               : return -1;
				case DIKS_APOSTROPHE              : return -1;
				case DIKS_PARENTHESIS_LEFT        : return -1;
				case DIKS_PARENTHESIS_RIGHT       : return -1;
				case DIKS_ASTERISK                : return -1;
				case DIKS_PLUS_SIGN               : return -1;
				case DIKS_COMMA                   : return -1;
				case DIKS_MINUS_SIGN              : return -1;
				case DIKS_PERIOD                  : return -1;
				case DIKS_SLASH                   : return -1;
				case DIKS_0                       : return KEY_0;
				case DIKS_1                       : return KEY_1;
				case DIKS_2                       : return KEY_2;
				case DIKS_3                       : return KEY_3;
				case DIKS_4                       : return KEY_4;
				case DIKS_5                       : return KEY_5;
				case DIKS_6                       : return KEY_6;
				case DIKS_7                       : return KEY_7;
				case DIKS_8                       : return KEY_8;
				case DIKS_9                       : return KEY_9;
				case DIKS_COLON                   : return -1;
				case DIKS_SEMICOLON               : return -1;
				case DIKS_LESS_THAN_SIGN          : return -1;
				case DIKS_EQUALS_SIGN             : return -1;
				case DIKS_GREATER_THAN_SIGN       : return -1;
				case DIKS_QUESTION_MARK           : return -1;
				case DIKS_AT                      : return -1;
				case DIKS_CAPITAL_A               : return KEY_A;
				case DIKS_CAPITAL_B               : return KEY_B;
				case DIKS_CAPITAL_C               : return KEY_C;
				case DIKS_CAPITAL_D               : return KEY_D;
				case DIKS_CAPITAL_E               : return KEY_E;
				case DIKS_CAPITAL_F               : return KEY_F;
				case DIKS_CAPITAL_G               : return KEY_G;
				case DIKS_CAPITAL_H               : return KEY_H;
				case DIKS_CAPITAL_I               : return KEY_I;
				case DIKS_CAPITAL_J               : return KEY_J;
				case DIKS_CAPITAL_K               : return KEY_K;
				case DIKS_CAPITAL_L               : return KEY_L;
				case DIKS_CAPITAL_M               : return KEY_M;
				case DIKS_CAPITAL_N               : return KEY_N;
				case DIKS_CAPITAL_O               : return KEY_O;
				case DIKS_CAPITAL_P               : return KEY_P;
				case DIKS_CAPITAL_Q               : return KEY_Q;
				case DIKS_CAPITAL_R               : return KEY_R;
				case DIKS_CAPITAL_S               : return KEY_S;
				case DIKS_CAPITAL_T               : return KEY_T;
				case DIKS_CAPITAL_U               : return KEY_U;
				case DIKS_CAPITAL_V               : return KEY_V;
				case DIKS_CAPITAL_W               : return KEY_W;
				case DIKS_CAPITAL_X               : return KEY_X;
				case DIKS_CAPITAL_Y               : return KEY_Y;
				case DIKS_CAPITAL_Z               : return KEY_Z;
				case DIKS_SQUARE_BRACKET_LEFT     : return -1;
				case DIKS_BACKSLASH               : return KEY_BACKSLASH;
				case DIKS_SQUARE_BRACKET_RIGHT    : return -1;
				case DIKS_CIRCUMFLEX_ACCENT       : return -1;
				case DIKS_UNDERSCORE              : return -1;
				case DIKS_GRAVE_ACCENT            : return -1;
				case DIKS_SMALL_A                 : return KEY_A;
				case DIKS_SMALL_B                 : return KEY_B;
				case DIKS_SMALL_C                 : return KEY_C;
				case DIKS_SMALL_D                 : return KEY_D;
				case DIKS_SMALL_E                 : return KEY_E;
				case DIKS_SMALL_F                 : return KEY_F;
				case DIKS_SMALL_G                 : return KEY_G;
				case DIKS_SMALL_H                 : return KEY_H;
				case DIKS_SMALL_I                 : return KEY_I;
				case DIKS_SMALL_J                 : return KEY_J;
				case DIKS_SMALL_K                 : return KEY_K;
				case DIKS_SMALL_L                 : return KEY_L;
				case DIKS_SMALL_M                 : return KEY_M;
				case DIKS_SMALL_N                 : return KEY_N;
				case DIKS_SMALL_O                 : return KEY_O;
				case DIKS_SMALL_P                 : return KEY_P;
				case DIKS_SMALL_Q                 : return KEY_Q;
				case DIKS_SMALL_R                 : return KEY_R;
				case DIKS_SMALL_S                 : return KEY_S;
				case DIKS_SMALL_T                 : return KEY_T;
				case DIKS_SMALL_U                 : return KEY_U;
				case DIKS_SMALL_V                 : return KEY_V;
				case DIKS_SMALL_W                 : return KEY_W;
				case DIKS_SMALL_X                 : return KEY_X;
				case DIKS_SMALL_Y                 : return KEY_Y;
				case DIKS_SMALL_Z                 : return KEY_Z;
				case DIKS_CURLY_BRACKET_LEFT      : return -1;
				case DIKS_VERTICAL_BAR            : return -1;
				case DIKS_CURLY_BRACKET_RIGHT     : return -1;
				case DIKS_TILDE                   : return -1;
				case DIKS_DELETE                  : return KEY_DELETE;

				/*
				* Unicode private area - DirectFB Spial keys
				*/
				case DIKS_CURSOR_LEFT             : return KEY_LEFT;
				case DIKS_CURSOR_RIGHT            : return KEY_RIGHT;
				case DIKS_CURSOR_UP               : return KEY_UP;
				case DIKS_CURSOR_DOWN             : return KEY_DOWN;
				case DIKS_INSERT                  : return KEY_INSERT;
				case DIKS_HOME                    : return KEY_HOME;
				case DIKS_END                     : return KEY_END;
				case DIKS_PAGE_UP                 : return KEY_CHANNELUP;
				case DIKS_PAGE_DOWN               : return KEY_CHANNELDOWN;
				case DIKS_PRINT                   : return -1;
				case DIKS_PAUSE                   : return KEY_PAUSE;
				case DIKS_OK                      : return KEY_OK;
				case DIKS_SELECT                  : return -1;
				case DIKS_GOTO                    : return -1;
				case DIKS_CLEAR                   : return KEY_CLEAR;
				case DIKS_POWER                   : return KEY_POWER;
				case DIKS_POWER2                  : return KEY_POWER;
				case DIKS_OPTION                  : return -1;
				case DIKS_MENU                    : return KEY_MENU;
				case DIKS_HELP                    : return KEY_HELP;
				case DIKS_INFO                    : return KEY_INFO;
				case DIKS_TIME                    : return -1;
				case DIKS_VENDOR                  : return -1;

				case DIKS_ARCHIVE                 : return -1;
				case DIKS_PROGRAM                 : return -1;
				case DIKS_CHANNEL                 : return -1;
				case DIKS_FAVORITES               : return -1;
				case DIKS_EPG                     : return -1;
				case DIKS_PVR                     : return -1;
				case DIKS_MHP                     : return -1;
				case DIKS_LANGUAGE                : return -1;
				case DIKS_TITLE                   : return -1;
				case DIKS_SUBTITLE                : return -1;
				case DIKS_ANGLE                   : return -1;
				case DIKS_ZOOM                    : return -1;
				case DIKS_MODE                    : return -1;
				case DIKS_KEYBOARD                : return -1;
				case DIKS_PC                      : return -1;
				case DIKS_SCREEN                  : return -1;

				case DIKS_TV                      : return KEY_TV;
				case DIKS_TV2                     : return KEY_TV;
				case DIKS_VCR                     : return -1;
				case DIKS_VCR2                    : return -1;
				case DIKS_SAT                     : return -1;
				case DIKS_SAT2                    : return -1;
				case DIKS_CD                      : return -1;
				case DIKS_TAPE                    : return -1;
				case DIKS_RADIO                   : return KEY_RADIO;
				case DIKS_TUNER                   : return -1;
				case DIKS_PLAYER                  : return -1;
				case DIKS_TEXT                    : return KEY_TEXT;
				case DIKS_DVD                     : return -1;
				case DIKS_AUX                     : return -1;
				case DIKS_MP3                     : return -1;
				case DIKS_PHONE                   : return -1;
				case DIKS_AUDIO                   : return KEY_AUDIO;
				case DIKS_VIDEO                   : return KEY_VIDEO;

				case DIKS_INTERNET                : return -1;
				case DIKS_MAIL                    : return -1;
				case DIKS_NEWS                    : return -1;
				case DIKS_DIRECTORY               : return -1;
				case DIKS_LIST                    : return -1;
				case DIKS_CALCULATOR              : return -1;
				case DIKS_MEMO                    : return -1;
				case DIKS_CALENDAR                : return -1;
				case DIKS_EDITOR                  : return -1;

				case DIKS_RED                     : return KEY_RED;
				case DIKS_GREEN                   : return KEY_GREEN;
				case DIKS_YELLOW                  : return KEY_YELLOW;
				case DIKS_BLUE                    : return KEY_BLUE;

				case DIKS_CHANNEL_UP              : return KEY_CHANNELUP;
				case DIKS_CHANNEL_DOWN            : return KEY_CHANNELDOWN;
				case DIKS_BACK                    : return KEY_BACK;
				case DIKS_FORWARD                 : return KEY_FORWARD;
				case DIKS_FIRST                   : return -1;
				case DIKS_LAST                    : return -1;
				case DIKS_VOLUME_UP               : return KEY_VOLUMEUP;
				case DIKS_VOLUME_DOWN             : return KEY_VOLUMEDOWN;
				case DIKS_MUTE                    : return KEY_MUTE;
				case DIKS_AB                      : return -1;
				case DIKS_PLAYPAUSE               : return -1;
				case DIKS_PLAY                    : return KEY_PLAY;
				case DIKS_STOP                    : return KEY_STOP;
				case DIKS_RESTART                 : return -1;
				case DIKS_SLOW                    : return -1;
				case DIKS_FAST                    : return -1;
				case DIKS_RECORD                  : return KEY_RECORD;
				case DIKS_EJECT                   : return -1;
				case DIKS_SHUFFLE                 : return -1;
				case DIKS_REWIND                  : return -1;
				case DIKS_FASTFORWARD             : return -1;
				case DIKS_PREVIOUS                : return KEY_PREVIOUS;
				case DIKS_NEXT                    : return KEY_NEXT;
				case DIKS_BEGIN                   : return -1;

				case DIKS_DIGITS                  : return -1;
				case DIKS_TEEN                    : return -1;
				case DIKS_TWEN                    : return -1;

				case DIKS_BREAK                   : return -1;
				case DIKS_EXIT                    : return KEY_EXIT;
				case DIKS_SETUP                   : return KEY_SETUP;

				case DIKS_CURSOR_LEFT_UP          : return -1;
				case DIKS_CURSOR_LEFT_DOWN        : return -1;
				case DIKS_CURSOR_UP_RIGHT         : return -1;
				case DIKS_CURSOR_DOWN_RIGHT       : return -1;

				/*
				* Unicode private area - DirectFB Fution keys
				*
				* More function keys are available v DFB_FUNCTION_KEY(n).
				*/
				case DIKS_F1                      : return KEY_MENU;
				case DIKS_F2                      : return KEY_VIDEO;
				case DIKS_F3                      : return KEY_INFO;
				case DIKS_F4                      : return KEY_TV;

				case DIKS_F5                      : return KEY_RADIO;
				case DIKS_F6                      : return KEY_MUTE;
				case DIKS_F7                      : return KEY_VOLUMEUP;
				case DIKS_F8                      : return KEY_VOLUMEDOWN;

				case DIKS_F9                      : return KEY_RED;
				case DIKS_F10                     : return KEY_GREEN;
				case DIKS_F11                     : return KEY_YELLOW;
				case DIKS_F12                     : return KEY_BLUE;

				/*
				* Unicode private area - DirectFB Mofier keys
				*/
				case DIKS_SHIFT                   : return -1;
				case DIKS_CONTROL                 : return -1;
				case DIKS_ALT                     : return -1;
				case DIKS_ALTGR                   : return -1;
				case DIKS_META                    : return -1;
				case DIKS_SUPER                   : return -1;
				case DIKS_HYPER                   : return -1;

				/*
				* Unicode private area - DirectFB Lock keys
				*/
				case DIKS_CAPS_LOCK               : return -1;
				case DIKS_NUM_LOCK                : return -1;
				case DIKS_SCROLL_LOCK             : return -1;

				/*
				* Unicode private area - DirectFB Dead keys
				*/
				case DIKS_DEAD_ABOVEDOT           : return -1;
				case DIKS_DEAD_ABOVERING          : return -1;
				case DIKS_DEAD_ACUTE              : return -1;
				case DIKS_DEAD_BREVE              : return -1;
				case DIKS_DEAD_CARON              : return -1;
				case DIKS_DEAD_CEDILLA            : return -1;
				case DIKS_DEAD_CIRCUMFLEX         : return -1;
				case DIKS_DEAD_DIAERESIS          : return -1;
				case DIKS_DEAD_DOUBLEACUTE        : return -1;
				case DIKS_DEAD_GRAVE              : return -1;
				case DIKS_DEAD_IOTA               : return -1;
				case DIKS_DEAD_MACRON             : return -1;
				case DIKS_DEAD_OGONEK             : return -1;
				case DIKS_DEAD_SEMIVOICED_SOUND   : return -1;
				case DIKS_DEAD_TILDE              : return -1;
				case DIKS_DEAD_VOICED_SOUND       : return -1;
			}



		break;

	}

	return -1;

}
#ifdef QBOXHD
int last_key_symbol = 0;
#endif
void eDirectFBInputDev::handleCode(long rccode)
{
#ifdef QBOXHD
	ePtr<gLCDDC> my_lcd_dc;
	gLCDDC::getInstance(my_lcd_dc);
#endif
	Input_Devices_t *ev = (Input_Devices_t *)rccode;
	
	setRCIdentifier( ev->rc_identifier );

	int key_symbol = convertKeySymbol(&ev->evt);

	switch (ev->evt.type) {
	        case DIET_KEYPRESS:
		    //eDebug("DIET_KEYPRESS %08X\n", key_symbol);
#ifdef QBOXHD
				my_lcd_dc->setUpdate(0);
				if(last_key_symbol==key_symbol)
				{
					input->keyPressed(eRCKey(this, key_symbol, eRCKey::flagRepeat));
				}
				else
				{
            	   /*emit*/ input->keyPressed(eRCKey(this, key_symbol, 0));
				}
				last_key_symbol=key_symbol;
#else
				/*emit*/ input->keyPressed(eRCKey(this, key_symbol, 0));
#endif
				break;

        	case DIET_KEYRELEASE:
		    //eDebug("DIET_KEYRELEASE %08X\n", key_symbol);
#ifdef QBOXHD
				my_lcd_dc->setUpdate(1);
#endif
				/*emit*/ input->keyPressed(eRCKey(this, key_symbol, eRCKey::flagBreak));
#ifdef QBOXHD
				my_lcd_dc->forceRefresh();
				last_key_symbol=0;
#endif
				break;

			default:
				break;
	}
}

eDirectFBInputDev::eDirectFBInputDev(eDirectFBInputEventDriver *driver)
	:eRCDevice("QBoxHD Remote Control", driver)
{ }

const char *eDirectFBInputDev::getDescription() const
{
    return "QBoxHD Remote Control";
}

