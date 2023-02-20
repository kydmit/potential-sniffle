import json
import threading
import tkinter as tk
import tkinter.ttk as ttk
import tkinter.filedialog as fd
import tkinter.messagebox as messageb
from tkinter import Toplevel
from PIL import Image, ImageTk
import cv2
import yolov4 as yolo
import os
import psutil
import pickle
from random import choice

a = 0  # текущий кадр
list_of_frames = []  # кадры исходного видео
list_of_objs = []  # (кадры)[[id, x1, y1, x2, y2],[id, x1, y1, x2, y2]...]]
video_file = ''  # путь к исходному видео
color = ['#f6eb3d', '#e7942f', '#c52936', '#cb1f84', '#653172', '#112379', '#2ab3db', '#27a891', '#229456',
         '#bbd23c']  # список цветов для контуров
cfg = {"Window_size": "952x715+338+139", "Last_saved_path": "./", "Last_opened_video_file": "./",
       "Rects_path": "./"}  # настройки программы
selected_mode = False  # режим выбора и редактирования
NewId = -1  # новый ID который присваиваем размечаемому обьекту
selected_Id = -1  # ID который выбрали
object_paths = []  # траектории движения всех путей

have_unsaved_changes = False

photo = None  # ImageTk.PhotoImage для канваса
frame = None  # PIL.Image которое ресайзим
resized = None  # PIL.Image заресайзенное изображение


def open_video():
    global video_file, cfg
    filetypes = (("Видеофайл", "*.avi *mp4"),
                 ("Любой", "*"))
    video_file = cfg["Last_opened_video_file"]
    video_file = fd.askopenfilename(title="Открыть файл", initialdir=os.path.split(video_file)[0], filetypes=filetypes)
    if video_file == "":
        return

    # cfg["Last_opened_video_file"] = os.path.split(video_file)[0]
    cfg["Last_opened_video_file"] = video_file

    # open_video_thread()

    t = threading.Thread(target=open_video_thread())
    t.start()


def save_database():  # записываем полученные контура в json
    # Writing to sample.json
    with open(f'{os.path.basename(video_file)[:-4]}.save', "wb") as outfile:
        pickle.dump(list_of_objs, outfile)
        # json.dump(list_of_objs, outfile)

    set_status_bar("База сохранена")
    root.update()

    max_id = 0
    for g in range(len(list_of_objs)):  # вычисляем количество idшников в базе
        for h in range(len(list_of_objs[g])):
            if list_of_objs[g][h][0] > max_id:
                max_id = list_of_objs[g][h][0]

    if max_id > -1:
        NewId = max_id + 1

    have_unsaved_changes = False


def load_database(video_file):
    global list_of_objs, NewId

    file_name = f'{os.path.basename(video_file)[:-4]}.save'
    if os.path.isfile(file_name):  # читаем конфиг файл
        with open(file_name, 'rb') as infile:
            list_of_objs = pickle.load(infile)

    max_id = 0
    for g in range(len(list_of_objs)):  # вычисляем количество idшников в базе
        for h in range(len(list_of_objs[g])):
            if list_of_objs[g][h][0] > max_id:
                max_id = list_of_objs[g][h][0]

    if max_id > -1:
        NewId = max_id + 1


