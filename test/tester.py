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
import random
import hashlib
import numpy as np
from PIL import Image
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


def validate_config_value(config):
    if 'name' not in config:
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

    if not isinstance(config['divide'], dict):
        print("    -> \033[31m[ValueError]: divide must be dictionary\033[0m")
        return False

    if not isinstance(config['divide']['size'], int):
        print("    -> \033[31m[ValueError]: size must be int\033[0m")
        return False

    if not isinstance(config['divide']['h'], int):
        print("    -> \033[31m[ValueError]: h must be int\033[0m")
        return False

    if not isinstance(config['divide']['w'], int):
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

    if config['mode'] != 'resize' and config['mode'] != 'crop':
        print("    -> \033[31m[ValueError]: mode must be 'resize' or 'crop'\033[0m")
        return False

    if config['timelimit'] < 300 or config['timelimit'] > 1200:
        print("    -> \033[31m[ValueError]: timelimit range must be [300, 1200]\033[0m")
        return False

    if config['selectable_times'] < 2 or config['selectable_times'] > 128:
        print("    -> \033[31m[ValueError]: timelimit range must be [2, 128]\033[0m")
        return False

    if config['divide']['w'] < 2 or config['divide']['w'] > 16:
        print("    -> \033[31m[ValueError]: w range must be [2, 16]\033[0m")
        return False

    if config['divide']['h'] < 2 or config['divide']['h'] > 16:
        print("    -> \033[31m[ValueError]: h range must be [2, 16]\033[0m")
        return False

    if config['divide']['size'] < 16 or config['divide']['size'] > 256:
        print("    -> \033[31m[ValueError]: size range must be [16, 256]\033[0m")
        return False

    if config['divide']['size'] * config['divide']['w'] > 2048:
        print("    -> \033[31m[ValueError]: size*w range must be [32, 2048]\033[0m")
        return False

    if config['divide']['size'] * config['divide']['h'] > 2048:
        print("    -> \033[31m[ValueError]: size*h range must be [32, 2048]\033[0m")
        return False

    if config['cost']['choice'] < 1 or config['cost']['choice'] > 500:
        print("    -> \033[31m[ValueError]: choice range must be [1, 500]\033[0m")
        return False

    if config['cost']['repl'] < 1 or config['cost']['repl'] > 500:
        print("    -> \033[31m[ValueError]: repl range must be [1, 500]\033[0m")
        return False

    return True


def fill_config_value(config):
    if 'random_state' not in config:
        random.seed()
        config['random_state'] = random.randint(0, 100000000)
    random.seed(config['random_state'])

    if 'mode' not in config:
        config['mode'] = 'resize'

    if 'timelimit' not in config:
        config['timelimit'] = random.randint(300, 1200)

    if 'selectable_times' not in config:
        config['selectable_times'] = random.randint(2, 128)

    if 'divide' not in config:
        config['divide'] = dict()

    if 'h' not in config['divide']:
        config['divide']['h'] = random.randint(2, 16)

    if 'w' not in config['divide']:
        config['divide']['w'] = random.randint(2, 16)

    if 'size' not in config['divide']:
        if isinstance(config['divide']['w'], int) and isinstance(config['divide']['h'], int):
            config['divide']['size'] = random.randint(16, min(256, int(2048 / max(config['divide']['w'], config['divide']['h']))))
        else:
            config['divide']['size'] = random.randint(16, 256)

    if 'cost' not in config:
        config['cost'] = dict()

    if 'choice' not in config['cost']:
        config['cost']['choice'] = random.randint(1, 500)

    if 'repl' not in config['cost']:
        config['cost']['repl'] = random.randint(1, 100)


