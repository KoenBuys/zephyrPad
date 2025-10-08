#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ws2812_demo, LOG_LEVEL_INF);

#define STRIP_NODE DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)

#define DELAY_TIME K_MSEC(50)

static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);

/* Array to hold pixel colors */
static struct led_rgb pixels[STRIP_NUM_PIXELS];

/* Update the LED strip with current pixel values */
static int update_strip(void)
{
	return led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
}

/* Set all LEDs to a specific color */
static void set_all_leds(uint8_t r, uint8_t g, uint8_t b)
{
	for (int i = 0; i < STRIP_NUM_PIXELS; i++) {
		pixels[i].r = r;
		pixels[i].g = g;
		pixels[i].b = b;
	}
}

/* Rainbow effect */
static void rainbow_effect(void)
{
	static uint8_t hue = 0;
	
	for (int i = 0; i < STRIP_NUM_PIXELS; i++) {
		uint8_t pixel_hue = (hue + (i * 256 / STRIP_NUM_PIXELS)) % 256;
		
		/* Simple HSV to RGB conversion (S=255, V=128) */
		uint8_t sector = pixel_hue / 43;
		uint8_t offset = pixel_hue % 43;
		uint8_t brightness = 128;
		
		switch (sector) {
		case 0:
			pixels[i].r = brightness;
			pixels[i].g = (offset * brightness) / 43;
			pixels[i].b = 0;
			break;
		case 1:
			pixels[i].r = brightness - ((offset * brightness) / 43);
			pixels[i].g = brightness;
			pixels[i].b = 0;
			break;
		case 2:
			pixels[i].r = 0;
			pixels[i].g = brightness;
			pixels[i].b = (offset * brightness) / 43;
			break;
		case 3:
			pixels[i].r = 0;
			pixels[i].g = brightness - ((offset * brightness) / 43);
			pixels[i].b = brightness;
			break;
		case 4:
			pixels[i].r = (offset * brightness) / 43;
			pixels[i].g = 0;
			pixels[i].b = brightness;
			break;
		case 5:
			pixels[i].r = brightness;
			pixels[i].g = 0;
			pixels[i].b = brightness - ((offset * brightness) / 43);
			break;
		}
	}
	
	hue += 2;
}

/* Chase effect */
static void chase_effect(uint8_t r, uint8_t g, uint8_t b)
{
	static int pos = 0;
	
	set_all_leds(0, 0, 0);
	pixels[pos].r = r;
	pixels[pos].g = g;
	pixels[pos].b = b;
	
	pos = (pos + 1) % STRIP_NUM_PIXELS;
}

int main(void)
{
	int rc;
	int effect = 0;
	int counter = 0;

	printk("WS2812 LED Strip Demo on ESP32-S3\n");

	if (!device_is_ready(strip)) {
		printk("LED strip device is not ready\n");
		return -ENODEV;
	}

	printk("Found LED strip device\n");
	printk("Number of LEDs: %d\n", STRIP_NUM_PIXELS);

	/* Clear all LEDs initially */
	set_all_leds(0, 0, 0);
	update_strip();

	while (1) {
		switch (effect) {
		case 0:
			/* Solid red */
			set_all_leds(255, 0, 0);
            update_strip();
			break;
		case 1:
			/* Solid green */
			set_all_leds(0, 255, 0);
            update_strip();
			break;
		case 2:
			/* Solid blue */
			set_all_leds(0, 0, 255);
            update_strip();
			break;
		case 3:
			/* Rainbow effect */
			rainbow_effect();
			break;
		case 4:
			/* Chase effect */
			chase_effect(255, 255, 255);
			break;
		}

		rc = update_strip();
		if (rc) {
			printk("Failed to update LED strip: %d\n", rc);
		}

		k_sleep(DELAY_TIME);
		
		/* Change effect every 5 seconds (100 x 50ms) */
		counter++;
		if (counter >= 100) {
			counter = 0;
			effect = (effect + 1) % 5;
			printk("Switching to effect %d\n", effect);
		}
	}

	return 0;
}