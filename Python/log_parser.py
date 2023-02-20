import os
from tqdm import tqdm


list_of_txt = []


def remove_forbiden_chars(value):
    deletechars = '\/:*?"<>|'
    for c in deletechars:
        value = value.replace(c, '')
    return value


print("Working...")

# ищем в корне скрипта .txt файлы и добавляем их имена в список
for file in os.listdir("./"):
    if file.endswith(".txt"):
        list_of_txt.append(file)

print("Found .txt files:", list_of_txt)

# по всем найденным txt файлам:
for i in range(len(list_of_txt)):

    # получаем объект txt файла
    file = open(f"{list_of_txt[i]}", "r")

    # считаем число строк в текущем лог файле
    line_count = sum(1 for line in open(f"{list_of_txt[i]}"))

    print("\nSearching camera logs in", list_of_txt[i], f"with {line_count} strings ...")

    for s in tqdm(range(line_count)):
        # считываем строку
        line = file.readline()

        # прерываем цикл, если строка пустая
        if not line:
            break

        # ищем в строке имя камеры, заключенное в <>
        cam_name_start_with = int(line.find("<"))
        cam_name_end_with = int(line.find(">"))

        # если в строке нет имени камеры
        if cam_name_start_with == -1 or cam_name_end_with == -1:
            # создаем отдельную папку для логов без имени камеры
            if not os.path.isdir('./other_logs'):
                os.mkdir('./other_logs')
            # открываем txt, если не существует - он создается, и пишется текущая строка
            with open('./other_logs/log.txt', 'a') as log_txt:
                log_txt.write(f'{line}')
                log_txt.close()
            continue

        # вытаскиваем имя камеры из строки и прогоняем через фильтр запрещенных символов
        cam_name = line[cam_name_start_with + 1: cam_name_end_with]
        cam_name = remove_forbiden_chars(cam_name)

        # для пустых скобок принтим сообщение и строку
        if cam_name == "":
            print("\nUndefined camera name in line:", line)
            # создаем отдельную папку для логов без имени камеры
            if not os.path.isdir('./other_logs'):
                os.mkdir('./other_logs')
            # открываем txt, если не существует - он создается, и пишется текущая строка
            with open('./other_logs/log.txt', 'a') as log_txt:
                log_txt.write(f'{line}')
                log_txt.close()
            continue

        # проверяем наличие папки с соответствующим именем, если нет - создаем
        if not os.path.isdir(f'./{cam_name}'):
            os.mkdir(f"./{cam_name}")

        # открываем txt, если не существует - он создается, и пишется текущая строка
        with open(f'./{cam_name}/log.txt', 'a') as log_txt:
            log_txt.write(f'{line}')
            log_txt.close()


# закрываем файл и прогресс бар
file.close


print("Done!")
