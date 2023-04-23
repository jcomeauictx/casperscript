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
    with open(filename1, 'r') as file1:
        with open(filename2, 'r') as file2:
            try:
                line1, line2 = next(file1), next(file2)
                print(line1, line2)
            except:
                raise

if __name__ == '__main__':
    vdiff(*sys.argv[1:])
