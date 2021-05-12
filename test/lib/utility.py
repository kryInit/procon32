import sys


def safety(func, *args, print_prefix='    -> ', **kwargs):
    try:
        return func(*args, **kwargs)
    except KeyboardInterrupt as e:
        printR(f'[{e.__class__.__name__}]', prefix=print_prefix)
        sys.exit(0)
    except Exception as e:
        printR(f'[{e.__class__.__name__}]: {e}', prefix=print_prefix)
        sys.exit(-1)


def printR(*args, prefix='', **kwargs):
    print(prefix, end='')
    if sys.stdout.isatty():
        print('\033[31m', end='')
    print(*args, **kwargs)
    if sys.stdout.isatty():
        print('\033[0m', end='', flush=True)


def printG(*args, prefix='', **kwargs):
    print(prefix, end='')
    if sys.stdout.isatty():
        print('\033[32m', end='')
    print(*args, **kwargs)
    if sys.stdout.isatty():
        print('\033[0m', end='', flush=True)

