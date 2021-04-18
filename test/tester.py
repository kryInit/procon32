__doc__ = """{f}

usage: 
    {f} run PATH [--forever]
    {f} images --list
    {f} images add [-r -f] PATH [PATH...]
    {f} images rm NAME [NAME...]
    {f} images cache --list
    {f} images cache clear NAME

options:
    -h, --help          show this help message
    -l, --list          show registered images
    -r, --recursive     specify a directory and add everything in it
    -f, --force         
    --forever

""".format(f=__file__)

import os
import sys
import toml
import math
import random
import hashlib
from PIL import Image, UnidentifiedImageError
from docopt import docopt

TEST_DIR = os.path.dirname(sys.argv[0])
TEST_DIR = '.' if TEST_DIR == '' else TEST_DIR
IMG_DIR = os.path.normpath(TEST_DIR + '/.images')
DATA_DIR = os.path.normpath(TEST_DIR + '/../.data')

def add_img(img_path, force):
    print('registering ' + img_path)
    
    # pathからファイル名のみを取得
    file_name = os.path.basename(img_path)

    # PIL.Imageで開けなければreturn
    try:
        img = Image.open(img_path)
    except Exception:
        print('    -> \033[31m[{}]: {}\033[0m'.format(sys.exc_info()[0].__name__, sys.exc_info()[1]))
        return False

    # forceではなく、すでに存在している場合はreturn
    out_path = IMG_DIR + '/' + os.path.splitext(file_name)[0] + ".ppm"
    if not force and os.path.exists(out_path):
        print('    -> \033[31m[RegisteredError]: this image name is already registered.\033[0m')
        return False

    # convert and save
    img.save(out_path)

    print('    -> \033[32mRegistered\033[0m')
    return True


def add_img_recursively(dir_path, force):
    # [todo] remove cache
    dir_path = os.path.normpath(dir_path)

    print('registering everything in ' + dir_path)

    if not os.path.isdir(dir_path):
        print("    -> \033[31m[DirectoryNotFoundError]: No such directory:'{}'\033[0m".format(dir_path))
        return
    
    file_name_list = os.listdir(dir_path)
    for file_name in file_name_list:
        path = dir_path + '/' + file_name
        if os.path.isdir(path):
            add_img_recursively(path, force)
        else:
            add_img(path, force)


def remove_registered_img(org_name):
    print('removing ' + org_name)
    
    name = org_name + '.ppm'

    path = IMG_DIR + '/' + name

    # 存在しなければreturn
    if not os.path.isfile(path):
        print('    -> \033[31m[FileNotFoundError]: No such image: {}\033[0m'.format(org_name))
        return
        
    os.remove(path)

    print('    -> \033[32mremoved\033[0m')
    return True


def check_config_value(config):
    if not 'name' in config:
        print("    -> \033[31m[ValueError]: name (of a image) is always required\033[0m")
        return False
    
    if not isinstance(config['name'], str):
        print("    -> \033[31m[ValueError]: mode must be string\033[0m")
        return False

    if not isinstance(config['timelimit'], int):
        print("    -> \033[31m[ValueError]: timelimit must be int\033[0m")
        return False

    if not isinstance(config['selectable_times'], int):
        print("    -> \033[31m[ValueError]: selectable_tiems must be int\033[0m")
        return False

    if not isinstance(config['partition'], dict):
        print("    -> \033[31m[ValueError]: partition must be dictionary\033[0m")
        return False

    if not isinstance(config['partition']['size'], int):
        print("    -> \033[31m[ValueError]: size must be int\033[0m")
        return False

    if not isinstance(config['partition']['h'], int):
        print("    -> \033[31m[ValueError]: h must be int\033[0m")
        return False

    if not isinstance(config['partition']['w'], int):
        print("    -> \033[31m[ValueError]: w must be int\033[0m")
        return False

    if not isinstance(config['cost'], dict):
        print("    -> \033[31m[ValueError]: cost must be dictionary\033[0m")
        return False

    if not isinstance(config['cost']['choice'], int):
        print("    -> \033[31m[ValueError]: choice must be int\033[0m")
        return False

    if not isinstance(config['cost']['repl'], int):
        print("    -> \033[31m[ValueError]: repl must be int\033[0m")
        return False

    if not isinstance(config['random_state'], int):
        print("    -> \033[31m[ValueError]: random_state must be int\033[0m")
        return False

    return True


