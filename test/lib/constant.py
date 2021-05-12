import os
import sys

TEST_DIR = os.path.dirname(sys.argv[0])
TEST_DIR = '.' if TEST_DIR == '' else TEST_DIR
IMG_DIR = TEST_DIR + '/.images'
DATA_DIR = os.path.normpath(TEST_DIR + '/../.data')
