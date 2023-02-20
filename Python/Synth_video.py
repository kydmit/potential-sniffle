import pickle
from tkinter import *
import cv2
from PIL import ImageTk, Image, ImageGrab
import tkinter as tk
import tkinter.filedialog as fd
import random
import subprocess
import numpy as np
import os
import matplotlib.pyplot as plt
from matplotlib import cm
import datetime
import pathlib
import time
import threading

root = tk.Tk()
root.title("Synth_Video")

line_coords = []
area_coords = []
resized_objs_cv2 = []
resized_objs_coords = []

x_current_zone = 0
y_current_zone = 0

mode = IntVar() #текущий режим
type_of_animation = IntVar() #текущий режим анимации
steps = 50 #число шагов для перспективы
bgr_file_extension = "" #расширение объекта фона
video_duration = 0 #длительность всего видео в миллисекундах
out_video = 0 #объект для записи выходного видео
flag_moving = False

time_video = 0 #время на видео
time_start = 0 #время появления объекта
time_stop = 0 #время пропадания объекта
current_time = 0
start_animation_time = 0
t = 0


def add_bgr(): #функция выбора фонового изображения
    global bgr1_pth, bgr1_cv2, bgr_file_extension, video_duration

    # задаем допустимые расширения и указываем путь до нового фона
    filetypes = (("Изображение/видео", "*.png *.jpg *.avi *.mp4 *.mkv"), ("Любой", "*"))
    bgr1_pth = fd.askopenfilename(title="Открыть файл", filetypes=filetypes)
    if bgr1_pth == "":
        return

    #выделяем расширение из пути
    bgr_file_extension = pathlib.Path(bgr1_pth).suffix

    #если выбрали картинку:
    if bgr_file_extension == ".png" or bgr_file_extension == ".jpg":
        # новый фон в cv2
        bgr1_cv2 = cv2.imread(bgr1_pth, cv2.IMREAD_UNCHANGED)

    # если выбрали видео:
    if bgr_file_extension == ".avi" or bgr_file_extension == ".mp4" or bgr_file_extension == ".mkv":
        #открываем видео
        cap = cv2.VideoCapture(bgr1_pth)

        #получаем фпс и общее число кадров видео
        fps = cap.get(cv2.CAP_PROP_FPS)
        frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

        #считаем общую длительность видео в миллисекундах
        video_duration = (frame_count / fps) * 1000

        #берем кадр
        frame = cap.read()

        #первый индекс потому что read возвращает кортеж и кадр там это первый индекс
        bgr1_cv2 = frame[1].copy()

    #ресайзим канвас под новый фон
    canvas.configure(width=bgr1_cv2.shape[1], height=bgr1_cv2.shape[0])

    # обновляем канвас
    update_frame_on_canvas()


def add_obj(): #функция выбора объекта
    global obj1_pth, obj1_cv2

    # задаем допустимые расширения и указываем путь до нового фона
    filetypes = (("Изображение", "*.png *jpg"), ("Любой", "*"))
    obj1_pth = fd.askopenfilename(title="Открыть файл", filetypes=filetypes)
    if obj1_pth == "":
        return

    # новый объект в cv2
    obj1_cv2 = cv2.imread(obj1_pth, cv2.IMREAD_UNCHANGED)

    # обновляем канвас
    update_frame_on_canvas()