def open_video_thread():
    global photo, frame, resized, a, list_of_objs, video_file, color
    a = 0
    slider.set(0)
    list_of_frames.clear()

    if video_file:
        print(video_file)

    # Load from file
    load_database(video_file)

    cap = cv2.VideoCapture(video_file)  # capture an video file
    total_frames_in_video = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

    # проверяем сколько есть оперативной памяти
    avaleble_memory = int(round(psutil.virtual_memory()[1] / 1024 / 1024))
    print(avaleble_memory)

    resolution_and_mb = [(800, 416, 1.3), (700, 416, 1.1), (500, 416, 0.8), (416, 416, 0.6)]

    new_w = -1
    new_h = -1

    for i in range(len(resolution_and_mb)):
        if resolution_and_mb[i][2] * total_frames_in_video < avaleble_memory:
            new_w = resolution_and_mb[i][0]
            new_h = resolution_and_mb[i][1]
            if i == 3:
                messageb.showinfo(choice(["Купите еще памяти!", "Чем память то заняли?"]),
                                  f"Маловато памяти, разрешение будет {resolution_and_mb[i][1]}")
            break

    if new_w == -1:
        need_memory = resolution_and_mb[len(resolution_and_mb) - 1][2] * total_frames_in_video
        messageb.showwarning(choice(["Купите еще памяти!", "Чем память то заняли?"]),
                             f"Слишком мало памяти! Нада еще {round((need_memory - avaleble_memory) / 1024, 1)} Гига")
        return

    p_f = -1  # кадр который сейчас обрабатываем

    lable_slider.configure(text=str(a) + '/' + str(total_frames_in_video))
    slider.configure(to=total_frames_in_video)

    while True:
        return_value, frame_cv2 = cap.read()  # захват кадра
        if not return_value:
            if len(list_of_frames) < 1:
                messageb.showwarning("Чет пошло не так", choice(
                    ["Ошибка при чтении видео", "Ошибака при чтении видео, обратитесь к системному администратору"]))
                return
            print('Video has ended')
            break

        p_f += 1

        frame_cv2 = cv2.cvtColor(frame_cv2, cv2.COLOR_BGR2RGB)  # изменение цветности
        resized_frame_cv2 = cv2.resize(frame_cv2, (new_w, new_h))  # resize

        list_of_frames.append(resized_frame_cv2)  # добавляем кадр в список кадров

        if p_f == 0:
            update_frame_on_canvas()  # отобразили первый кадр

        if p_f % 5 == 0:
            root.update()
            set_status_bar('Идёт загрузка видеофайла: ' + str(p_f) + '/' + str(total_frames_in_video), p_f,
                           total_frames_in_video)

    # frame = Image.fromarray(list_of_frames[0])  # для отображения первого кадра
    set_status_bar("Загрузка завершена")

    avaleble_memory_after = psutil.virtual_memory()[1] / 1024 / 1024
    print("memory sped: ", (avaleble_memory - avaleble_memory_after) / total_frames_in_video)

    update_frame_on_canvas()

    if len(list_of_objs) < 1:
        run_yolo = messageb.askokcancel("Yolo", f"Разметить yolo?")
        if run_yolo:
            mark_yolo()

    calc_all_paths()


def change_scale_value(scale_value):
    global a, frame
    if len(list_of_frames) > 0:
        a = int(round(float(scale_value)))
        if (a < len(list_of_frames) - 1):
            lable_slider.configure(text=str(a) + '/' + str(len(list_of_frames) - 1))
            frame = list_of_frames[a]
            update_frame_on_canvas()


def resize_image(e):
    global resized
    if resized is not None:
        update_frame_on_canvas()


def update_frame_on_canvas():  #
    global frame, photo, resized, a, color

    canv.delete('all')

    if (len(list_of_frames) < 1) or (a > len(list_of_frames) - 1):
        return

    width = canv.winfo_width()
    height = canv.winfo_height()

    frame = Image.fromarray(list_of_frames[a])  # конвертируем cv2 image в pil
    resized = frame.resize((width, height), Image.ANTIALIAS)  # ресайзим под канвас
    photo = ImageTk.PhotoImage(resized)  # для отображения
    canv.create_image(0, 0, anchor='nw', image=photo)  # рисуем картинку на канвасе

    if len(list_of_objs) < 1:
        return

    current_frame_objs = list_of_objs[a]

    for i in range(len(current_frame_objs) - 1):  # рисуем все контура для текущего кадра
        id = current_frame_objs[i][0]
        r_color = color[id % len(color)]
        if id == -1 or selected_mode:
            r_color = '#808080'

        rect = canv.create_rectangle(int(current_frame_objs[i][1] * width),
                                     int(current_frame_objs[i][2] * height),
                                     int(current_frame_objs[i][3] * width),
                                     int(current_frame_objs[i][4] * height), outline=r_color, tags="objs", width=2)
        if selected_mode and id > -1 and id == selected_Id:  # выделяем выбранный контур
            r_color = color[id % len(color)]
            rect = canv.create_rectangle(int(current_frame_objs[i][1] * width),
                                         int(current_frame_objs[i][2] * height),
                                         int(current_frame_objs[i][3] * width),
                                         int(current_frame_objs[i][4] * height), outline=r_color, tags="objs", width=5)

        show_paths(id, a)
    # if selected_mode:
    # ресуем рамку вокруг символизирующюю включеный режим
    # rect = canv.create_rectangle(1, 1, width-1, height-1, outline=color[selected_Id % len(color)], tags="objs", width=5)


def prev(q):  # предыдущий кадр
    global a
    print(a)
    if a > 0:
        a -= 1
    slider.set(a)


def next(q):  # следующий кадр
    global a
    print(a)
    if a < len(list_of_frames) - 1:
        a += 1
    slider.set(a)


