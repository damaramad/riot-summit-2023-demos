#!/usr/bin/env python3

# Copyright (C) 2024 Université de Lille
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

# Damien Amara <damien.amara@univ-lille.fr>

"""Relocation script"""


import sys


from elftools.elf.elffile import ELFFile, ELFError
from elftools.elf.relocation import RelocationSection
from elftools.elf.enums import ENUM_RELOC_TYPE_ARM as r_types


# The order of the relocation tables matter, as it reflects the
# writing order in the relocation.bin file
RELOCATION_TABLES = [
    '.rel.rom.ram'
]


def usage():
    """Print how to to use the script and exit"""
    print(f'usage: {sys.argv[0]} <ELF> <OUTPUT>')
    sys.exit(1)


def die(message):
    """Print error message and exit"""
    print(f'\033[91;1m{sys.argv[0]}: {message}\033[0m', file=sys.stderr)
    sys.exit(1)


def to_word(x):
    """Convert a python integer to a LE 4-bytes bytearray"""
    return x.to_bytes(4, byteorder='little')


def get_r_type(r_info):
    """Get the relocation type from r_info"""
    return r_info & 0xff


def process_table(elf, relocation_table):
    """Parse a relocation table to extract the r_offset"""
    sh = elf.get_section_by_name(relocation_table)
    if not sh:
        return to_word(0)
    if not isinstance(sh, RelocationSection):
        die(f'{relocation_table}: is not a relocation table')
    if sh.is_RELA():
        die(f'{relocation_table}: unsupported RELA')
    xs = bytearray(to_word(sh.num_relocations()))
    for i, entry in enumerate(sh.iter_relocations()):
        if get_r_type(entry['r_info']) != r_types['R_ARM_ABS32']:
            die(f'{relocation_table}: entry {i}: unsupported '
                'relocation type')
        xs += to_word(entry['r_offset'])
    return xs


def process_tables(elf, relocation_tables):
    """Process each relocation table"""
    xs = bytearray()
    for relocation_table in relocation_tables:
        xs += process_table(elf, relocation_table)
    return xs


if __name__ == '__main__':
    try:
        with open(sys.argv[1], 'rb') as f:
            xs = process_tables(ELFFile(f), RELOCATION_TABLES)
        with open(sys.argv[2], 'wb') as f:
            f.write(xs)
        sys.exit(0)
    except FileNotFoundError as e:
        die(f'{sys.argv[1]}: no such file or directory')
    except ELFError as e:
        die(f'{sys.argv[1]}: {str(e)}')
    except IndexError:
        usage()
