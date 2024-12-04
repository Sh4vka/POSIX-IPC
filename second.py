from multiprocessing import shared_memory
import subprocess
import time
import os

# Функция для чтения данных из разделяемой памяти
def read_shared_memory(shm_name, size):
    try:
        shm = shared_memory.SharedMemory(name=shm_name)
        buffer = shm.buf
        data = buffer[:size]  # Получаем данные как байты
        print(f"Data read from shared memory '{shm_name}': {data}")
        shm.close()
        return data
    except FileNotFoundError:
        print(f"Shared memory '{shm_name}' not found.")
        return None

# Функция для выполнения первой программы
def run_first_prog(file_name, shm_name):
    subprocess.run(["python3", "first.py", file_name, shm_name], check=True)

# Функция для выполнения операции XOR
def xor_data(data1, data2):
    # Выполняем побитовую операцию XOR на всех байтах
    return bytes([b1 ^ b2 for b1, b2 in zip(data1, data2)])

# Функция для записи данных в файл
def write_to_file(filename, data):
    with open(filename, "wb") as f:
        f.write(data)

# Основная функция
def main():
    text_file = "text1.txt"
    random_file = "text2.txt"
    shm_size = 4096
    output_file = "xor_result.txt"

    print("Создаем две разделенные памяти")
    run_first_prog(text_file, "shm_text")
    time.sleep(1)  # Добавление задержки для ожидания завершения записи
    run_first_prog(random_file, "shm_random")
    time.sleep(1)

    # Чтение данных из разделяемой памяти
    print("Читаем данные")
    text_data = read_shared_memory("shm_text", shm_size)
    random_data = read_shared_memory("shm_random", shm_size)

    if text_data and random_data:
        # Применяем операцию XOR
        xor_result = xor_data(text_data, random_data)

        # Записываем результат в файл
        print(f"Записываем результат XOR в файл '{output_file}'")
        write_to_file(output_file, xor_result)

    # Шаг 2: Запускаем вторую программу с файлом, полученным в результате XOR и случайной последовательностью
    print("Запускаем вторую программу с результатом XOR и случайной последовательностью")
    run_first_prog(output_file, "shm_text_2")  # Новый shm для второго шага
    time.sleep(1)

    # Чтение данных после второго шага
    print("Читаем данные после второго XOR")
    final_data = read_shared_memory("shm_text_2", shm_size)

    if final_data:
        # Вторичный XOR с результатом второго шага
        xor_result_final = xor_data(xor_result, final_data)

        # Сравниваем текстовый файл до и после шифрования
        print("Сравниваем исходный и зашифрованный файл")

        with open(text_file, "rb") as f:
            original_data = f.read()

        # Сравниваем исходные и зашифрованные данные
        if original_data == xor_result_final:
            print("Исходный и зашифрованный файлы совпадают!")
        else:
            print("Файлы не совпадают!")

    # Попытка удалить память, если она больше не используется
    try:
        shm_text = shared_memory.SharedMemory(name="shm_text")
        shm_text.unlink()  # Удаляем разделяемую память
    except FileNotFoundError:
        print("Shared memory 'shm_text' already unlinked.")
    
    try:
        shm_random = shared_memory.SharedMemory(name="shm_random")
        shm_random.unlink()  # Удаляем разделяемую память
    except FileNotFoundError:
        print("Shared memory 'shm_random' already unlinked.")

    try:
        shm_text_2 = shared_memory.SharedMemory(name="shm_text_2")
        shm_text_2.unlink()  # Удаляем разделяемую память
    except FileNotFoundError:
        print("Shared memory 'shm_text_2' already unlinked.")

if __name__ == "__main__":
    main()
