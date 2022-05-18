#include <stdio.h>
#include "boards/pico.h"
#include "hardware/gpio.h"
#include "hid_desc.hpp"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define LED_ON gpio_set_mask(1ul << LED_PIN)
#define LED_OFF gpio_clr_mask(1ul << LED_PIN)

#define DEBOUNCE_MS 8ul
#define PICO_GPIO_MASK 0x1C7FFFFF
#define PICO_GPIO_COUNT 26

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


my_report    gamepadA;
my_report    gamepadB;

uint32_t gpio_now = 0;
uint32_t gpio_old = 0;

uint32_t timers[PICO_GPIO_COUNT];
uint32_t states[PICO_GPIO_COUNT];

uint32_t time_now = 0;


void init()
{
	gpio_init_mask(PICO_GPIO_MASK);
	gpio_set_dir_masked(PICO_GPIO_MASK, GPIO_IN);

	_gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	// Enable pull-up resistors on all inputs
  	for (int i = 0; i <= 28; i++)
	  if (PICO_GPIO_MASK & (1ul << i)) gpio_pull_up(i);

	TinyUSB_Device_Init(0);
	usb_hid.begin();
	while( !TinyUSBDevice.mounted() ) sleep_ms(1);

	// Set initial states
	for ( uint16_t i = 0; i < PICO_GPIO_COUNT; i++ )
	{
		timers[i] = time_us_64() / 1000;
		states[i] = false;
	}
}

bool sendA = false;
bool sendB = false;

int main()
{
	init();
	if (!usb_hid.ready()) return 0;

	while (true)
	{
		// Get all GPIO and invers since we're using pull-ups
		gpio_now = ~gpio_get_all();
		time_now = time_us_64() / 1000;

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
			| (states[6] << 2)
			| (states[7] << 3)
			| (states[8] << 4)
			| (states[9] << 5)
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
}
