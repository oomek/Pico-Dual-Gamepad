
#include "Adafruit_TinyUSB.h"

/* This sketch demonstrates USB HID gamepad use.
 * This sketch is only valid on boards which have native USB support
 * and compatibility with Adafruit TinyUSB library.
 * For example SAMD21, SAMD51, nRF52840.
 *
 * Make sure you select the TinyUSB USB stack if you have a SAMD board.
 * You can test the gamepad on a Windows system by pressing WIN+R, writing Joy.cpl and pressing Enter.
 */

// // HID report descriptor using TinyUSB's template
// // Single Report (no ID) descriptor
// uint8_t const desc_hid_report[] =
// {
//   TUD_HID_REPORT_DESC_GAMEPAD(),
// };

// // USB HID object. For ESP32 these values cannot be changed after this declaration
// // desc report, desc len, protocol, interval, use out endpoint
// Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);


#define MY_GAMEPAD(...) \
	HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                 ,\
	HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  )                 ,\
	HID_COLLECTION ( HID_COLLECTION_APPLICATION )                 ,\
		/* Report ID if any */\
		__VA_ARGS__ \
		/* 8 bit X, Y, Z, Rz, Rx, Ry (min -127, max 127 ) */ \
		HID_USAGE_PAGE   ( HID_USAGE_PAGE_DESKTOP                 ) ,\
		HID_USAGE        ( HID_USAGE_DESKTOP_X                    ) ,\
		HID_USAGE        ( HID_USAGE_DESKTOP_Y                    ) ,\
		/*HID_USAGE        ( HID_USAGE_DESKTOP_Z                    ) ,*/\
		/*HID_USAGE        ( HID_USAGE_DESKTOP_RZ                   ) ,*/\
		/*HID_USAGE        ( HID_USAGE_DESKTOP_RX                   ) ,*/\
		/*HID_USAGE        ( HID_USAGE_DESKTOP_RY                   ) ,*/\
		HID_LOGICAL_MIN  ( 0x81                                   ) ,\
		HID_LOGICAL_MAX  ( 0x7f                                   ) ,\
		HID_REPORT_COUNT ( 2                                      ) ,\
		HID_REPORT_SIZE  ( 8                                      ) ,\
		HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
		/* 8 bit DPad/Hat Button Map  */ \
		/*HID_USAGE_PAGE   ( HID_USAGE_PAGE_DESKTOP                 ) ,*/\
		/*HID_USAGE        ( HID_USAGE_DESKTOP_HAT_SWITCH           ) ,*/\
		/*HID_LOGICAL_MIN  ( 1                                      ) ,*/\
		/*HID_LOGICAL_MAX  ( 8                                      ) ,*/\
		/*HID_PHYSICAL_MIN ( 0                                      ) ,*/\
		/*HID_PHYSICAL_MAX_N ( 315, 2                               ) ,*/\
		/*HID_REPORT_COUNT ( 1                                      ) ,*/\
		/*HID_REPORT_SIZE  ( 8                                      ) ,*/\
		/*HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,*/\
		/* 16 bit Button Map */ \
		HID_USAGE_PAGE   ( HID_USAGE_PAGE_BUTTON                  ) ,\
		HID_USAGE_MIN    ( 1                                      ) ,\
		HID_USAGE_MAX    ( 9                                      ) ,\
		HID_LOGICAL_MIN  ( 0                                      ) ,\
		HID_LOGICAL_MAX  ( 1                                      ) ,\
		HID_REPORT_COUNT ( 32                                     ) ,\
		HID_REPORT_SIZE  ( 1                                      ) ,\
		HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
	HID_COLLECTION_END \


// Report ID
enum
{
	RID_GAMEPAD_A = 1,
	RID_GAMEPAD_B
};

// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] =
{
	MY_GAMEPAD( HID_REPORT_ID(RID_GAMEPAD_A)),
	MY_GAMEPAD( HID_REPORT_ID(RID_GAMEPAD_B))
};

// USB HID object. For ESP32 these values cannot be changed after this declaration
// desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 1, false);

typedef struct TU_ATTR_PACKED
{
	int8_t  x;         ///< Delta x  movement of left analog-stick
	int8_t  y;         ///< Delta y  movement of left analog-stick
	uint32_t buttons;  ///< Buttons mask for currently pressed buttons
}my_report;


