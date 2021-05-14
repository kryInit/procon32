__doc__ = """{f}

usage: 
    {f} requirement TEST_CONFIG_PATH TESTCASE_PATH [--force --result-only --cleanup-all]
    {f} performance TEST_CONFIG_PATH TESTCASE_PATH [--no-log --score-only]
    {f} performance log TESTCASE_PATH
    {f} images list [--detail]
    {f} images add IMG_PATH [IMG_PATH...] [--recursive --force] 
    {f} images rm IMG_NAME [IMG_NAME...]

options:
    -h, --help          show this help message
    -r, --recursive     specify a directory and add everything in it
    -d, --detail        
    -f, --force         
    --forever

""".format(f=__file__)

from docopt import docopt
from lib.test import requirement_test, performance_test
from lib.testlog import dump_performance_test_log
from lib.images import dump_image_list, add_images, remove_images

# --hoge-onlyは--hoge-onlyを除いた引数でプログラムを呼び出し、出力先を適当なファイルにしておいて、
if __name__ == '__main__':
    args = docopt(__doc__)
    if args['requirement']:
        requirement_test(args)
    elif args['performance']:
        if args['log']:
            dump_performance_test_log(args)
        else:
            performance_test(args)
    elif args['images']:
        if args['list']:
            dump_image_list(args)
        elif args['add']:
            add_images(args)
        elif args['rm']:
            remove_images(args)

