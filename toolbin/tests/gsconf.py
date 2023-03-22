#!/usr/bin/python
# Copyright (C) 2001-2021 Artifex Software, Inc.
# All Rights Reserved.
#
# This software is provided AS-IS with no warranty, either express or
# implied.
#
# This software is distributed under license and may not be copied,
# modified or distributed except as expressly authorized under the terms
# of the license contained in the file LICENSE in this distribution.
#
# Refer to licensing information at http://www.artifex.com or contact
# Artifex Software, Inc.,  1305 Grant Avenue - Suite 200, Novato,
# CA 94945, U.S.A., +1(415)492-9861, for further information.
#


# gsconf.py
#
# configuration file parser for regression tests

from __future__ import print_function
import os
import pwd
import re
import sys
import time
import logging

# python3 compatibility
try:
    FileNotFoundError
except NameError:
    FileNotFoundError = OSError

CALLER = os.path.splitext(os.path.basename(sys.argv[0]))[0]
FILENAME = os.path.join(os.path.dirname(sys.argv[0]), 'testing.cfg')
USER = pwd.getpwuid(os.geteuid()).pw_name
TEST_CONFIG = {  # set some sane (?) defaults in case no testing.cfg exists
    'report_from': CALLER + '@localhost',
    'report_to': USER + '@localhost',
}

def parse_config(source=FILENAME, config=None):
    r'''
    read and update configuration from array or from file

    >>> TEST_CONFIG['report_from']
    'doctest@localhost'
    >>> parse_config(['#bleah gah\r\r\r', 'parse this!\n\r\n\r\r', '1 2\t '])
    >>> TEST_CONFIG['1']
    '2\t '
    >>> TEST_CONFIG['parse']
    'this!'
    '''
    config = config or TEST_CONFIG
    if os.path.exists(str(source)):
        try:
            source = open(source, 'r')
        except (TypeError, OSError, FileNotFoundError):
            logging.error("Could not open config file '%s'.", filename)
            logging.warning("Using default configuration")
            return

    # ignore comments and blank lines
    pattern = re.compile("^((?:(?=[^#]))[^\s]+)\s+(.*)$")

    for line in map(lambda s:s.rstrip('\r\n'), source):
        match = pattern.match(line)
        if match:
            config.update((match.groups(),))

def get_dailydb_name():
    return dailydir + time.strftime("%Y%m%d", time.localtime()) # mhw + ".db"

if __name__ == '__main__':
    parse_config()
