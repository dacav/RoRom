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

#include "nxt.h"

#include <stdio.h>

/* Various constants, NXOS related */
#define TX_ENDPOINT 1
#define RX_ENDPOINT 2

/* USB Parameters of NXT/NxOS */
static const unsigned NXT_VENDOR_ID  = 0x0694;
static const unsigned NXT_PRODUCT_ID = 0xff00;

/* USB Parameters of Sam-ba */
static const unsigned SAM_VENDOR_ID  = 0x03eb;
static const unsigned SAM_PRODUCT_ID = 0x6124;

const char *nxt_strerr (nxt_err_t err)
{
    switch (err) {
        case NXT_SUCCESS:
            return "Success";
        case NXT_INITFAIL:
            return "Initialization fail";
        case NXT_SAMBA:
            return "SAM-BA found";
        case NXT_NODEV:
            return "Device not found";
        default:
            return NULL;
    }
}

const char *nxt_libusb_strerr (nxt_link_t *link)
{
    switch (link->usb_error) {
        case LIBUSB_SUCCESS:
            return "Success";
        case LIBUSB_ERROR_IO:
            return "I/O error";
        case LIBUSB_ERROR_INVALID_PARAM:
            return "Invalid param";
        case LIBUSB_ERROR_ACCESS:
            return "Access error";
        case LIBUSB_ERROR_NO_DEVICE:
            return "Device not found";
        case LIBUSB_ERROR_NOT_FOUND:
            return "Entity not found";
        case LIBUSB_ERROR_BUSY:
            return "Resource busy";
        case LIBUSB_ERROR_TIMEOUT:
            return "Timed out";
        case LIBUSB_ERROR_OVERFLOW:
            return "Overflow";
        case LIBUSB_ERROR_PIPE:
            return "Pipe error";
        case LIBUSB_ERROR_INTERRUPTED:
            return "Interrupted";
        case LIBUSB_ERROR_NO_MEM:
            return "Not enough memory";
        case LIBUSB_ERROR_NOT_SUPPORTED:
            return "Operation not supported";
        case LIBUSB_ERROR_OTHER:
        default:
            return "Unknown error";
    }
}

nxt_err_t nxt_init (nxt_link_t *nxt)
{
    libusb_context *context;
    libusb_device_handle *handle;
    int err;

    if ((err = libusb_init(&context)) != LIBUSB_SUCCESS) {
        nxt->usb_error = err;
        return NXT_INITFAIL;
    }
 
    handle = libusb_open_device_with_vid_pid(context, NXT_VENDOR_ID,
             NXT_PRODUCT_ID);
    if (handle == NULL) {
        handle = libusb_open_device_with_vid_pid(context, SAM_VENDOR_ID,
                 SAM_PRODUCT_ID);
        if (handle != NULL) { 
            libusb_close(handle);
            libusb_exit(context);
            return NXT_SAMBA;
        } else {
            return NXT_NODEV;
        }
    }

    nxt->context = context;
    nxt->handle = handle;
    return NXT_SUCCESS;
}

void nxt_free (nxt_link_t *nxt)
{
    libusb_close(nxt->handle);
    libusb_exit(nxt->context);
}

static inline
int transfer (nxt_link_t *nxt, unsigned char endp, int *transf,
              void *buffer, size_t len)
{
    return libusb_bulk_transfer(nxt->handle, endp, buffer, len, transf, 0);
}

ssize_t nxt_send (nxt_link_t *nxt, const void *buffer, size_t len)
{
    ssize_t transf;
    int err = transfer(nxt, TX_ENDPOINT, &transf, (void *)buffer, len);

    if (err != LIBUSB_SUCCESS) {
        transf = -1;
    }
    nxt->usb_error = err;
    return transf;
}

ssize_t nxt_receive (nxt_link_t *nxt, void *buffer, size_t len)
{
    ssize_t transf;
    int err = transfer(nxt, RX_ENDPOINT, &transf, buffer, len);

    if (err != LIBUSB_SUCCESS) {
        transf = -1;
    }
    nxt->usb_error = err;
    return transf;
}