def add_info_by_comment(path, config):
    with open(path, mode='rb') as f:
        img = f.readlines()

    div = config['divide']

    cost = config['cost']
    choice_cost = cost['choice']
    repl_cost = cost['repl']

    img.insert(1, ('# ' + str(div['w']) + ' ' + str(div['h']) + '\n').encode())
    img.insert(2, ('# ' + str(config['selectable_times']) + '\n').encode())
    img.insert(3, ('# ' + str(choice_cost) + ' ' + str(repl_cost) + '\n').encode())

    with open(path, mode='wb') as f:
        f.writelines(img)


def shuffle_and_rotate_img(img, config):
    random.seed(config['random_state'])
    div = config['divide']
    div_size = div['size']
    div_w = div['w']
    div_h = div['h']

    order = list(range(div_h*div_w))
    random.shuffle(order)

    split_imgs = []
    for y in range(div_h):
        for x in range(div_w):
            split_imgs.append(img.crop((x*div_size, y*div_size, (x+1)*div_size, (y+1)*div_size)))

    rotate_log = []

    for y in range(div_h):
        for x in range(div_w):
            idx = y*div_w + x
            direction = random.randint(0,3)
            if y == 0 and x == 0:
                direction = 0
            rotate_log.append(direction)
            img.paste(split_imgs[order[idx]].rotate(direction*90), (x*div_size, y*div_size))

    shuffle_log = np.zeros(div_h*div_w, object)
    for y in range(div_h):
        for x in range(div_w):
            idx = y*div_w + x
            shuffle_log[order[idx]] = '{:X}{:X}'.format(x, y)

    shuffle_log = shuffle_log.reshape(div_h, div_w)
    return shuffle_log, rotate_log


def create_img_and_ans_for_test(testcase_name, config):
    print('[' + testcase_name + '] creating image and answer for test')

    fill_config_value(config)
    if not validate_config_value(config):
        return False

    org_name = config['name']
    in_path = IMG_DIR + '/' + org_name + '.ppm'

    # 存在しなければreturn
    if not os.path.isfile(in_path):
        print('    -> \033[31m[FileNotFoundError]: No such image: {}\033[0m'.format(org_name))
        return False

    HASH = hashlib.md5(toml.dumps(config).encode('utf-8')).hexdigest()
    out_path = DATA_DIR + '/' + HASH

    img = Image.open(in_path)

    div = config['divide']
    w_size = div['size'] * div['w']
    h_size = div['size'] * div['h']

    if config['mode'] == 'resize':
        img = img.resize((w_size, h_size))

    if config['mode'] == 'crop':
        img = img.crop((0, 0, w_size, h_size))

    shuffle_log, rotate_log = shuffle_and_rotate_img(img, config)

    if not os.path.isdir(out_path):
        os.mkdir(out_path)

    img_path = out_path + '/prob.ppm'
    img.save(img_path)
    add_info_by_comment(img_path, config)

    ans_path = out_path + '/true_ans.txt'
    with open(ans_path, mode='w') as f:
        f.writelines(map(str, rotate_log))
        f.write('\n')
        f.write('\n'.join(map(lambda x: ' '.join(x), shuffle_log) ))

    print('    -> \033[32mcreated\033[0m')
    return True


def run_test(path):
    print('\nchecking toml file: ' + path)
    
    try:
        testcases = toml.load(open(args['PATH'][0]))
    except Exception:
        print('    -> \033[31m[{}]: {}\033[0m'.format(sys.exc_info()[0].__name__, sys.exc_info()[1]))
        return False
        
    print('    -> \033[32mpassed\033[0m\n')
    print('testing with ' + path + '\n')
    for testcase_name, config in testcases.items():
        print('[' + testcase_name + '] start test')

        if not create_img_and_ans_for_test(testcase_name, config):
            print('\033[31m[' + testcase_name + '] failed\033[0m\n')

        print('[' + testcase_name + '] \033[32mpassed\033[0m\n')

    return True


if __name__ == '__main__':
    args = docopt(__doc__)
    if args['run']:
        run_test(args['PATH'][0])

    elif args['images']:

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

