#include "Arduino.h"
#include "Adafruit_TinyUSB.h"
#include "hid_desc.hpp"

#define LED_PIN PICO_DEFAULT_LED_PIN

#define DEBOUNCE_MS 8ul
#define PICO_GPIO_MASK 0x1C7FFFFF
#define PICO_GPIO_COUNT 26

#define LED_ENABLED // Comment to disable the LED
#define WAKE_ENABLED // Comment to disable waking from sleep

#define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))
#define REBOOT AIRCR_Register = 0x5FA0004;


Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 1, false);

// Button to GPIO array
uint16_t buttons[PICO_GPIO_COUNT] =
{
	// Gamepad A
	2,3,4,5,           // up, down, left, right
	10,11,12,13,14,15, // buttons 1-6
	0,28,27,           // buttons 7,8,9

	// Gamepad B
	6,7,8,9,           // up, down, left, right
	21,20,19,18,17,16, // buttons 1-6
	1,26,22            // buttons 7,8,9
};

pd_report gamepadA;
pd_report gamepadB;

uint32_t gpio_now = 0;
uint32_t gpio_old = 0;

uint32_t timers[PICO_GPIO_COUNT];
uint32_t states[PICO_GPIO_COUNT];

uint32_t time_now = 0;

bool sendA = false;
bool sendB = false;


void setup()
{
    Serial.end();
    TinyUSBDevice.setID(0x16c0, 0x05e1);
    TinyUSBDevice.setProductDescriptor("Pico Dual Gamepad");
	gpio_init_mask(PICO_GPIO_MASK);
	gpio_set_dir_masked(PICO_GPIO_MASK, GPIO_IN);

	// Enable pull-up resistors on all inputs
  	for (int i = 0; i <= 28; i++)
	  if (PICO_GPIO_MASK & (1ul << i)) gpio_pull_up(i);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	usb_hid.begin();
	while( !TinyUSBDevice.mounted() ) sleep_ms(1);

	// Set initial states
	for ( uint16_t i = 0; i < PICO_GPIO_COUNT; i++ )
	{
		timers[i] = time_us_64() / 1000;
		states[i] = false;
	}
}

void loop()
{
    // Get all GPIO and invers since we're using pull-ups
    gpio_now = ~gpio_get_all();
    gpio_now &= PICO_GPIO_MASK;
    time_now = time_us_64() / 1000;

#ifdef LED_ENABLED
    if (gpio_now) gpio_put(LED_PIN, true);
    else gpio_put(LED_PIN, false);
#endif

#ifdef WAKE_ENABLED
		if (TinyUSBDevice.suspended() && gpio_now)
		{
			// Wake up the host if it's in suspended state
			// and REMOTE_WAKEUP feature is enabled by the host
			TinyUSBDevice.remoteWakeup();

			// Reboot Pico while host is resuming from sleep
			// to prevent Pico locking up
			REBOOT;
			exit(0);
		}
#endif

    for (int i = 0; i < PICO_GPIO_COUNT; i++)
    {
        if (time_now - timers[i] > DEBOUNCE_MS)
        {
            uint32_t state = (gpio_now & (1ul << buttons[i])) ? 1 : 0;
            if (state != states[i])
            {
                states[i] = state;
                timers[i] = time_now;
                if ( i < 13 )
                    sendA = true;
                else
                    sendB = true;
            }
        }
    }

    if (sendA)
    {
        gamepadA.x = states[2] * -127 + states[3] * 127;
        gamepadA.y = states[0] * -127 + states[1] * 127;

        gamepadA.buttons = 0ul
        | (states[4]  << 0)
        | (states[5]  << 1)
        | (states[6]  << 2)
        | (states[7]  << 3)
        | (states[8]  << 4)
        | (states[9]  << 5)
        | (states[10] << 6)
        | (states[11] << 7)
        | (states[12] << 8);

        while(!usb_hid.ready());
        usb_hid.sendReport(RID_GAMEPAD_A, &gamepadA, sizeof(gamepadA));
        sendA = false;
    }

    if (sendB)
    {
        gamepadB.x = states[15] * -127 + states[16] * 127;
        gamepadB.y = states[13] * -127 + states[14] * 127;

        gamepadB.buttons = 0ul
        | (states[17] << 0)
        | (states[18] << 1)
        | (states[19] << 2)
        | (states[20] << 3)
        | (states[21] << 4)
        | (states[22] << 5)
        | (states[23] << 6)
        | (states[24] << 7)
        | (states[25] << 8);

        while(!usb_hid.ready());
        usb_hid.sendReport(RID_GAMEPAD_B, &gamepadB, sizeof(gamepadB));
        sendB = false;
    }
}
