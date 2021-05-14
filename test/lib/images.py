import os
from .utility import *
from PIL import Image
from .constant import PROJECT_TOP_DIR, TEST_DIR, IMG_DIR, DATA_DIR


def dump_image_list():
    sep = ', ' if sys.stdout.isatty() else '\n'
    print(sep.join(list(map((lambda filename: os.path.splitext(filename)[0]),
                            filter(lambda filename: filename[0] != '.', os.listdir(IMG_DIR))))))


def add_image(img_path, force):
    print(f'[{img_path}] ', end='', flush=True)

    # pathからファイル名のみを取得
    file_name = os.path.basename(img_path)

    # PIL.Imageで開く
    img = Image.open(img_path)

    # forceではなく、すでに存在している場合はraise
    out_path = IMG_DIR + '/' + os.path.splitext(file_name)[0] + ".ppm"
    if not force and os.path.exists(out_path):
        raise FileExistsError("this image with this name has already been registered.")

    # convert and save
    img.save(out_path)

    printG('registered', prefix='')


def add_images_recursively(dir_path, force):
    dir_path = os.path.normpath(dir_path)

    if not os.path.isdir(dir_path):
        raise NotADirectoryError(f"No such directory: {dir_path}")

    file_name_list = os.listdir(dir_path)
    for file_name in file_name_list:
        try:
            path = dir_path + '/' + file_name
            if os.path.isdir(path):
                add_images_recursively(path, force)
            else:
                safety(add_image, path, force, print_prefix='')
        except SystemExit:
            pass


def add_images(args):
    for img_path in args['IMG_PATH']:
        try:
            if args['--recursive']:
                safety(add_images_recursively, img_path, args['--force'], print_prefix='')
            else:
                safety(add_image, img_path, args['--force'], print_prefix='')
        except SystemExit:
            pass


def remove_image(img_name):
    path = f'{IMG_DIR}/{img_name}.ppm'
    if not os.path.isfile(path):
        raise FileNotFoundError(f"No such image: {img_name}")
    os.remove(path)


def remove_images(args):
    for img_name in args['IMG_NAME']:
        try:
            print(f'[{img_name}] ', end='', flush=True)
            safety(remove_image, img_name, print_prefix='')
            printG('removed', prefix='')
        except SystemExit:
            pass
