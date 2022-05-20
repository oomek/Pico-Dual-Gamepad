#define PD_GAMEPAD(...) \
	HID_USAGE_PAGE   (HID_USAGE_PAGE_DESKTOP                ),\
	HID_USAGE        (HID_USAGE_DESKTOP_GAMEPAD             ),\
	HID_COLLECTION   (HID_COLLECTION_APPLICATION            ),\
	__VA_ARGS__ \
	HID_USAGE_PAGE   (HID_USAGE_PAGE_DESKTOP                ),\
	HID_USAGE        (HID_USAGE_DESKTOP_X                   ),\
	HID_USAGE        (HID_USAGE_DESKTOP_Y                   ),\
	HID_LOGICAL_MIN  (0x81                                  ),\
	HID_LOGICAL_MAX  (0x7f                                  ),\
	HID_REPORT_COUNT (2                                     ),\
	HID_REPORT_SIZE  (8                                     ),\
	HID_INPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),\
	HID_USAGE_PAGE   (HID_USAGE_PAGE_BUTTON                 ),\
	HID_USAGE_MIN    (1                                     ),\
	HID_USAGE_MAX    (9                                     ),\
	HID_LOGICAL_MIN  (0                                     ),\
	HID_LOGICAL_MAX  (1                                     ),\
	HID_REPORT_COUNT (32                                    ),\
	HID_REPORT_SIZE  (1                                     ),\
	HID_INPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),\
	HID_COLLECTION_END \

// Report IDs
enum
{
	RID_GAMEPAD_A = 1,
	RID_GAMEPAD_B
};

// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] =
{
	PD_GAMEPAD(HID_REPORT_ID(RID_GAMEPAD_A)),
	PD_GAMEPAD(HID_REPORT_ID(RID_GAMEPAD_B))
};

typedef struct TU_ATTR_PACKED
{
	int8_t  x;         ///< Delta x  movement of left analog-stick
	int8_t  y;         ///< Delta y  movement of left analog-stick
	uint32_t buttons;  ///< Buttons mask for currently pressed buttons
}pd_report;


