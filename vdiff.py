#!/usr/bin/python3
'''
visual diff two pnm files
'''
import sys
import logging
from PIL import Image

logging.basicConfig(level=logging.DEBUG if __debug__ else logging.INFO)

MULTIPLIER = {
    'P1': 1,
    'P2': 1,
    'P3': 3,
}

MODES = {
    'P1': '1',
    'P2': 'L',
    'P3': 'RGB',
}

def vdiff(filename1, filename2, sidebyside=False):
    '''
    generate pnm of differences between two pnm files
    '''
    file1 = open(filename1, 'r')
    file2 = open(filename2, 'r')
    pnmtypes = (get(file1).strip(), get(file2).strip())
    if pnmtypes[0] != pnmtypes[1]:
        raise ValueError('Different PNM types %s and %s' % pnmtypes)
    logging.debug('pnmtypes: %s', pnmtypes)
    dimensions = (get(file1).strip(), get(file2).strip())
    if dimensions[0] != dimensions[1]:
        raise ValueError('Different dimensions %s and %s' % dimensions)
    dimensions = tuple(map(lambda s: tuple(map(int, s.split())), dimensions))
    logging.debug('dimensions: %s', dimensions)
    expected_length = int.__mul__(*dimensions[0]) * MULTIPLIER[pnmtypes[0]]
    logging.debug('expected file length: %d', expected_length)
    if pnmtypes[0] == 'P1':
        maxvalues = ('1', '1')
    else:
        maxvalues = (get(file1).strip(), get(file2).strip())
    logging.debug('maxvalues: %s', maxvalues)
    if maxvalues[0] != maxvalues[1]:
        raise ValueError('incompatible max values %s', maxvalues)
    maxvalues = tuple(map(int, maxvalues))
    data = [[], []]
    while True:
        try:
            raw1, raw2 = get(file1).strip(), get(file2).strip()
            if ' ' in raw1:
                raw1, raw2 = raw1.split(), raw2.split()
            data[0] += list(map(int, raw1))
            data[1] += list(map(int, raw2))
        except StopIteration:
            break
    lengths = tuple(map(len, data))
    if lengths[0] != lengths[1]:
        raise ValueError('Different lengths: %s' % lines)
    if lengths[0] != expected_length:
        raise ValueError('Unexpected length: %d != %d' %
                         (lengths[0], expected_length))
    imagebytes = (bytes(data[0]), bytes(data[1]))
    image1 = Image.frombytes(MODES[pnmtypes[0]], dimensions[0], imagebytes[0])
    image2 = Image.frombytes(MODES[pnmtypes[1]], dimensions[1], imagebytes[1])
    merged = Image.frombytes(MODES[pnmtypes[0]], dimensions[0], merge(*data))
    if sidebyside:
        image1.show()
        image2.show()
    else:
        merged.show()

def merge(data0, data1):
    '''
    return difference, as bytes, between two arrays of numbers
    '''
    return bytes([abs(data1[n] - data0[n]) for n in range(len(data0))])

def get(iterable):
    '''
    return next non-comment from iterable

    >>> get(iter(['#bleah', 'test#']))
    'test'
    '''
    value = next(iterable)
    if value.startswith('#'):
        return get(iterable)  # too many comments in a row will stackoverflow
    if '#' in value:
        value = value[0:value.index('#')]
    return value

if __name__ == '__main__':
    vdiff(*sys.argv[1:])
