#!/usr/bin/env python3

# Copyright (C) 2024 Université de Lille
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

# Damien Amara <damien.amara@univ-lille.fr>

"""Symbols script"""


import sys


from elftools.elf.elffile import ELFFile, ELFError
from elftools.elf.sections import SymbolTableSection


# The order of the symbols matter, as it reflects the
# writing order in the symbols.bin file
SYMBOLS = [
    ( 'start'          , 'STT_FUNC'   ),
    ( '__rom_size'     , 'STT_NOTYPE' ),
    ( '__rom_ram_size' , 'STT_NOTYPE' ),
    ( '__ram_size'     , 'STT_NOTYPE' ),
    ( '__got_size'     , 'STT_NOTYPE' ),
    ( '__rom_ram_end'  , 'STT_NOTYPE' )
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


def process_symbol(sh, symbol_name, symbol_type):
    """Parse the symbol table sections to extract the st_value"""
    symbols = sh.get_symbol_by_name(symbol_name)
    if not symbols:
        die(f'.symtab: {symbol_name}: no symbol with this name')
    global_symbol = None
    if len(symbols) > 1:
        for symbol in symbols:
            if symbol['st_info']['bind'] == 'STB_GLOBAL':
                if global_symbol is not None:
                    die(f'.symtab: {symbol_name}: more than one '
                        'global symbol with this name')
                global_symbol = symbol
        if global_symbol is None:
            die(f'.symtab: {symbol_name}: no global symbol '
                'exists with this name')
    else:
        symbol = symbols[0]
        if symbol['st_info']['bind'] != 'STB_GLOBAL':
            die(f'.symtab: {symbol_name}: not a global symbol')
        global_symbol = symbol
    if global_symbol['st_info']['type'] != symbol_type:
        die(f'.symtab: {symbol_name}: expected {symbol_type} '
            'symbol type')
    return to_word(global_symbol.entry['st_value'])


def process_symbols(elf, symbols):
    """Process each symbol"""
    sh = elf.get_section_by_name('.symtab')
    if not sh:
        die(f'.symtab: no section with this name found')
    if not isinstance(sh, SymbolTableSection):
        die(f'.symtab: is not a symbol table section')
    if sh['sh_type'] != 'SHT_SYMTAB':
        die(f'.symtab: is not a SHT_SYMTAB section')
    xs = bytearray()
    for symbol in symbols:
        xs += process_symbol(sh, symbol[0], symbol[1])
    return xs


if __name__ == '__main__':
    try:
        with open(sys.argv[1], 'rb') as f:
            xs = process_symbols(ELFFile(f), SYMBOLS)
        with open(sys.argv[2], 'wb') as f:
            f.write(xs)
        sys.exit(0)
    except FileNotFoundError as e:
        die(f'{sys.argv[1]}: no such file or directory')
    except ELFError as e:
        die(f'{sys.argv[1]}: {str(e)}')
    except IndexError:
        usage()
