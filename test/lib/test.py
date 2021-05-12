import os
import sys
import toml
import random
import pprint
from .utility import *
from .constant import TEST_DIR, IMG_DIR, DATA_DIR

"""
class MyError(Exception):
    def __init__(self, message='Empty message'):
        self.message = message

    def __str__(self):
        return self.message
"""


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

    config = validate_and_fill_recursively(config, 'config', conditions)


def validate_and_fill_testcases(testcases):
    conditions = ({
        'name': (str(), None,
                 lambda n: f": image named {n} is not exists" if not os.path.exists(IMG_DIR+'/'+n+'.ppm') else None),
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


def create_testdata(testcase):
    pass


def clean_up(testcases):
    pass


def exec_preprocess(config):
    pass


def exec_tests(config, testcases):
    for test_name, testcase in testcases.items():
        print(f'[{test_name}] start')
        safety(exec_test, config, testcase, print_prefix='')
        printG('done', prefix=f'[{test_name}] ')


def exec_test(config, testcase):
    # raise ValueError("ve")
    pass


def exec_postprocess(config):
    pass


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
    safety(validate_and_fill_config, config)
    printG('passed', prefix='    -> ')

    print('\nvalidate testcases')
    safety(validate_and_fill_testcases, testcases)

    print('=============================')
    pprint.pprint(config)
    print('=============================')
    pprint.pprint(testcases)
    print('=============================')

    print('\ncreate testdata')
    for test_name, testcase in testcases.items():
        print(f'[{test_name}] ', end='')
        safety(create_testdata, testcase, print_prefix='')
        printG('done')

    print('\nclean up')
    safety(clean_up, testcases)
    printG('done', prefix='    -> ')

    print('\nexecute preprocess')
    safety(exec_preprocess, config)
    printG('done', prefix='    -> ')

    print('\nexecute test')
    try:
        safety(exec_tests, config, testcases)
    except SystemExit as e:
        failed = True if e.code != 0 else False
        printR('stopped')
    else:
        failed = False
        printG('all test done')

    print("\nexecute postprocess")
    safety(exec_postprocess, config)
    printG('done', prefix='    -> ')

    print('\nclean up')
    safety(clean_up, testcases)
    printG('done', prefix='    -> ')

    # print('\nappend log')
    # safety(append_log, testcases)
    # printG('done', prefix='    -> ')

    if failed:
        printR('\nfailed')
        sys.exit(-1)
    else:
        printG('\ncomplete')

    """
    try:
        for test_name, testcase in testcases.items():
            fill_testcase(testcase)
            testcases[test_name] = testcase
            # create_testcase_data(testcases)
            # clean_up_testcase_dir(testcases)
    except MyError as e:
        printR(e)
    """


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


