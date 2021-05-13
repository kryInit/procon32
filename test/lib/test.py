import os
import sys
import toml
import random
import pprint
import hashlib
import subprocess
import numpy as np
from PIL import Image
from .utility import *
from .constant import PROJECT_TOP_DIR, TEST_DIR, IMG_DIR, DATA_DIR


def validate_and_fill_recursively(data, name, condition):
    if data is None:
        if len(condition) >= 2 and condition[1] is not None:
            if callable(condition[1]):
                data = condition[1]()
            else:
                data = condition[1]
        else:
            raise ValueError(name + ' is required')

    if not isinstance(data, type(condition[0])):
        raise ValueError(name + ' must be ' + type(condition[0]).__name__)

    if len(condition) >= 3:
        message = condition[2](data)
        if message is not None:
            raise ValueError(name + message)

    if isinstance(data, dict):
        tmp = {}
        for n, t in condition[0].items():
            tmp[n] = validate_and_fill_recursively(data.get(n), name+'.'+n, t)
        data = tmp
    elif isinstance(data, list):
        tmp = [0]*len(data)
        for i in range(len(data)):
            tmp[i] = validate_and_fill_recursively(data[i], name+f'[{i}]', condition[0])
        data = tmp

    return data


def validate_and_fill_config(config):
    conditions = ({
        'preprocess': ([{
            'name': (str(), 'unknown'),
            'run': ([str()],),
            'stdout': (bool(), True),
            'stderr': (bool(), True),
        }], []),
        'postprocess': ([{
            'name': (str(), 'unknown'),
            'run': ([str()],),
            'stdout': (bool(), True),
            'stderr': (bool(), True),
        }], []),
        'test': ([{
            'name': (str(), 'unknown'),
            'type': (str(), '',
                     lambda t: " must be 'image restorer' or 'procedure builder' or 'E2E' or none"
                     if t != '' and t != 'image restorer' and t != 'procedure builder' and t != 'E2E' else None),
            'run': ([str()],),
            'stdout': (bool(), True),
            'stderr': (bool(), True),
        }],)
    },)

    return validate_and_fill_recursively(config, 'config', conditions)


def validate_and_fill_testcases(testcases):
    conditions = ({
        'name': (str(), None,
                 lambda n: f": image named '{n}' is not exists" if not os.path.exists(IMG_DIR+'/'+n+'.ppm') else None),
        'timelimit': (int(), lambda: random.randint(300, 1200)),
        'mode': (str(), 'resize',
                 lambda m: " must be crop or resize" if m != 'resize' and m != 'crop' else None),
        'selectable_times': (int(), lambda: random.randint(2, 128)),
        'random_state': (int(),),
        'divide': ({
            'size': (int(), lambda: random.randint(16, 128),
                     lambda s: " range must be [16, 256]" if s < 16 or 256 < s else None),
            'h': (int(), lambda: random.randint(2, 8),
                  lambda h: " range must be [2, 16]" if h < 2 or 16 < h else None),
            'w': (int(), lambda: random.randint(2, 8),
                  lambda w: " range must be [2, 16]" if w < 2 or 16 < w else None),
        }, lambda: {'size': random.randint(16, 128), 'h': random.randint(2, 8), 'w': random.randint(2, 8)}),
        'cost': ({
            'choice': (int(), lambda: random.randint(1, 500),
                       lambda c: " range must be [1, 500]" if c < 1 or 500 < c else None),
            'repl': (int(), lambda: random.randint(1, 100),
                     lambda r: " range must be [1, 100]" if r < 1 or 100 < r else None)
        }, lambda: {'choice': random.randint(1, 500), 'repl': random.randint(1, 100)})
    },)

    for test_name, testcase in testcases.items():
        if 'random_state' not in testcase:
            random.seed()
            seed = random.randint(0, 100000)
            random.seed(seed)
            testcases[test_name]['random_state'] = seed
        else:
            if not isinstance(testcase['random_state'], int):
                raise ValueError(f'testcases[{test_name}].random_state must be int')
            random.seed(testcase['random_state'])

        testcases[test_name] = validate_and_fill_recursively(testcase, f'testcases[{test_name}]', conditions)


