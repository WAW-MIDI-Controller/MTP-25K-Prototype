#include <usb_names.h>

#define MANUFACTURER_NAME          \
	{                          \
		'T', 'E', 'E', 'N', 'S', 'Y', '-', '4', '.', '1' \
	}
#define MANUFACTURER_NAME_LEN 10

#define PRODUCT_NAME                                                            \
	{                                                                       \
		'M', 'T', 'P', '-', '2', '5', 'K' \
	}
#define PRODUCT_NAME_LEN 7

#define SERIAL_NUMBER                                            \
	{                                                        \
		'M', 'T', 'P', '0', '0', '1' \
	}
#define SERIAL_NUMBER_LEN 6

struct usb_string_descriptor_struct usb_string_manufacturer_name = {
	2 + MANUFACTURER_NAME_LEN * 2,
	3,
	MANUFACTURER_NAME};

struct usb_string_descriptor_struct usb_string_product_name = {
	2 + PRODUCT_NAME_LEN * 2,
	3,
	PRODUCT_NAME};

struct usb_string_descriptor_struct usb_string_serial_number = {
	2 + SERIAL_NUMBER_LEN * 2,
	3,
	SERIAL_NUMBER};