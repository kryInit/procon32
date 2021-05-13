__doc__ = """{f}

usage: 
    {f} requirement TEST_CONFIG_PATH TESTCASE_PATH [--force --result-only]
    {f} performance TEST_CONFIG_PATH TESTCASE_PATH [--no-log --score-only]
    {f} performance log TESTCASE_PATH
    {f} images --list
    {f} images add [-r -f] IMG_PATH [IMG_PATH...]
    {f} images rm IMG_NAME [IMG_NAME...]

options:
    -h, --help          show this help message
    -l, --list          show registered images
    -r, --recursive     specify a directory and add everything in it
    -f, --force         
    --forever

""".format(f=__file__)

from docopt import docopt
from lib.test import requirement_test, performance_test
from lib.testlog import dump_performance_test_log
from lib.images import dump_image_list, add_image, remove_image

# --hoge-onlyは--hoge-onlyを除いた引数でプログラムを呼び出し、出力先を適当なファイルにしておいて、
if __name__ == '__main__':
    args = docopt(__doc__)
    print(args)
    if args['requirement']:
        requirement_test(args)
    elif args['performance']:
        if args['log']:
            dump_performance_test_log(args)
        else:
            performance_test(args)
    elif args['images']:
        if args['--list']:
            dump_image_list()
        elif args['add']:
            add_image(args)
        elif args['rm']:
            remove_image(args)

