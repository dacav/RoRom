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

#include <elfsword.h>
#include <elf.h>
#include <dacav.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

static const unsigned FLASH_START = 0x100000;
static const unsigned FLASH_END   = 0x140000;
static const unsigned RAM_START   = 0x200000;
static const unsigned RAM_END     = 0x204000;

typedef struct {
    const char *nxos_filename;
    const char *ro_filename;
    const char *rw_filename;
} options_t;

const char *optstring = "+n:o:w:h";
const struct option longopts[] = {
    {"nxos-image", 1, NULL, 'n'},
    {"ro-image", 1, NULL, 'o'},
    {"rw-image", 1, NULL, 'w'},
    {"help", 0, NULL, 'h'},
    {NULL, 0, NULL, 0}              // end of options set
};

static
void help (const char *appname)
{
    fprintf(stderr, 
"Necessary options for %s:\n"
"   --nxos-image <file> | -n <file>     Starting image.\n"
"   --ro-image <file>   | -o <file>     Output file for read-only image.\n"
"   --rw-image <file>   | -w <file>     Output file for read-write image.\n"
"\n",
            appname);
}

static
int parse_options (int argc, char **argv, options_t *opts)
{
    int opt;
    enum { 
        OPT_NONE = 0x00,
        OPT_N    = 0x01,
        OPT_O    = 0x02,
        OPT_W    = 0x04,
        OPT_ALL  = 0x07
    } optcheck = OPT_NONE;

    while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) 
            != -1) {
        switch (opt) {
            case 'n':
                opts->nxos_filename = optarg;
                optcheck |= OPT_N;
                break;
            case 'o':
                opts->ro_filename = optarg;
                optcheck |= OPT_O;
                break;
            case 'w':
                opts->rw_filename = optarg;
                optcheck |= OPT_W;
                break;
            case 'h':
                help(argv[0]);
                exit(1);
            case '?':
            default:
                return 1;
        }
    }
    return optcheck == OPT_ALL ? 0 : 1;
}

static
void write_progheads (elf_t *nxos, FILE *ro_out, FILE *rw_out)
{
    diter_t *phit = elf_proghead_iter_new(nxos);

    FILE *target;
    uint32_t vaddr;

    // The flash virtual address addendum is obtained at runtime basing on
    // the position of the read-only segment's first byte.
    int32_t flash_virtual = -1;

    while (diter_hasnext(phit)) {
        Elf32_Phdr *hdr = diter_next(phit);
        if (hdr->p_type != PT_LOAD) {
            fprintf(stderr, "Not loadable segment found, skipped\n");
            continue;
        }

        vaddr = hdr->p_vaddr;
        printf("Segment:\n"
               "    Virtual address: %p\n"
               "    Belongs to ", (void *)vaddr);
        if (vaddr >= FLASH_START && vaddr < FLASH_END) {
            target = ro_out;
            if (flash_virtual == -1) {
                flash_virtual = vaddr;
            }
            vaddr -= flash_virtual;
            printf("FLASH (subtact %p, obtaining ", (void *)flash_virtual);
        } else if (vaddr >= RAM_START && vaddr < RAM_END) {
            target = rw_out;
            vaddr -= RAM_START;
            printf("RAM (subtact %p, obtaining ", (void *)RAM_START);
        } else {
            printf("...Nothing, i was joking\n");
            fprintf(stderr, "Unbound segment, virtual addr: %p\n",
                    (void *)vaddr);
            continue;
        }
        printf("%p)\n", (void *)vaddr);

        // Position file descriptor;
        fseek(target, vaddr, SEEK_SET);

        // Stored part of the binary image
        size_t size = hdr->p_filesz;
        if (size > 0) {
            printf("    Writing %d bytes\n", size);
            fwrite(nxos->file.data8b + hdr->p_offset, size, 1, target);
        }

        // The remaining part of memsz must be zeroed
        size = hdr->p_memsz < size ? hdr->p_memsz - size : 0;
        printf("    Filling with zero %d bytes\n", size);
        while (size --) fputc(0, target);
    }
    
    elf_proghead_iter_free(phit);
}

int main (int argc, char **argv)
{
    options_t opts;
    opts.ro_filename = opts.rw_filename = opts.nxos_filename = NULL;
    if (parse_options(argc, argv, &opts)) {
        fprintf(stderr, "Try with --help\n");
        exit(1);
    }

    elf_t *nxos_img;
    elf_err_t error;

    error = elf_map_file(opts.nxos_filename, &nxos_img);
    if (error != ELF_SUCCESS) {
        fprintf(stderr, "Opening %s: %s\n", opts.nxos_filename,
                                            elf_error(error));
        exit(1);
    }

    FILE *ro_out = fopen(opts.ro_filename, "wb");
    if (ro_out == NULL) {
        fprintf(stderr, "Opening %s: %s\n", opts.ro_filename,
                strerror(errno));
        elf_release_file(nxos_img);
        exit(1);
    }

    FILE *rw_out = fopen(opts.rw_filename, "wb");
    if (rw_out == NULL) {
        elf_release_file(nxos_img);
        fclose(ro_out);
        exit(1);
    }

    write_progheads(nxos_img, ro_out, rw_out);

    fclose(ro_out);
    fclose(rw_out);
    elf_release_file(nxos_img);

    exit(0);
}