def yolo_process():
    # create modal window
    # modal_window = Toplevel(root)
    # modal_window.title("yolo process")
    # pbar = ttk.Progressbar(modal_window, orient='horizontal', length=300, maximum=len(list_of_frames),
    #                       mode="determinate")
    # label1 = tk.Label(modal_window)
    # modal_window.iconbitmap("icon.ico")
    # modal_window.transient(root)
    # modal_window.grab_set()
    # modal_window.focus_set()
    # root.update()
    set_status_bar("Загрузка Yolo...")
    root.update()
    # run YOLO
    yolo.init()
    set_status_bar("Выполняется Yolo...")
    list_of_objs.clear()
    frame_num = 0
    for fr in list_of_frames:
        list_of_objs.append(yolo.process_one_frame(fr))
        # pbar.configure(value=frame_num)
        # pbar.pack()
        frame_num += 1
        set_status_bar('Обработано кадров: ' + str(frame_num) + '/' + str(len(list_of_frames) - 1), frame_num,
                       len(list_of_frames) - 1)
        # label1.configure(text='Обработано кадров: ' + str(frame_num) + '/' + str(len(list_of_frames) - 1))
        # label1.pack()
        # modal_window.update()

        root.update()

    # modal_window.destroy()


def mark_yolo():  # размечаем йоло, если база существует, переспрашиваем
    global list_of_objs, video_file

    if os.path.isfile(f'{os.path.basename(video_file)[:-4]}.save'):
        ask_process = messageb.askokcancel("База уже существует", "Пересчитать еще раз?")
        if ask_process:
            yolo_process()
            save_database()
    else:
        yolo_process()
        save_database()


def on_close():  # при закрытии переспрашиваем о выходе и сохраняем конфиг программы
    if have_unsaved_changes:
        save = messageb.askyesno("Выход", "Сохранить изменения в базе?")
        if save:
            save_database()
        else:
            shure = messageb.askyesno("Выход", "Точно не сохранять?")
            if not shure:
                save_database()

        # close = messageb.askokcancel(choice(["Выход", "Пока пока", "Аливедерчи", "До свидания"]), "Выйти из программы?")
        # if close:
        cfg["Window_size"] = root.geometry()

    # сохраняем настройки
    with open("config.json", "w") as conf:
        json.dump(cfg, conf)
    root.destroy()


def set_status_bar(text, i=-1, max_i=-1):
    statusbar.configure("LabeledProgressbar", text=text)
    if i > -1 and max_i > -1:
        p_s.configure(value=(100 * i) / max_i)
    else:
        p_s.configure(value=0)

    # .configure(value=frame_num)


def export_imgs():  # нарезаем видео по базе контуров на картинки
    global list_of_frames, list_of_objs, frame

    dir_for_imgs = fd.askdirectory(title="Укажите путь сохранения картинок", initialdir=cfg["Last_saved_path"])

    if dir_for_imgs == "":
        return

    cfg["Last_saved_path"] = dir_for_imgs

    frame0 = Image.fromarray(list_of_frames[0])

    set_status_bar("Идёт экспорт объектов...")

    root.update()

    for i in range(len(list_of_frames) - 1):

        frame = list_of_frames[i]
        frame_loc = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        current_frame_objs = list_of_objs[i]

        for j in range(len(current_frame_objs) - 1):

            left = int(current_frame_objs[j][1] * frame0.width)
            top = int(current_frame_objs[j][2] * frame0.height)
            right = int(current_frame_objs[j][3] * frame0.width)
            bottom = int(current_frame_objs[j][4] * frame0.height)

            # отрисовка рестов и отображение
            # cv2.rectangle(frame_loc, (left, top), (right, bottom), (0, 255, 0), 2)
            # cv2.imshow("window_name", frame_loc)
            cv2.waitKey(1)

            dir_name = os.path.join(dir_for_imgs, f'{os.path.basename(video_file)[:-4]}_export_obj',
                                    str(current_frame_objs[j][0]))

            if not os.path.exists(dir_name):
                os.makedirs(dir_name, exist_ok=True)

            image_name = f"{current_frame_objs[j][0]}_{i}.jpg"

            # нарезка на картинки
            crop_img = frame_loc[top:bottom, left:right]

            try:
                cv2.imwrite(os.path.join(dir_name, image_name), crop_img, [cv2.IMWRITE_JPEG_QUALITY, 85])
            except Exception:
                pass

    set_status_bar("Экспорт объектов завершен")
    root.update()

    os.startfile(dir_for_imgs)