def add_info_by_comment(path, testcase):
    with open(path, mode='rb') as f:
        img = f.readlines()

    div = testcase['divide']

    cost = testcase['cost']
    choice_cost = cost['choice']
    repl_cost = cost['repl']

    img.insert(1, ('# ' + str(div['w']) + ' ' + str(div['h']) + '\n').encode())
    img.insert(2, ('# ' + str(testcase['selectable_times']) + '\n').encode())
    img.insert(3, ('# ' + str(choice_cost) + ' ' + str(repl_cost) + '\n').encode())

    with open(path, mode='wb') as f:
        f.writelines(img)


def shuffle_and_rotate_img(img, testcase):
    random.seed(testcase['random_state'])
    div = testcase['divide']
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
            direction = random.randint(0, 3)
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


def create_testdata(testcase):
    hash_val = hashlib.md5(toml.dumps(testcase).encode('utf-8')).hexdigest()
    out_path = DATA_DIR + '/' + hash_val

    if os.path.isdir(out_path):
        return
    os.mkdir(out_path)

    img_name = testcase['name']
    in_img_path = IMG_DIR + '/' + img_name + '.ppm'
    img = Image.open(in_img_path)

    div = testcase['divide']
    w_size = div['size'] * div['w']
    h_size = div['size'] * div['h']

    if testcase['mode'] == 'resize':
        img = img.resize((w_size, h_size))

    if testcase['mode'] == 'crop':
        img = img.crop((0, 0, w_size, h_size))

    shuffle_log, rotate_log = shuffle_and_rotate_img(img, testcase)

    out_img_path = out_path + '/prob.ppm'
    img.save(out_img_path)
    add_info_by_comment(out_img_path, testcase)

    ans_path = out_path + '/true_original_state.txt'
    with open(ans_path, mode='w') as f:
        f.writelines(map(str, rotate_log))
        f.write('\n')
        f.write('\n'.join(map(lambda x: ' '.join(x), shuffle_log)))
        f.write('\n')

    subprocess.run("python " + PROJECT_TOP_DIR + "/utility/image_divider.py " + out_path + '/prob.ppm', shell=True)


def clean_up(testcases):
    targets = ['original_state.txt', 'restoration_procedure.txt']
    for testcase in testcases.values():
        hash_val = hashlib.md5(toml.dumps(testcase).encode('utf-8')).hexdigest()
        out_path = DATA_DIR + '/' + hash_val

        for file_name in targets:
            if os.path.isfile(out_path+'/'+file_name):
                os.remove(out_path+'/'+file_name)


def exec_subprocess(process, env=""):
    env += "PROJECT_TOP_DIR=" + os.path.abspath(PROJECT_TOP_DIR) + "; "
    cmd = env + ' && '.join(process['run'])
    result = subprocess.run(cmd,
                            shell=True,
                            stdout=subprocess.DEVNULL if not process['stdout'] else None,
                            stderr=subprocess.DEVNULL if not process['stderr'] else None)
    if result.returncode != 0:
        raise ChildProcessError("preprocess failed")


def validate_original_state(prob_dir):
    true_ans_path = prob_dir+'/true_original_state.txt'
    with open(true_ans_path) as f:
        true_state = f.read()

    ans_path = prob_dir+'/original_state.txt'
    with open(ans_path) as f:
        state = f.read()

    if true_state != state:
        raise RuntimeError("wrong original_state: " + ans_path)


def exec_test(testcase, process, force):
    try:
        hash_val = hashlib.md5(toml.dumps(testcase).encode('utf-8')).hexdigest()
        prob_dir = DATA_DIR + '/' + hash_val
        env = "PROB_DIR=" + prob_dir + "; "
        exec_subprocess(process, env)
        if process['type'] == 'image restorer':
            validate_original_state(prob_dir)
    except Exception as e:
        if force:
            return False
        else:
            raise e
    return True


def append_log(testcases):
    pass


