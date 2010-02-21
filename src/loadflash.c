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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nxt.h"

#ifndef DELAY_uSECS
#define DELAY_uSECS 50000
#endif

#ifndef COLS_NUMBER
#define COLS_NUMBER 48
#endif

static
int get_ro_size (const char *path)
{
    struct stat buf;
    if (stat(path, &buf) == -1) {
        return -1;
    } else {
        return buf.st_size;
    }
}

typedef struct {
    uint32_t start_address;
    int32_t length;
} tx_data_t;

static
void display_percentage (ssize_t total, tx_data_t *tx)
{
    float percentage;
    unsigned col;

    percentage = (float)(total - tx->length) / total;
    fprintf(stdout, "Position: %8p %6.2f%% |",
            (void *)tx->start_address, 100 * percentage);
    for (col = 0; col < COLS_NUMBER * percentage; col ++) {
        fprintf(stdout, "=");
    }
    fprintf(stdout, "|\r");
    fflush(stdout);
}

int main (int argc, char **argv)
{
    const char *filename;
    tx_data_t data;
    nxt_link_t nxt;
    uint8_t buffer[USB_BUFSIZE];
    size_t read, sent;
    ssize_t total;
    FILE *ro_data;

    if (argc < 2) {
        fprintf(stderr, "Provide the ROM image as first parameter.\n");
        exit(1);
    }
    filename = argv[1];

    // FIXME this should be parametrized. Basically it's the address of
    // the first unlocked flash memory location.
    data.start_address = 0x108000;

    total = data.length = get_ro_size(filename);
    if (total == -1) {
        fprintf(stderr, "Invalid ROM image\n");
        exit(1);
    }
    
    if (nxt_init(&nxt)) {
        fprintf(stderr, "Initialization failed\n");
        exit(1);
    }

    ro_data = fopen(filename, "rb");
    if (ro_data == NULL) {
        fprintf(stderr, "Error in opening %s\n", filename);
        nxt_free(&nxt);
    }

    fprintf(stdout, "Sync sent: %d bytes\n"
                    "           start=%p\n"
                    "           length=%d\n",
            nxt_send(&nxt, (void *)&data, sizeof(tx_data_t)),
            (void *)data.start_address,
            data.length);

    while ((read = fread((void *)buffer, 1, USB_BUFSIZE, ro_data)) > 0) {
        sent = nxt_send(&nxt, (void *)buffer, read);
        data.length -= sent;
        data.start_address += sent;
        display_percentage(total, &data);
        if (sent == -1) {
            fprintf(stderr, "Send failed: %s\r", nxt_libusb_strerr(&nxt));
            break;
        }
        usleep(DELAY_uSECS);
    }
    fprintf(stdout, "\nCompleted\n");

    if (ferror(ro_data)) {
        fprintf(stderr, "File error in %s\n", filename);
    }

    fclose(ro_data);
    nxt_free(&nxt);
    exit(0);
}