def export_rects():  # сохраняем базу контуров в txt
    global list_of_frames, list_of_objs

    dir_for_rects = fd.askdirectory(title="Укажите путь сохранения файла контуров", initialdir=cfg["Rects_path"])

    if dir_for_rects == "":
        return

    cfg["Rects_path"] = dir_for_rects

    frame = Image.fromarray(list_of_frames[0])

    with open(f'{dir_for_rects}/{os.path.basename(video_file)[:-4]}_export_rects.txt', 'w') as file:  # Открываем фаил
        for i in range(len(list_of_frames) - 1):
            current_frame_objs = list_of_objs[i]
            for j in range(len(current_frame_objs) - 1):
                left = current_frame_objs[j][1]
                top = current_frame_objs[j][2]
                right = current_frame_objs[j][3]
                bottom = current_frame_objs[j][4]

                file.write(f'{i} {j} {left} {top} {right} {bottom}\n')

    set_status_bar("Экспорт контуров завершен")
    root.update()

    os.startfile(dir_for_rects)


def rect_select(eventorigin):  # выбор контура по лкм
    global list_of_frames, list_of_objs, a, selected_mode, selected_Id, NewId, have_unsaved_changes

    if len(list_of_objs) < 1 or len(list_of_objs[a]) < 1:
        return

    x = round(eventorigin.x / canv.winfo_width(), 4)
    y = round(eventorigin.y / canv.winfo_height(), 4)
    # print(x, y)

    current_frame_objs = list_of_objs[a]
    find = False
    for i in range(len(list_of_objs[a]) - 1):
        cur_rec = current_frame_objs[i]
        id = cur_rec[0]
        left = cur_rec[1]
        top = cur_rec[2]
        right = cur_rec[3]
        bottom = cur_rec[4]

        if y >= top and y <= bottom:
            if x >= left and x <= right:
                find = True
                if not selected_mode:
                    # еще никого не выбрали
                    if id == -1:
                        NewId += 1
                        print(id)
                        id = NewId
                    selected_mode = True
                    selected_Id = id
                    cur_color = f"plus {color[selected_Id % len(color)]}"
                    canv.configure(cursor=cur_color)  # меняем цвет курсора
                else:
                    # стираем всех таких на всякий случай
                    for j in range(len(list_of_objs[a]) - 1):
                        if current_frame_objs[j][0] == selected_Id:
                            current_frame_objs[j] = (
                                -1, current_frame_objs[j][1], current_frame_objs[j][2], current_frame_objs[j][3],
                                current_frame_objs[j][4])
                            break
                have_unsaved_changes = True
                current_frame_objs[i] = (selected_Id, left, top, right, bottom)
                update_frame_on_canvas()
                break

    set_status_bar(f"Редактирование ID: {selected_Id}"
                   f", ESC - выйти")
    if not find:  # отменяем режим
        remove_select(1)

    root.update()


def remove_select(q):  # отмена выбора контура
    global selected_Id, selected_mode
    selected_Id = -1
    selected_mode = False

    set_status_bar("Выберите объект для редактирования")
    canv.configure(cursor="arrow")  # меняем цвет курсора
    root.update()

    update_frame_on_canvas()


def calc_all_paths():  # посчитать все траектории движения контуров
    return 1
    global list_of_objs
    max_id = 0

    object_paths.clear()

    for g in range(len(list_of_objs)):  # вычисляем количество idшников в базе
        for h in range(len(list_of_objs[g])):
            if list_of_objs[g][h][0] > max_id:
                max_id = list_of_objs[g][h][0]

    if max_id < 2:
        return

    for k in range(max_id):  # добавляем в список количество пустых списков равно количеству id в базе
        object_paths.append([])

    for j in range(len(list_of_objs) - 1):  # записываем в список координаты контуров для отрисовки траекторий
        for i in range(len(list_of_objs[j]) - 1):
            current_frame_objs = list_of_objs[j]
            if current_frame_objs == []:
                continue
            left = current_frame_objs[i][1]
            top = current_frame_objs[i][2]
            right = current_frame_objs[i][3]
            bottom = current_frame_objs[i][4]
            id = current_frame_objs[i][0]

            x = round((left + right) / 2, 4)
            y = round((top + bottom) / 2, 4)

            object_paths[id - 1].append([x, y, j])

    show_paths(1, 10)


