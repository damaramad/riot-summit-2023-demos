#!/usr/bin/env python3

# Copyright (C) 2024 Université de Lille
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

# Damien Amara <damien.amara@univ-lille.fr>

"""Padding script"""


import pathlib, sys


# The default value used for padding. This value corresponds to
# the default state of non-volatile NAND flash memories
PADDING_VALUE = b'\xff'


# The Pip binary size must be a multiple of this value. It
# corresponds to the minimum alignment required by the MPU of
# the ARMv7-M architecture
MPU_ALIGNMENT = 32


def usage():
    """Print how to to use this script and exit"""
    print(f'usage: {sys.argv[0]} BINARY PADDING')
    sys.exit(1)


def round(x, y):
    """Round x to the next power of two y"""
    return ((x + y - 1) & ~(y - 1))


if __name__ == '__main__':
    try:
        size = pathlib.Path(sys.argv[1]).stat().st_size
        padding = round(size, MPU_ALIGNMENT) - size
        with open(sys.argv[2], 'wb') as f:
            f.write(padding * PADDING_VALUE)
        sys.exit(0)
    except IndexError:
        usage()