def save_json():
    data_line = {}
    data_area = {}

    if len(line_coords) > 0:
        for k in range(len(line_coords)):
            data_line[f"line_{k}"] = {
                "object_path": obj1_pth,
                "start_x": line_coords[k][0],
                "stop_x": line_coords[k][1],
                "start_y": line_coords[k][2],
                "stop_y": line_coords[k][3],
                "start_time": line_coords[k][4],
                "stop_time": line_coords[k][5]
            }

    if len(area_coords) > 0:
        for k in range(len(area_coords)):
            data_area[f"area_{k}"] = {
                "object_path": obj1_pth,
                "start_x": area_coords[k][0],
                "stop_x": area_coords[k][1],
                "start_y": area_coords[k][2],
                "stop_y": area_coords[k][3],
            }

    # Store data (serialize)
    with open(f'{os.path.basename(str(obj1_pil))[:-4]}.json', "wb") as outfile:
        if len(data_line) > 0:
            pickle.dump(data_line, outfile, protocol=pickle.HIGHEST_PROTOCOL)
        if len(data_area) > 0:
            pickle.dump(data_area, outfile, protocol=pickle.HIGHEST_PROTOCOL)

    # Load data (deserialize)
    # with open(f'{os.path.basename(str(obj2))[:-4]}.json', 'rb') as handle:
    #     unserialized_data = pickle.load(handle)

    #print(data_line)
    #print(data_area)
    print("База сохранена")


def mouse_down(eventorigin):
    global x, y

    # запоминаем абсолютные координаты мышки при нажатии ЛКМ
    x = eventorigin.x / canvas.winfo_width()
    y = eventorigin.y / canvas.winfo_height()


def mouse_up(eventorigin):
    # режим линий
    if mode.get() == 0:
        # добавляем нарисованную линию в список линий
        line_coords.append((x, y, eventorigin.x / canvas.winfo_width(), eventorigin.y / canvas.winfo_height(),
                            float(ent_timestart.get()), float(ent_timestop.get())))
    # режим области
    elif mode.get() == 1:
        # добавляем нарисованный рект в список ректов
        area_coords.append((x, y, eventorigin.x / canvas.winfo_width(), eventorigin.y / canvas.winfo_height()))

    print("line_coords:", line_coords)
    print("area_coords:", area_coords)

    # обновляем канвас
    update_frame_on_canvas()


def delete_lines(eventorigin):
    # чистим списки с векторами и зонами
    line_coords.clear()
    area_coords.clear()

    # обновляем канвас
    update_frame_on_canvas()

    print("Clear")


def resize_window(eventorigin):
    #функция через функцию, так как нельзя биндить функции на без аргумента eventorigin
    update_frame_on_canvas()


