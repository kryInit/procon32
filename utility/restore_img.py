import sys
from PIL import Image

data_path = sys.argv[1]
img_path = data_path + '/prob.ppm'
config_path = data_path + '/prob.txt'
ans_path = data_path + '/original_state.txt'

img = Image.open(img_path)

with open(config_path) as f:
    div_w, div_h = map(int, f.readline().split())

div_size = img.size[0] // div_w

split_imgs = []
for y in range(div_h):
    for x in range(div_w):
        split_imgs.append(img.crop((x*div_size, y*div_size, (x+1)*div_size, (y+1)*div_size)))

with open(ans_path) as f:
    rotations = f.readline().splitlines()[0]
    for i in range(len(rotations)):
        split_imgs[i] = split_imgs[i].rotate(360-int(rotations[i])*90)

    orig_idxes = f.readlines()
    for y, i in enumerate(orig_idxes):
        for x, j in enumerate(i.split()):
            idx = int(j[0], base=16) + int(j[1], base=16)*div_w
            img.paste(split_imgs[idx], (x*div_size, y*div_size))

img.save('./dump.jpg')