def requirement_test(args):
    testcase_path = args['TESTCASE_PATH']
    config_path = args['TEST_CONFIG_PATH']

    print('\ncheck config file: ' + config_path)
    config = safety(toml.load, safety(open, config_path))
    printG('passed', prefix='    -> ')

    print('\ncheck testcase file: ' + testcase_path)
    testcases = safety(toml.load, safety(open, testcase_path))
    printG('passed', prefix='    -> ')

    print('\nvalidate config')
    config = safety(validate_and_fill_config, config)
    printG('passed', prefix='    -> ')

    print('\nvalidate testcases')
    safety(validate_and_fill_testcases, testcases)
    printG('passed', prefix='    -> ')

    print('\ncreate testdata')
    for test_name, testcase in testcases.items():
        print(f'[{test_name}] ', end='')
        safety(create_testdata, testcase, print_prefix='')
        printG('done')

    print('\nclean up')
    safety(clean_up, testcases)
    printG('done', prefix='    -> ')

    if config['preprocess']:
        print('\nexecute preprocess')
    for preprocess in config['preprocess']:
        name = preprocess['name']
        if preprocess['stdout'] or preprocess['stderr']:
            print(f'[{name}] start')
        else:
            print(f'[{name}] ', end='', flush=True)
        prefix = f'[{name}] ' if preprocess['stdout'] or preprocess['stderr'] else ''
        safety(exec_subprocess, preprocess, print_prefix=prefix)
        printG('done', prefix=prefix)

    print('\nexecute test')
    failed = False
    try:
        process_count = 0
        pass_count = 0
        passed_list = set()
        failed_list = set()
        for test_name, testcase in testcases.items():
            print(f'[{test_name}] start')
            for process in config['test']:
                process_name = process['name']
                if process['stdout'] or process['stderr']:
                    print(f'[{test_name}]::[{process_name}] start')
                else:
                    print(f'[{test_name}]::[{process_name}] ', end='', flush=True)
                prefix = f'[{test_name}]::[{process_name}] ' if process['stdout'] or process['stderr'] else ''
                passed = safety(exec_test, testcase, process, args['--force'], print_prefix=prefix)
                process_count += 1
                if passed:
                    pass_count += 1
                    passed_list.add(test_name)
                    printG('done', prefix=prefix)
                else:
                    failed = True
                    failed_list.add(test_name)
                    printR('failed', prefix=prefix)
            printG('done\n', prefix=f'[{test_name}] ')
        if args['--force']:
            print('pass count: ' + str(pass_count) + '/' + str(process_count))
            printG(*passed_list, prefix='passed test name: ', sep=', ')
            printR(*failed_list, prefix='failed test name: ', sep=', ')
    except SystemExit as e:
        failed = True if e.code != 0 else False
        printR('stopped')
    else:
        printG('all test done')

    if config['postprocess']:
        print("\nexecute postprocess")
    for postprocess in config['postprocess']:
        name = postprocess['name']
        if postprocess['stdout'] or testcase['stderr']:
            print(f'[{name}] start')
        else:
            print(f'[{name}] ', end='', flush=True)
        safety(exec_subprocess, postprocess)
        printG('done', prefix=f'[{name}] ' if preprocess['stdout'] or preprocess['stderr'] else '')

    if not failed:
        print('\nclean up')
        safety(clean_up, testcases)
        printG('done', prefix='    -> ')

        printG('\ncomplete')
    else:
        printR('\nfailed')
        sys.exit(-1)


def performance_test(args):
    print('performance test')

    """
    a = toml.load("test.toml")
    print(a)
    PROJECT_TOP_DIR = os.path.abspath(os.path.dirname(sys.argv[1]) + os.path.pardir)
    hoge = "PROJECT_TOP_DIR=" + PROJECT_TOP_DIR + "; "

    cmd = hoge + 'if [ $? = 0 ]; then ' + '; else exit -1; fi; if [ $? = 0 ]; then '.join(a['preprocess'][0]['run']) + '; else exit -1; fi;'
    print(cmd)
    print("executing...")
    result = subprocess.run(cmd, shell=True)
    print(result.returncode)
    """