def update_frame_on_canvas():
    global bgr1_pil, obj1_cv2, out_video

    # чистим канвас
    canvas.delete('all')

    # считываем текущие размеры канваса
    width = canvas.winfo_width()
    height = canvas.winfo_height()

    # копируем текущий фон и дальнейшние действия делаем на копии
    img_upd = bgr1_cv2.copy()

    width2 = img_upd.shape[1]
    height2 = img_upd.shape[0]

    #не отображаем линии и прямоугольники если объекты движутся
    if not flag_moving:
        if mode.get() == 0: # рисуем линии в cv2
            for i in range(len(line_coords)):
                cv2.line(img_upd, (int(line_coords[i][0] * width2), int(line_coords[i][1] * height2)),
                         (int(line_coords[i][2] * width2), int(line_coords[i][3] * height2)), (0, 0, 0), thickness=2)

        if mode.get() == 1: # рисуем прямоугольники в cv2
            for i in range(len(area_coords)):
                cv2.rectangle(img_upd, (int(area_coords[i][0] * width), int(area_coords[i][1] * height)),
                         (int(area_coords[i][2] * width), int(area_coords[i][3] * height)), (0, 0, 0), thickness=2)

    #если объекты движутся
    if flag_moving:
        # если движение по линиям:
        if mode.get() == 0:
            #по всем объектам:
            for k in range(len(resized_objs_cv2)):
                time_start = line_coords[k][4]
                time_stop = line_coords[k][5]
                #если время появления объекта попадает в интервал то добавляем его
                if current_time > time_start and current_time < time_stop:
                    #перепишем текущий объект для удобства:
                    curr_obj = resized_objs_cv2[k]

                    # задаем координаты для добавления туда объекта
                    x, y = int(resized_objs_coords[k][0] - curr_obj.shape[1] / 2), \
                           int(resized_objs_coords[k][1] - curr_obj.shape[0] / 2)

                    #проверяем выход за границы фона
                    x, y, curr_obj, x_cut, y_cut = check_borders(x, y, curr_obj, img_upd)

                    # всталяем объект в фон на новом кадре по координатам х, у
                    img_upd = PlaceWithAlpha(curr_obj, img_upd, y, x)

        # если движение по зоне:
        if mode.get() == 1:
            # если время появления объекта попадает в интервал то добавляем его
            #if time_video / 1000 > time_start and time_video / 1000 < time_stop:
            # перепишем объект для удобства:
            curr_obj = obj1_cv2.copy()

            # задаем координаты центра для добавления туда объекта
            x, y = int(x_current_zone - curr_obj.shape[1] / 2), int(y_current_zone - curr_obj.shape[0] / 2)

            # проверяем выход за границы фона
            x, y, curr_obj, x_cut, y_cut = check_borders(x, y, curr_obj, img_upd)

            # всталяем объект в фон на новом кадре по координатам х, у
            img_upd = PlaceWithAlpha(curr_obj, img_upd, y, x)

    else: #для отображения без движения
        # всталяем объект в фон на новом кадре по центру
        img_upd = PlaceWithAlpha(obj1_cv2, img_upd, int(bgr1_cv2.shape[0] / 2 - obj1_cv2.shape[0] / 2),
                                                    int(bgr1_cv2.shape[1] / 2 - obj1_cv2.shape[1] / 2))

    #записываем кадр в видео
    if flag_moving:
        #cv2.imshow('write', img_upd)
        out_video.write(img_upd)

    # ресайзим cv2 Image под размер окна
    img_upd = cv2.resize(img_upd, (width, height), interpolation=cv2.INTER_NEAREST)

    # штука для цветности
    img_upd = np.flip(img_upd, -1)
    img_upd = Image.fromarray((img_upd).astype(np.uint8))

    #делаем PhotoImage из Image
    bgr1_pil = ImageTk.PhotoImage(image=img_upd)

    # отображаем
    canvas.create_image(bgr1_pil.width() / 2, bgr1_pil.height() / 2, image=bgr1_pil)


def select_frame_from_video(time_video):
    global bgr1_cv2

    #открываем видео
    cap = cv2.VideoCapture(bgr1_pth)

    #берем текущее время со старта анимации в мс
    time_video *= 1000

    #если текущее время больше чем длительность видео, делим на длительность и берем остаток, то есть запускаем видео заново
    if time_video >= video_duration:
        time_video = time_video % video_duration

    #выбираем кадр который соответствует этому времени
    cap.set(cv2.CAP_PROP_POS_MSEC, time_video)

    #берем кадр
    frame = cap.read()

    #первый индекс потому что read возвращает кортеж и кадр там это первый индекс
    bgr1_cv2 = frame[1].copy()


