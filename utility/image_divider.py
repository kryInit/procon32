# バイナリを扱う際に改行コードの扱いに気を使いたくないのでバイナリと問題データに分ける

import os
import sys
import shutil
from PIL import Image

if len(sys.argv) < 2:
    print('image path required for argument')
    sys.exit(-1)

if not os.path.exists(sys.argv[1]):
    print(sys.argv[1] + ' is not exists.')
    sys.exit(-1)

dir_path = os.path.dirname(sys.argv[1])

in_img_path = sys.argv[1]
# out_img_path = dir_path + '/prob.ppm.bin'
out_img_dir = dir_path + '/frags'
out_txt_path = dir_path + '/prob.txt'

# ケースの変数を読み取り
with open(in_img_path, mode='rb') as f:
    def readline():
        return f.readline().decode('utf-8').split()

    _ = readline()
    _, div_w, div_h = readline()
    _, selectable_times = readline()
    _, choice_cost, repl_cost = readline()
    pix_w, pix_h = readline()

# 別ファイル(out_txt_path)に保存
with open(out_txt_path, mode='w') as f:
    data = [[div_w, div_h], [selectable_times], [choice_cost, repl_cost], [pix_w, pix_h]]
    f.write('\n'.join(map(lambda x: ' '.join(x), data)))

pix_w = int(pix_w)
pix_h = int(pix_h)
div_w = int(div_w)
div_h = int(div_h)

# 画像を分割して保存後最初の数行(ヘッダ)を削除
img = Image.open(in_img_path)
if not os.path.exists(out_img_dir):
    os.mkdir(out_img_dir)
frag_size = round(pix_w / div_w)
for y in range(div_h):
    for x in range(div_w):
        idx = '{:X}{:X}'.format(x, y)
        tmp_img = img.crop((x*frag_size, y*frag_size, (x+1)*frag_size, (y+1)*frag_size))
        out_img_path = out_img_dir + '/' + idx + '.ppm'
        tmp_img.save(out_img_path)
        shutil.copy(out_img_path, out_img_path + '.bin')
        with open(out_img_path, mode='rb') as f:
            data = f.readlines()

        with open(out_img_path + '.bin', mode='wb') as f:
            f.writelines(data[3:])
