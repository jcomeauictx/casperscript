#!/usr/bin/python3
'''
visual diff two pnm files
'''
import sys
from PIL import Image

def vdiff(filename1, filename2):
    '''
    generate pnm of differences between two pnm files
    '''
    file1 = open(filename1, 'r')
    file2 = open(filename2, 'r')
    pnmtypes = (next(file1).strip(), next(file2).strip())
    if pnmtypes[0] != pnmtypes[1]:
        raise ValueError('Different PNM types %s and %s' % pnmtypes)
    dimensions = (next(file1).strip(), next(file2).strip())
    if dimensions[0] != dimensions[1]:
        raise ValueError('Different dimensions %s and %s' % dimensions)
    while True:
        try:
            line1, line2 = next(file1).strip(), next(file2).strip()
            print(line1, line2)
        except StopIteration:
            break

if __name__ == '__main__':
    vdiff(*sys.argv[1:])