def line_moving(pers_one_step, y_one_step, pers_entered_back, frame_width, frame_height):
    global resized_objs_cv2, resized_objs_coords, current_time

    # размечаем в списках место для хранения изображений и координат
    resized_objs_cv2 = [None] * len(line_coords)
    resized_objs_coords = [None] * len(line_coords)

    #высчитываем текущее время со старта аниации
    current_time = time.time() - start_animation_time

    if flag_moving:
        for k in range(len(line_coords)):
            # переписываем в переменные данные линий
            start_x = line_coords[k][0] * frame_width
            start_y = line_coords[k][1] * frame_height
            stop_x = line_coords[k][2] * frame_width
            stop_y = line_coords[k][3] * frame_height
            time_start = line_coords[k][4]
            time_stop = line_coords[k][5]

            #считаем скорости по осям
            speed_x = (stop_x - start_x) / (time_stop - time_start)
            speed_y = (stop_y - start_y) / (time_stop - time_start)

            # считаем текущие x,y в зависимости от стартовой точки, текущего шага и скорости
            current_x = start_x + speed_x * current_time
            current_y = start_y + speed_y * current_time

            # граничные условия, если по одной координате уже доехали до конца, а по другой еще нет
            if start_x < stop_x and current_x > stop_x:
                current_x = stop_x

            if start_y < stop_y and current_y > stop_y:
                current_y = stop_y

            if start_x > stop_x and current_x < stop_x:
                current_x = stop_x

            if start_y > stop_y and current_y < stop_y:
                current_y = stop_y

            # вычисляем текущий шаг по Y
            cur_step = current_y / y_one_step

            # считаем перспективу для текущего шага
            cur_pers = pers_entered_back + cur_step * pers_one_step

            # считаем размеры для текущего кадра и ресайзим его
            new_width = int(obj1_cv2.shape[1] * cur_pers)
            new_height = int(obj1_cv2.shape[0] * cur_pers)
            new_size = (new_width, new_height)

            obj_resized = cv2.resize(obj1_cv2, new_size, cv2.INTER_NEAREST)

            # записываем заресайзенное изображение и координаты в список по индексу, соответствующему порядковому
            # номеру этой линии в data_for_lines
            resized_objs_cv2[k] = obj_resized
            resized_objs_coords[k] = [current_x, current_y]

        #шаги для разных типов анимации
        # if type_of_animation.get() == 0:
        #     curr_step = i
        # elif type_of_animation.get() == 1:
        #     curr_step = steps - i
        # elif type_of_animation.get() == 2:
        #     #тк тут едем туда и обратно, сначала шаг передается от 0 до steps -1, потом steps + (от 0 до steps -1)
        #     if curr_step < steps - 1:
        #         curr_step = i
        #     else:
        #         curr_step = steps + (steps - i)

        if bgr_file_extension == ".avi" or bgr_file_extension == ".mp4" or bgr_file_extension == ".mkv":
            #берем текущее время это время на видео и отдаем в функцию выбора кадра видео
            select_frame_from_video(current_time)

        update_frame_on_canvas()

        cv2.waitKey(10)

        root.update()


