import sys
from multiprocessing import shared_memory

def read_write_to_shared_memory(filename, shm_name, size):
    # Удаляем старую разделяемую память, если она существует
    try:
        existing_shm = shared_memory.SharedMemory(name=shm_name)
        existing_shm.unlink()  # Удаляем старую память
    except FileNotFoundError:
        pass  # Просто пропускаем, если памяти нет

    try:
        with open(filename, 'r') as f:
            data = f.read(size)
    except FileNotFoundError:
        print("File not found")
        sys.exit(1)

    # Создаём или открываем уже существующую разделяемую память
    shm = shared_memory.SharedMemory(name=shm_name, create=True, size=size)
    buffer = shm.buf
    buffer[:len(data)] = data.encode('utf-8')
    for i in range(len(data), size):
        buffer[i] = 0

    print(f"Parent process: Data written to shared memory '{shm_name}'.")

    # Вывод данных, находящихся в разделяемой памяти
    print("Data in shared memory:")
    data_in_shm = shm.buf[:len(data)].tobytes().rstrip(b'\0').decode('utf-8')
    print(data_in_shm)

    shm.close()


def main():
    if len(sys.argv) != 3:
        print("Usage: python3 first.py <file_name> <shm_name>")
        sys.exit(1)

    file_name = sys.argv[1]
    shm_name = sys.argv[2]
    shm_size = 4096

    read_write_to_shared_memory(file_name, shm_name, shm_size)


if __name__ == "__main__":
    main()
