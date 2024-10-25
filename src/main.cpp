/*
 * The MIT License (MIT)
 *
 * HID Dual Gamepad firmware for Raspberry Pi Pico
 *
 * Copyright (c) 2024 Radek Dutkiewicz (radicdotkey@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */


#define LED_ENABLED // Comment to disable the LED
#define WAKE_ENABLED // Comment to disable waking from sleep
#define RA_LAYOUT // If enabled P1 has 10 buttons, P2 has 8 buttons, so P1 has more buttons to map Pause and Quit in RetroArch

#include "Arduino.h"
#include "Adafruit_TinyUSB.h"
#include "hid_desc.hpp"

#define LED_PIN PICO_DEFAULT_LED_PIN

#define DEBOUNCE_MS 8ul
#define PICO_GPIO_MASK 0x1C7FFFFF

#define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))
#define REBOOT AIRCR_Register = 0x5FA0004;


Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 1, false);

#ifdef RA_LAYOUT

// GPIO to button layout for RA
// P1 has 10 buttons, P2 has 8 buttons
uint16_t playerA_gpio[] =
{
	2,3,4,5,           // up, down, left, right
	10,11,12,13,14,15, // buttons 1-6
	0,28,27,22         // buttons 7,8,9,10
};

uint16_t playerB_gpio[] =
{
	6,7,8,9,           // up, down, left, right
	21,20,19,18,17,16, // buttons 1-6
	1,26               // buttons 7,8
};

#else

// GPIO to button layout conventional
// P1 has 9 buttons, P2 has 9 buttons
uint16_t playerA_gpio[] =
{
	2,3,4,5,           // up, down, left, right
	10,11,12,13,14,15, // buttons 1-6
	0,28,27            // buttons 7,8,9
};

uint16_t playerB_gpio[] =
{
	6,7,8,9,           // up, down, left, right
	21,20,19,18,17,16, // buttons 1-6
	1,26,22            // buttons 7,8,9
};

#endif

#define PLAYER_A_SIZE (sizeof(playerA_gpio)/sizeof(playerA_gpio[0]))
#define PLAYER_B_SIZE (sizeof(playerB_gpio)/sizeof(playerB_gpio[0]))

pd_report gamepadA;
pd_report gamepadB;

uint32_t gpio_now = 0;
uint32_t gpio_old = 0;

uint32_t timersA[PLAYER_A_SIZE];
uint32_t timersB[PLAYER_B_SIZE];

uint32_t statesA[PLAYER_A_SIZE];
uint32_t statesB[PLAYER_B_SIZE];

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
	for ( uint16_t i = 0; i < PLAYER_A_SIZE; i++ )
	{
		timersA[i] = time_us_64() / 1000;
		statesA[i] = false;
	}
    for ( uint16_t i = 0; i < PLAYER_B_SIZE; i++ )
	{
		timersB[i] = time_us_64() / 1000;
		statesB[i] = false;
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

    for (int i = 0; i < PLAYER_A_SIZE; i++)
    {
        if (time_now - timersA[i] > DEBOUNCE_MS)
        {
            uint32_t state = (gpio_now & (1ul << playerA_gpio[i])) ? 1 : 0;
            if (state != statesA[i])
            {
                statesA[i] = state;
                timersA[i] = time_now;
                sendA = true;
            }
        }
    }

    for (int i = 0; i < PLAYER_B_SIZE; i++)
    {
        if (time_now - timersB[i] > DEBOUNCE_MS)
        {
            uint32_t state = (gpio_now & (1ul << playerB_gpio[i])) ? 1 : 0;
            if (state != statesB[i])
            {
                statesB[i] = state;
                timersB[i] = time_now;
                sendB = true;
            }
        }
    }

    if (sendA)
    {
        gamepadA.x = statesA[3] - statesA[2];
        gamepadA.y = statesA[1] - statesA[0];

        gamepadA.buttons = 0ul;

        for (int i = 4; i < PLAYER_A_SIZE; i++)
            gamepadA.buttons |= (statesA[i] << i-4);

        while(!usb_hid.ready());
        usb_hid.sendReport(RID_GAMEPAD_A, &gamepadA, sizeof(gamepadA));
        sendA = false;
    }

    if (sendB)
    {
        gamepadB.x = statesB[3] - statesB[2];
        gamepadB.y = statesB[1] - statesB[0];

        gamepadB.buttons = 0ul;

        for (int i = 4; i < PLAYER_B_SIZE; i++)
            gamepadB.buttons |= (statesB[i] << i-4);

        while(!usb_hid.ready());
        usb_hid.sendReport(RID_GAMEPAD_B, &gamepadB, sizeof(gamepadB));
        sendB = false;
    }
}