def start_mov():
    global flag_moving, x_current_zone, y_current_zone, out_video, start_animation_time, current_time

    start_animation_time = time.time()
    current_time = 0

    flag_moving = True
    print('Moving...')

    frame_width = bgr1_cv2.shape[1]
    frame_height = bgr1_cv2.shape[0]

    #переменная с время-дата для названия выходного видео
    now = datetime.datetime.now()
    time_data = now.strftime("%H-%M-%d-%m-%Y")

    #кодек
    fourcc = cv2.VideoWriter_fourcc('M','J','P','G')
    #создаем объект VideoWriter для сохранения кадров
    out_video = cv2.VideoWriter(f'output_{time_data}.avi', fourcc, 5, (bgr1_cv2.shape[1], bgr1_cv2.shape[0]))

    # если режим-линии
    if mode.get() == 0:
        # считываем значение перспективы из окна
        pers_entered = float(ent_pers.get())

        # вычисляем перспективу для самой дальней и ближней точки и их дельту по всему Y
        pers_entered_back = 1 / pers_entered
        pers_entered_front = 1 * pers_entered
        pers_delta = pers_entered_front - pers_entered_back

        # считаем изменение перспективы и изменение Y за 1 шаг
        pers_one_step = pers_delta / steps
        y_one_step = frame_height / steps

        # вычисляем максимальное время пути из всех линий
        time_stop_max = 0
        for h in line_coords:
            if time_stop_max < h[5]:
                time_stop_max = h[5]

        # в зависимости от выбранного типа анимации управляем шагом
        if type_of_animation.get() == 0:
            while current_time < time_stop_max:
                line_moving(pers_one_step, y_one_step, pers_entered_back, frame_width, frame_height)

        elif type_of_animation.get() == 1:
            for i in reversed(range(steps)):
                line_moving(i, pers_one_step, y_one_step, pers_entered_back, frame_width, frame_height)
        elif type_of_animation.get() == 2:
            for i in range(steps):
                line_moving(i, pers_one_step, y_one_step, pers_entered_back)
            for i in reversed(range(steps)):
                line_moving(i, pers_one_step, y_one_step, pers_entered_back)

        #data_for_lines.clear()

    elif mode.get() == 1:

        start_x = area_coords[0][0] * frame_width
        start_y = area_coords[0][1] * frame_height
        stop_x = area_coords[0][2] * frame_width
        stop_y = area_coords[0][3] * frame_height
        time_stop = float(ent_timestop.get())

        center_x = (stop_x + start_x) / 2
        center_y = (stop_y + start_y) / 2

        if start_x < stop_x:
            x_step = abs(stop_x - center_x) / steps
        else:
            x_step = -(abs(stop_x - center_x) / steps)

        if start_y < stop_y:
            y_step = abs(stop_y - center_y) / steps
        else:
            y_step = -(abs(stop_y - center_y) / steps)

        direction = random.randint(1, 4)

        # выбираем начальный dx dy
        if direction == 1:  # +/+
            pass
        elif direction == 2:  # -/-
            x_step = -x_step
            y_step = -y_step
        elif direction == 3:  # +/-
            y_step = -y_step
        elif direction == 4:  # -/+
            x_step = -x_step

        x_current_zone = random.randint(int(start_x), int(stop_x))
        y_current_zone = random.randint(int(start_y), int(stop_y))

        i = 0
        while flag_moving:
            current_time = time.time() - start_animation_time

            #добавляем к текущим координатам 1 шаг
            x_current_zone += x_step
            y_current_zone += y_step

            #если фон-видео отправляем на выбор кадра для фона
            if bgr_file_extension == ".avi" or bgr_file_extension == ".mp4" or bgr_file_extension == ".mkv":
                select_frame_from_video(current_time)

            update_frame_on_canvas()

            cv2.waitKey(20)

            root.update()

            #если доехали до границы фона по какой-то оси, инвертируем ее
            if x_current_zone >= stop_x or x_current_zone <= start_x:
                x_step = -x_step

            if y_current_zone >= stop_y or y_current_zone <= start_y:
                y_step = -y_step

            i += 1
            if current_time > time_stop:
                break

    # переключаем флаг
    flag_moving = False

    out_video.release()

    print('Finish')


def check_borders(x, y, curr_obj, img_upd):
    x_cut = 0
    y_cut = 0
    # проверка на выходы за границы:
    # выход по х в +:
    if x + curr_obj.shape[1] >= img_upd.shape[1]:
        # вычисляем сколько отрезать в случае выхода
        x_cut = (x + curr_obj.shape[1]) - img_upd.shape[1]

    # выход по у в +:
    if y + curr_obj.shape[0] >= img_upd.shape[0]:
        y_cut = (y + curr_obj.shape[0]) - img_upd.shape[0]

    # выход по х в -:
    if x <= 0:
        x_cut = x
        x = 0

    # выход по у в -:
    if y <= 0:
        y_cut = y
        y = 0

    # для случаев захода за границы x+ y- режем так:
    if x_cut > 0 and y_cut < 0:
        curr_obj = curr_obj[0 - y_cut:curr_obj.shape[0], 0:curr_obj.shape[1] - x_cut]
    # для случаев захода за границы x- y+ режем так:
    elif x_cut < 0 and y_cut > 0:
        curr_obj = curr_obj[0:curr_obj.shape[0] - y_cut, 0 - x_cut:curr_obj.shape[1]]
    # для случаев захода за границы в + режем так:
    elif x_cut > 0 or y_cut > 0:
        curr_obj = curr_obj[0:curr_obj.shape[0] - y_cut, 0:curr_obj.shape[1] - x_cut]
    # для случаев захода за границы в - режем так:
    elif x_cut < 0 or y_cut < 0:
        curr_obj = curr_obj[0 - y_cut:curr_obj.shape[0], 0 - x_cut:curr_obj.shape[1]]

    return x, y, curr_obj, x_cut, y_cut


