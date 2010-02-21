/*
 * Copyright 2009 Giovanni Simoni
 *
 * This file is part of RoRom Utils.
 *
 * RoRom Utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RoRom Utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RoRom Utils.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __defined_nxt_h
#define __defined_nxt_h

#include <libusb-1.0/libusb.h>

// nxos usb buffer size
#define USB_BUFSIZE 64

typedef struct {
    libusb_context *context;
    libusb_device_handle *handle;
    int usb_error;
} nxt_link_t;

typedef enum {
    NXT_SUCCESS = 0,    // works fine;
    NXT_INITFAIL = 1,   // initialization error;
    NXT_SAMBA = 2,      // found samba error;
    NXT_NODEV = 3       // device not found;
} nxt_err_t;

// strerror for this module's functions
const char *nxt_strerr (nxt_err_t err);

// strerror for libnxt 1.0 (to be used if the error comes from libusb)
const char *nxt_libusb_strerr (nxt_link_t *link);

// constructor
nxt_err_t nxt_init (nxt_link_t *nxt);

// sending and receiving primitives
ssize_t nxt_send (nxt_link_t *nxt, const void *buffer, size_t len);
ssize_t nxt_receive (nxt_link_t *nxt, void *buffer, size_t len);

// destructor
void nxt_free (nxt_link_t *nxt);

#endif // __defined_nxt_h
