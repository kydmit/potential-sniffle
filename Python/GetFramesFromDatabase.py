import os
import cv2
from tqdm import tqdm

img_dir = "./PETS09-S2L1/img1"
database_name = "PETS09-S2L1-groundtruth"
dir_name = os.getcwd()
ground_truth_line = open(os.path.join(dir_name, "gt/gt" + ".txt"), 'r').read().splitlines()
#frame_old = -1
#cap = cv2.VideoCapture(video_file)

# for i in range(6):
#     return_value, frame = cap.read()

size = cv2.imread(os.path.join(img_dir, "000001.jpg"))
height0, width0, channels = size.shape

for line in tqdm(range(len(ground_truth_line))):
    all_params = ground_truth_line[line].split(",")
    frame_num, id_obj, left, top, width, height, b, c, d, e = map(float, all_params)
    frame_num, id_obj, left, top, width, height, b, c, d, e = map(int, [frame_num, id_obj, left, top, width, height, b, c, d, e])

    right = left + width
    bottom = top + height

    if left < 0:
        x3 = 0
    if right > width0:
        x4 = width0
    if top < 0:
        y3 = 0
    if bottom > height0:
        y4 = height0

    frame_1 = f"{frame_num:06}.jpg"
    frame = cv2.imread(os.path.join(img_dir, frame_1))

    if frame_num % 18 == 0:
        continue

    # отрисовка
    cv2.rectangle(frame, (left, top), (right, bottom), (0, 255, 0), 2)
    cv2.waitKey(10)
    cv2.imshow("window_name", frame)

    # создаем паки
    image_name = f"{id_obj}_{frame_num}.jpg"

    dirname = os.path.join(dir_name, database_name, str(id_obj))

    if not os.path.exists(dirname):
        os.makedirs(dirname, exist_ok=True)

    image_name = f"{id_obj}_{frame_num}.jpg"
    dirname = os.path.join(dir_name, database_name, str(id_obj))

    # нарезка
    crop_img = frame[top:bottom, left:right]
    # if crop_img.empty():
    # pass
    try:
        cv2.imwrite(os.path.join(dirname, image_name), crop_img, [cv2.IMWRITE_JPEG_QUALITY, 85])
    except Exception:
        pass