def stop_mov():
    global t
    #flag_moving = False
    t.stop()
    print("Stopped")


def change_mode_line():
    global mode
    mode.set(0)
    print('Line mode')


def change_mode_area():
    global mode
    mode.set(1)
    print('Area mode')


def change_type_straight():
    global type_of_animation
    type_of_animation.set(0)
    print('Type of animation: Straight')


def change_type_reverse():
    global type_of_animation
    type_of_animation.set(1)
    print('Type of animation: Reverse')


def change_type_ping_pong():
    global type_of_animation
    type_of_animation.set(2)
    print('Type of animation: Ping-Pong')


def PlaceWithAlpha(foreground1, background, x, y):
    #берем маску по объекту
    alpha1 = foreground1[:, :, 3].copy()

    #какая-то черно-белая версия фона
    alpha = background[:, :, 2].copy()

    #красит фон полностью в черный
    alpha.fill(0)

    #вставляем в черный фон маску объекта
    alpha[x: x + int(alpha1.shape[0]), y: y + int(alpha1.shape[1])] = alpha1

    #делаем новое пустое черное окно с размерами фона
    alpha3 = np.zeros((np.array(background).shape[0], np.array(background).shape[1], 3))

    #берется по очереди каждый канал, зачем-неизвестно, на выходе та же маска но без этого не работет мультиплай
    alpha3[:, :, 0] = alpha  # same value in each channel
    alpha3[:, :, 1] = alpha
    alpha3[:, :, 2] = alpha
    alpha = alpha3  #трехканальный фон

    # делаем новое пустое черное окно с размерами объекта
    foreground = np.zeros((np.array(foreground1).shape[0], np.array(foreground1).shape[1], 3))

    # берется по очереди каждый канал, зачем-неизвестно, на выходе та же маска но без этого не работет мультиплай
    foreground[:, :, 0] = foreground1[:, :, 0]  # same value in each channel
    foreground[:, :, 1] = foreground1[:, :, 1]
    foreground[:, :, 2] = foreground1[:, :, 2]

    # делаем новое пустое черное окно с размерами фона
    foreground3 = np.zeros((np.array(background).shape[0], np.array(background).shape[1], 3))
    #foreground3.fill(255)

    # вставляем маску объекта в черное окно размером фона
    foreground3[x: x + int(foreground.shape[0]), y: y + int(foreground.shape[1])] = foreground

    foreground = foreground3 #трехканальный объект

    # Convert uint8 to float
    foreground = foreground.astype(float)
    background = background.astype(float)

    # Normalize the alpha mask to keep intensity between 0 and 1
    alpha = alpha.astype(float) / 255

    # Multiply the background with ( 1 - alpha )
    background = cv2.multiply(1.0 - alpha, background)

    # Add the masked foreground and background.
    background = cv2.add(foreground, background) / 255

    #умножаем обратно на 255 чтобы получить целые числа значений пикселей
    background = (background * 255).astype(np.uint8)

    return background

#инициализируем первый верхний фрейм для лэйблов
frame1 = Frame(root)

# инициализация надписей и строк для ввода параметров на главном окне root
lbl_timestart = LabelFrame(text="Time start:")
lbl_timestart.pack(anchor=NW)
ent_timestart = Entry(lbl_timestart, width=10)
ent_timestart.insert(END, '0')
ent_timestart.pack()

lbl_timestop = LabelFrame(text="Time stop:")
lbl_timestop.pack(anchor=NW)
ent_timestop = Entry(lbl_timestop, width=10)
ent_timestop.insert(END, '10')
ent_timestop.pack()

