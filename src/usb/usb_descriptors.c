/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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

#include "tusb.h"

#if CFG_TUD_CDC && !CFG_TUD_MSC

/*
 * usb_descriptors.c
 *
 * TinyUSB descriptor for one CDC ACM interface.
 */

#include <string.h>

#include "tusb.h"

/* --------------------------------------------------------------------------
 * Device descriptor
 * -------------------------------------------------------------------------- */

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,

    .bcdUSB             = 0x0200,

    /*
     * CDC uses an Interface Association Descriptor (IAD), therefore use
     * the USB Miscellaneous class at device level.
     */
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    /* Replace with your own VID/PID for a released product. */
    .idVendor           = 0xCafe,
    .idProduct          = 0x4001,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

/* --------------------------------------------------------------------------
 * Configuration descriptor
 * -------------------------------------------------------------------------- */

enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL
};

enum {
    CONFIG_NUM = 1
};

#define EPNUM_CDC_NOTIF   0x81
#define EPNUM_CDC_OUT     0x02
#define EPNUM_CDC_IN      0x82

#define CONFIG_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

/*
 * Full-Speed CDC notification endpoint size.
 *
 * The data endpoints use 64-byte packets.
 */
#define CDC_NOTIF_EP_SIZE  8
#define CDC_DATA_EP_SIZE   64

uint8_t const desc_configuration[] = {
    /*
     * Configuration number,
     * interface count,
     * string index,
     * total length,
     * attributes,
     * power in mA.
     */
    TUD_CONFIG_DESCRIPTOR(
        CONFIG_NUM,
        ITF_NUM_TOTAL,
        0,
        CONFIG_TOTAL_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
        100
    ),

    /*
     * First interface number,
     * interface string index,
     * notification endpoint,
     * notification endpoint size,
     * data OUT endpoint,
     * data IN endpoint,
     * data endpoint size.
     */
    TUD_CDC_DESCRIPTOR(
        ITF_NUM_CDC,
        4,
        EPNUM_CDC_NOTIF,
        CDC_NOTIF_EP_SIZE,
        EPNUM_CDC_OUT,
        EPNUM_CDC_IN,
        CDC_DATA_EP_SIZE
    )
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;

    return desc_configuration;
}

/* --------------------------------------------------------------------------
 * String descriptors
 * -------------------------------------------------------------------------- */

enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
    STRID_CDC
};

static char const *string_desc_arr[] = {
    /*
     * Index 0 is handled separately as the language-ID descriptor.
     */
    NULL,

    "Erich Styger",
    "Zumo K22 Robot",
    "000001",
    "Zumo CDC"
};

/*
 * USB string descriptors are UTF-16. This buffer contains:
 *
 *   word 0: descriptor type and length
 *   word 1..N: UTF-16 characters
 */
static uint16_t desc_string[32];

uint16_t const *tud_descriptor_string_cb(
    uint8_t index,
    uint16_t langid
)
{
    (void)langid;

    uint8_t char_count;

    if (index == STRID_LANGID) {
        /*
         * English, United States: 0x0409
         */
        desc_string[1] = 0x0409;
        char_count = 1;
    } else {
        if (index >= TU_ARRAY_SIZE(string_desc_arr)) {
            return NULL;
        }

        char const *string = string_desc_arr[index];

        if (string == NULL) {
            return NULL;
        }

        size_t const length = strlen(string);

        /*
         * Leave word 0 for descriptor type and total length.
         */
        char_count = (uint8_t)length;

        if (char_count > 31) {
            char_count = 31;
        }

        for (uint8_t i = 0; i < char_count; ++i) {
            desc_string[1 + i] = (uint8_t)string[i];
        }
    }

    /*
     * Low byte: total descriptor length in bytes.
     * High byte: descriptor type.
     */
    desc_string[0] =
        (uint16_t)((TUSB_DESC_STRING << 8) |
                   (2 * char_count + 2));

    return desc_string;
}

#else

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
  // Length of descritor
  .bLength            = sizeof(tusb_desc_device_t),
  // Descriptor typ is device descriptor
  .bDescriptorType    = TUSB_DESC_DEVICE,
  // Is USB 2.0 standard
  .bcdUSB             = USB_BCD,

  // Use Interface Association Descriptor (IAD) for CDC
  // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
  .bDeviceClass       = TUSB_CLASS_MISC,
  .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
  .bDeviceProtocol    = MISC_PROTOCOL_IAD,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor           = USB_VID,
  .idProduct          = USB_PID,
  .bcdDevice          = 0x0100,

  .iManufacturer      = 0x01,
  .iProduct           = 0x02,
  .iSerialNumber      = 0x03,

  .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
  ITF_NUM_CDC = 0,
  ITF_NUM_CDC_DATA,
  ITF_NUM_MSC,
  ITF_NUM_TOTAL
};

  #define EPNUM_CDC_NOTIF   0x81
  #define EPNUM_CDC_OUT     0x02
  #define EPNUM_CDC_IN      0x82

  #define EPNUM_MSC_OUT     0x03
  #define EPNUM_MSC_IN      0x83

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MSC_DESC_LEN)

// full speed configuration
uint8_t const desc_fs_configuration[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

  // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

  // Interface number, string index, EP Out & EP In address, EP size
  TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 5, EPNUM_MSC_OUT, EPNUM_MSC_IN, 64),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

  return desc_fs_configuration;

}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const* string_desc_arr [] =
{
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "TinyUSB",                     // 1: Manufacturer
  "TinyUSB Device",              // 2: Product
  "123456789012",                // 3: Serials, should use chip ID
  "TinyUSB CDC",                 // 4: CDC Interface
  "TinyUSB MSC",                 // 5: MSC Interface
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  uint8_t chr_count;

  if ( index == 0)
  {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = (uint8_t) strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    // Convert ASCII string into UTF-16
    for(uint8_t i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8 ) | (2*chr_count + 2));

  return _desc_str;
}

#endif /* CFG_TUD_CDC && !CFG_TUD_MSC */