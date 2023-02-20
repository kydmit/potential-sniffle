from datetime import timedelta
import cv2
import numpy as np
import os
from tqdm import tqdm

# то есть, если видео длительностью 30 секунд, сохраняется 10 кадров в секунду = всего сохраняется 300 кадров
SAVING_FRAMES_PER_SECOND = 2

max_images = 10000
OBJECTS_list = []

def format_timedelta(td):
    """Служебная функция для классного форматирования объектов timedelta (например, 00:00:20.05)
    исключая микросекунды и сохраняя миллисекунды"""
    result = str(td)
    try:
        result, ms = result.split(".")
    except ValueError:
        return "-" + result + ".00".replace(":", "-")
    ms = int(ms)
    ms = round(ms / 1e4)
    return f"-{result}.{ms:02}".replace(":", "-")


def get_saving_frames_durations(cap, saving_fps):
    """Функция, которая возвращает список длительностей сохраняемых кадров"""
    s = []
    # получаем длительность клипа, разделив количество кадров на количество кадров в секунду
    clip_duration = cap.get(cv2.CAP_PROP_FRAME_COUNT) / cap.get(cv2.CAP_PROP_FPS)
    # используем np.arange() для выполнения шагов с плавающей запятой
    for i in np.arange(0, clip_duration, 1 / saving_fps):
        s.append(i)
    return s


def read_database(dir_name):
    # get all classes
    #global classes
    #classes = open(os.path.join(dir_name, "obj.names"), 'r').read().splitlines()

    # create classes folders
    global all_files_list
    all_files_list = open(os.path.join(dir_name, "train.txt"), 'r').read().splitlines()
    for i in tqdm(range(len(all_files_list))):
        #if i % max_images == 0:  # on 0 and every max_images
            database_name = os.path.basename(os.path.basename(dir_name) + f"_{i}")
            global database_folder
            database_folder = os.path.join(os.getcwd(), "DATA", database_name)
            global img_num
            img_num = -1
            create_class_folders(database_folder)


def create_class_folders(database_folder):
    if os.path.isdir(database_folder):
        # https://i.stack.imgur.com/9UVnC.png
        COLOR = '\n\033[97;101m'
        ENDC = '\033[0m\n'
        print(
            COLOR + f" folder {os.path.basename(database_folder)} already exists, please delete the folder so I can continue " + ENDC)
        input()

    for cl in all_files_list[0]:
        new_cl_folder = os.path.join(database_folder, cl)
        if not os.path.exists(database_folder):
            os.makedirs(new_cl_folder, exist_ok=True)


def main(video_file):
    filename, _ = os.path.splitext(video_file)
    filename += "-opencv"
    dir_name = os.getcwd()
    # создаем папку по названию видео файла
    if not os.path.isdir(filename):
        os.mkdir(filename)
    # читать файл с точками контуров
    read_database(dir_name)


    # читать видео файл
    cap = cv2.VideoCapture(video_file)
    # получить FPS видео
    fps = cap.get(cv2.CAP_PROP_FPS)
    # если наше SAVING_FRAMES_PER_SECOND больше FPS видео, то установливаем минимальное
    saving_frames_per_second = min(fps, SAVING_FRAMES_PER_SECOND)
    # получить список длительностей кадров для сохранения
    saving_frames_durations = get_saving_frames_durations(cap, saving_frames_per_second)
    # начало цикла
    count = 0
    save_count = 0
    while True:
        is_read, frame = cap.read()
        if not is_read:
            # выйти из цикла, если нет фреймов для чтения
            break
        # получаем длительность, разделив текущее количество кадров на FPS
        frame_duration = count / fps
        try:
            # получить самую первоначальную длительность для сохранения
            closest_duration = saving_frames_durations[0]
        except IndexError:
            # список пуст, все кадры сохранены
            break
        if frame_duration >= closest_duration:
            # если ближайшая длительность меньше или равна длительности текущего кадра,
            # сохраняем фрейм
            frame_duration_formatted = format_timedelta(timedelta(seconds=frame_duration))
            saveframe_name = os.path.join(filename, f"frame{frame_duration_formatted}.jpg")
            cv2.imwrite(saveframe_name, frame)
            save_count += 1
            print(f"{saveframe_name} сохранён")
            # удалить текущую длительность из списка, так как этот кадр уже сохранен
            try:
                saving_frames_durations.pop(0)
            except IndexError:
                pass
        # увеличить счечик кадров count
        count += 1

    print(f"Итого сохранено кадров {save_count}")


if __name__ == "__main__":
    import sys

    video_file = "video.avi" #sys.argv[1]
    import time

    begtime = time.perf_counter()
    main(video_file)
    endtime = time.perf_counter()
    print(f"\nЗатрачено, с: {endtime - begtime} ")