lbl_pers = LabelFrame(text="Pers:")
lbl_pers.pack(anchor=NW)
ent_pers = Entry(lbl_pers, width=10)
ent_pers.insert(END, '1.5')
ent_pers.pack()

#пакуем первый верхний фрейм
frame1.pack(side=TOP)

#инициализируем второй нижний фрейм для канваса
frame2 = Frame(root)

# фон по умолчанию
bgr1_pth = "./bgr1.jpg"
# выделяем расширение из пути
bgr_file_extension = pathlib.Path(bgr1_pth).suffix
# фон в cv2
bgr1_cv2 = cv2.imread(bgr1_pth, cv2.IMREAD_COLOR)
# фон в pil для отображения на канвасе
bgr1 = Image.open(bgr1_pth)
bgr1_pil = ImageTk.PhotoImage(image=bgr1)


# объект по умолчанию
obj1_pth = "./obj2.png"
# объект в cv2
obj1_cv2 = cv2.imread(obj1_pth, cv2.IMREAD_UNCHANGED)
# объект в pil для отображения на канвасе
obj1 = Image.open(obj1_pth)
obj1_pil = ImageTk.PhotoImage(image=obj1)


#размещаем по умолчанию объект на фоне по центру
background = PlaceWithAlpha(obj1_cv2, bgr1_cv2, int(bgr1_cv2.shape[0] / 2 - obj1_cv2.shape[0] / 2),
                                                int(bgr1_cv2.shape[1] / 2 - obj1_cv2.shape[1] / 2))

# штука для цветности
background = np.flip(background, -1)
bgr1_pil11 = Image.fromarray((background).astype(np.uint8))
bgr1_pil = ImageTk.PhotoImage(image=bgr1_pil11)

# инициализация канваса с размерами фонового изображения
canvas = Canvas(frame2, width=bgr1_pil.width(), height=bgr1_pil.height())
canvas.pack(expand=True, fill=BOTH, side=TOP)

#пакуем второй нижний фрейм
frame2.pack(side=TOP)

# отображаем на канвасе по умолчанию
canvas.create_image(bgr1_pil.width() / 2, bgr1_pil.height() / 2, image=bgr1_pil)

# действия при нажатии на канвас
canvas.bind('<Button-3>', delete_lines)
canvas.bind('<Button-1>', mouse_down)
canvas.bind('<ButtonRelease-1>', mouse_up)
canvas.bind('<Configure>', resize_window)

m = Menu(root)  # создается объект Меню на главном окне
root.config(menu=m)  # окно конфигурируется с указанием меню для него

m.add_cascade(label="Add background", command=add_bgr)
m.add_cascade(label="Add object", command=add_obj)
m.add_cascade(label="Start moving", command=start_mov)
#m.add_cascade(label="Stop", command=stop_mov)
m.add_cascade(label="Save", command=save_json)

sm = Menu(m)  # создается пункт меню с размещением на основном меню (m)
m.add_cascade(label="Mode", menu=sm)  # пункту располагается на основном меню (m)
sm.add_radiobutton(label='Line', value=0, variable=mode, command=change_mode_line)
sm.add_radiobutton(label='Area', value=1, variable=mode, command=change_mode_area)

gm = Menu(m)
m.add_cascade(label="Type of animation", menu=gm)  # пункту располагается на основном меню (m)
gm.add_radiobutton(label='Straight', value=0, variable=type_of_animation, command=change_type_straight)
#gm.add_radiobutton(label='Reverse', value=1, variable=type_of_animation, command=change_type_reverse)
#gm.add_radiobutton(label='Ping-Pong', value=2, variable=type_of_animation, command=change_type_ping_pong)


root.mainloop()

# переделать все на время, пример: пользователь ввел что обьект появляется на 5 секунде, и после этого вся его анимация занимает 15 секунд
# (движение от начала линии до конца)

# Выделение обьектов по координатам (кликнули выделился обьект) его можно удалить, или поменять параметры (time_start, speed)

# сохранять настройки все в файл