def show_paths(id, frame_n):  # отрисовка траекторий для контуров текущего кадра

    if len(object_paths) < 1:
        return
    w = canv.winfo_width()
    h = canv.winfo_height()

    r_color = color[id % len(color)]  # цвет выбирается от 0 до 10 элемента, 11=1...

    id -= 1
    skip = 10
    s_iter = 0
    for i in range(int((len(
            object_paths[id]) - 1) / skip)):  # рисуем линию от положения контура на предыдущем кадре до текущего
        pt1 = object_paths[id][s_iter]
        s_iter += skip
        pt2 = object_paths[id][s_iter]
        # print(pt1, pt2)
        if object_paths[id][i * skip][2] < frame_n:
            line = canv.create_line(int(pt1[0] * w), int(pt1[1] * h), int(pt2[0] * w), int(pt2[1] * h), fill=r_color,
                                    width=2)


def do_popup(event):
    try:
        m.tk_popup(event.x_root, event.y_root)
    finally:
        m.grab_release()


root = tk.Tk()
root.configure(width='850', height='650')
root.title("Tracker Mark II")
frame1 = tk.Frame(root)

# change icon
#root.iconbitmap("icon.ico")

if os.path.isfile("config.json"):  # читаем конфиг файл
    with open("config.json", "r") as conf:
        cfg = json.load(conf)
        root.geometry(cfg["Window_size"])
        print("cfg:", cfg)
else:
    root.eval('tk::PlaceWindow . center')  # выставялем в центе

canv = tk.Canvas(frame1)
canv.pack(side='top', fill=tk.BOTH, expand=True)

m = tk.Menu(root)  # создается объект Меню на главном окне
root.config(menu=m)  # окно конфигурируется с указанием меню для него

fm = tk.Menu(m)  # создается пункт меню с размещением на основном меню (m)
m.add_cascade(label="Меню", menu=fm)  # пункту располагается на основном меню (m)
fm.add_command(label="Открыть видео-файл..", command=open_video)  # формируется список команд пункта меню
fm.add_command(label="Экспорт объектов..", command=export_imgs)
fm.add_command(label="Экспорт контуров..", command=export_rects)
fm.add_command(label="Сохранить базу", command=save_database)
fm.add_command(label="Разметить Yolo", command=mark_yolo)
fm.add_command(label="Выход", command=on_close)

# pop-up меню на канвасе
mpopup = tk.Menu(canv, tearoff=0)
mpopup.add_command(label="Удалить текущий")
mpopup.add_command(label="Удалить все до")
mpopup.add_command(label="Удалить все после")
mpopup.add_command(label="Удалить вообще")

# canv.bind("<Button-3>", do_popup)

frame1.pack(padx='10', pady='10', side='top', expand='true', fill='both')

slider = ttk.Scale(frame1)
slider.configure(from_='0', orient='horizontal', state='normal', to=100, variable=a)
slider.pack(anchor='s', expand='true', fill='x', ipady='3', side='left')
slider.configure(command=change_scale_value)

lable_slider = ttk.Label(frame1)
lable_slider.configure(anchor='center', font='TkDefaultFont', text='0', width='10')
lable_slider.pack(side='bottom')

statusbar = ttk.Style(root)
# add the label to the progressbar style
statusbar.layout("LabeledProgressbar",
                 [('LabeledProgressbar.trough',
                   {'children': [('LabeledProgressbar.pbar',
                                  {'side': 'left', 'sticky': 'ns'}),
                                 ("LabeledProgressbar.label",  # label inside the bar
                                  {"sticky": ""})],
                    'sticky': 'nswe'})])

# statusbar = tk.Label(root, text="Откройте Меню->Открыть видеофайл и выберите видео", bd=1, relief=tk.SUNKEN, anchor=tk.S)
p_s = ttk.Progressbar(root, orient="horizontal", length=100, mode="determinate", style="LabeledProgressbar")
p_s.pack(side='bottom', fill=tk.X)

canv.bind("<Configure>", resize_image)
canv.bind("<Button-1>", rect_select)  # выбор контура на лкм

root.bind("<Left>", prev)
root.bind("<Right>", next)
root.protocol("WM_DELETE_WINDOW", on_close)
root.bind("<Escape>", remove_select)

# спрашиваем не открыть ли последний файл
video_file = cfg["Last_opened_video_file"]
if len(video_file) > 4:
    open_last = messageb.askokcancel(
        choice(["Я помню последний открытый файл", "Че у нас там было в последний раз...", "Здратути"]),
        f"Открыть {video_file}?")
    if open_last:
        open_video_thread()
else:
    open_video()

root.mainloop()
