#!/usr/bin/python
# Copyright (C) 2001-2023 Artifex Software, Inc.
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
# Artifex Software, Inc.,  39 Mesa Street, Suite 108A, San Francisco,
# CA 94129, USA, for further information.
#


# gsconf.py
#
'''
configuration file parser for regression tests
'''

from __future__ import print_function
import os
import pwd
import re
import sys
import time
import logging
import tempfile

# python3 compatibility
try:
    FileNotFoundError
except NameError:
    FileNotFoundError = OSError

CALLER = os.path.splitext(os.path.basename(sys.argv[0]))[0]
TESTDIR = os.path.dirname(os.path.realpath(sys.argv[0]))
TOOLBIN = os.path.dirname(TESTDIR)
GSROOT = os.path.dirname(TOOLBIN)
if 'toolbin' not in os.listdir(GSROOT):
    raise FileNotFoundError('Unexpected directory structure, cannot continue')
CONFIGFILE = os.path.join(TESTDIR, 'testing.cfg')
USER = pwd.getpwuid(os.geteuid()).pw_name
TEMPDIR = tempfile.mkdtemp()
INSTALLTREE = os.path.join(TEMPDIR, 'install')
LOGDIR = os.path.join(TEMPDIR, 'log')
UPDATE_STDOUT = os.path.join(LOGDIR, 'update.log')
UPDATE_STDERR = os.path.join(LOGDIR, 'update.err')
MAKE_STDOUT = os.path.join(LOGDIR, 'make.log')
MAKE_STDERR = os.path.join(LOGDIR, 'make.err')
CONFIG_STDOUT = os.path.join(LOGDIR, 'config.log')
CONFIG_STDERR = os.path.join(LOGDIR, 'config.err')
INSTALL_STDOUT = os.path.join(LOGDIR, 'install.log')
INSTALL_STDERR = os.path.join(LOGDIR, 'install.err')
CUMULATIVE = 'cumulative.log'
HISTORY = 'history.log'
DEFAULT_CONFIG = {  # set some sane (?) defaults in case no testing.cfg exists
    'report_from': CALLER + '@localhost',
    'report_to': USER + '@localhost',
    'gsroot': GSROOT,
    'installtree': INSTALLTREE,
    'logdir': LOGDIR,
    'update_stdout': UPDATE_STDOUT,
    'update_stderr': UPDATE_STDERR,
    'make_stdout': MAKE_STDOUT,
    'make_stderr': MAKE_STDERR,
    'config_stdout': CONFIG_STDOUT,
    'config_stderr': CONFIG_STDERR,
    'install_stdout': INSTALL_STDOUT,
    'install_stderr': INSTALL_STDERR,
    'cumulative': CUMULATIVE,
    'history': HISTORY,
}
# jc@unternet.net thinks this is a smelly hack, but since I'm not really sure
# why Artifex chose to use the module itself to store the config, I don't
# dare mess with it.

if 'gsconf' not in sys.modules:  # the case if this file is run directly
    import gsconf  # pylint: disable=import-self, unused-import
GSCONF = sys.modules['gsconf']
GSCONF.__dict__.update(DEFAULT_CONFIG)

def parse_config(source=CONFIGFILE, config=None):
    r'''
    read and update configuration from array or from file

    >>> length = len(GSCONF.__dict__)
    >>> GSCONF.report_from in ['doctest@localhost', 'gsconf@localhost']
    True
    >>> parse_config(['#bleah gah\r\r\r', 'parse this!\n\r\n\r\r', 't 2\t '])
    >>> GSCONF.t
    '2\t '
    >>> GSCONF.parse
    'this!'
    >>> len(GSCONF.__dict__) - length  # make sure the comment wasn't included
    2
    '''
    config = config or GSCONF
    if os.path.exists(str(source)):
        try:
            source = open(source, 'r')
        except (TypeError, OSError, FileNotFoundError):
            logging.error("Could not open config file '%s'.", source)
            logging.warning("Using default configuration")
            return

    # ignore comments and blank lines
    pattern = re.compile(r'^((?:(?=[^#]))[^\s]+)\s+(.*)$')
    # pylint: disable=deprecated-lambda
    for line in map(lambda s: s.rstrip('\r\n'), source):
        match = pattern.match(line)
        if match:
            config.__dict__.update((match.groups(),))

def get_dailydb_name():
    '''
    assumes dailydir has already been read from configuration file
    '''
    # pylint: disable=undefined-variable
    timestamp = time.strftime("%Y%m%d", time.localtime()) # mhw + ".db"
    try:
        return dailydir + timestamp # mhw + ".db"
    except NameError:
        logging.fatal('gsconf.dailydir has not been defined')
        raise

parse_config()

if __name__ == '__main__':
    import doctest
    doctest.testmod()
    for key, value in DEFAULT_CONFIG.iteritems():
        print(key + ':', value)
