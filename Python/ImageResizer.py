import os
import shutil

import cv2
import sys
import time


dsize = (416, 416)

if len(sys.argv) <= 1:
    img_dir = "./"
    #img_dir = "D://Images//2e72656b63757e87_trucker_avi"
else:
    img_dir = sys.argv[1]

methods = ['NEAREST', 'LINEAR', 'CUBIC', 'AREA', 'LANCZOS4']
methods_num = ['0', '1', '2', '3', '4']
t_0 = 0
list_of_images = []
list_of_dirs = []


def calc_time(i, max_i):
    global t_0

    if i == 0:
        t_0 = time.perf_counter()

    t_cur = time.perf_counter() - t_0

    if i > 0:
        t_one_iter = t_cur / i
        t_total = max_i * t_one_iter

        t_remained = (t_total - t_cur) / 60

        if t_remained > 60:
            t_remained = f'{round((t_remained / 60), 2)} hr remained'

        elif t_remained < 1:
            t_remained = f'{round((t_remained * 60), 2)} sec remained'

        else:
            t_remained = f'{round(t_remained, 2)} min remained'

        return t_remained


def main():

    for f in os.listdir(img_dir):
        if os.path.splitext(f)[1].lower() in ('.jpg', '.png'):
            list_of_images.append(f)
        if os.path.splitext(f)[1].lower() in ('.dir'):
            list_of_dirs.append(f)

    max_i = len(list_of_images)

    for i in range(len(list_of_images)):
        print(calc_time(i, max_i))
        image_path = f'{img_dir}/{list_of_images[i]}'
        for j in range(len(methods_num)):

            pic_in = cv2.imread(image_path)
            pic_out = cv2.resize(pic_in, dsize, interpolation=int(methods_num[j]))

            dirname = f'{os.path.dirname(img_dir)}/{os.path.basename(img_dir)}_{methods[j]}'

            if not os.path.exists(dirname):
                os.makedirs(dirname, exist_ok=True)

            cv2.imwrite(f'{dirname}/{methods[j]}_{list_of_images[i]}', pic_out, [cv2.IMWRITE_JPEG_QUALITY, 100])

    for d in range(len(list_of_dirs)):
        if not os.path.exists(f'{dirname}/{list_of_dirs[d]}'):
            for g in range(len(methods)):
                shutil.copytree(f'{img_dir}/{list_of_dirs[d]}',
                                f'{os.path.dirname(img_dir)}/{os.path.basename(img_dir)}_{methods[g]}/{list_of_dirs[d]}')

main()