def fill_config_value(config):
    if not 'random_state' in config:
        config['random_state'] = random.randint(0, 100000000)
    random.seed(config['random_state'])

    if not 'mode' in config:
        config['mode'] = 'resize'

    if not 'timelimit' in config:
        config['timelimit'] = random.randint(300, 1200)

    if not 'selectable_times' in config:
        config['selectable_times'] = random.randint(2, 128)

    if not 'partition' in config:
        config['partition'] = dict()

    if not 'h' in config['partition']:
        config['partition']['h'] = random.randint(2, 16)

    if not 'w' in config['partition']:
        config['partition']['w'] = random.randint(2, 16)

    if not 'size' in config['partition']: 
        config['partition']['size'] = random.randint(16, 256)

    if not 'cost' in config:
        config['cost'] = dict()

    if not 'choice' in config['cost']:
        config['cost']['choice'] = random.randint(1, 500)

    if not 'repl' in config['cost']:
        config['cost']['repl'] = random.randint(1, 100)


def add_info_by_comment(path, config):
    with open(path, mode='rb') as f:
        img = f.readlines()

    par = config['partition']

    cost = config['cost']
    choice_cost = cost['choice']
    repl_cost = cost['repl']

    img.insert(1, ('# ' + str(par['w']) + ' ' + str(par['h']) + '\n').encode())
    img.insert(2, ('# ' + str(config['selectable_times']) + '\n').encode())
    img.insert(3, ('# ' + str(choice_cost) + ' ' + str(repl_cost) + '\n').encode())

    with open(path, mode='wb') as f:
        f.writelines(img)


def create_img_for_test(testcase_name, config):
    print('[' + testcase_name + '] creating image for test')

    fill_config_value(config)
    if not check_config_value(config):
        return

    org_name = config['name']
    name = org_name + '.ppm'

    in_path = IMG_DIR + '/' + name

    # 存在しなければreturn
    if not os.path.isfile(in_path):
        print('    -> \033[31m[FileNotFoundError]: No such image: {}\033[0m'.format(org_name))
        return

    HASH = hashlib.md5(toml.dumps(config).encode('utf-8')).hexdigest()
    out_path = DATA_DIR + '/' + HASH

    img = Image.open(in_path)

    par = config['partition']
    w_size = par['size'] * par['w']
    h_size = par['size'] * par['h']

    if config['mode'] == 'resize':
        img = img.resize((w_size, h_size))

    if config['mode'] == 'crop':
        img = img.crop((0, 0, w_size, h_size))

    if not os.path.isdir(out_path):
        os.mkdir(out_path)
    img_path = out_path + '/' + name

    img.save(img_path)
    add_info_by_comment(img_path, config)

    print('    -> \033[32mcreated\033[0m')
    return True


def run_test(path):
    print('checking toml file: ' + path)
    
    try:
        testcases = toml.load(open(args['PATH'][0]))
    except Exception:
        print('    -> \033[31m[{}]: {}\033[0m'.format(sys.exc_info()[0].__name__, sys.exc_info()[1]))
        return False
        
    print('    -> \033[32mpassed\033[0m')
    print('testing with ' + path)
    for testcase_name, config in testcases.items():
        create_img_for_test(testcase_name, config)




if __name__ == '__main__':
    args = docopt(__doc__)
    print(args)
    if args['run']:
        print('run')
        run_test(args['PATH'][0])

    elif args['images']:
        print('images')

        if args['--list']:
            for i in map((lambda filename: os.path.splitext(filename)[0]), os.listdir(IMG_DIR)):
                print(i)

        elif args['add']:
            if args['--recursive']:
                for path in args['PATH']:
                    add_img_recursively(path, args['--force'])
            else:
                for path in args['PATH']:
                    add_img(path, args['--force'])

        elif args['rm']:
            for name in args['NAME']:
                remove_registered_img(name)

