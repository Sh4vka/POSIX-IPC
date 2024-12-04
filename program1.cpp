#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

const size_t BUFFER_SIZE = 1024;

void write_to_shm(const std::string& file_name, const std::string& shm_name) {
    // Открытие файла
    std::ifstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка при открытии файла: " << file_name << std::endl;
        return;
    }

    // Чтение содержимого файла
    char buffer[BUFFER_SIZE];
    file.read(buffer, BUFFER_SIZE);
    size_t bytes_read = file.gcount();
    file.close();

    // Открытие или создание сегмента разделяемой памяти
    int shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Ошибка при открытии сегмента разделяемой памяти!" << std::endl;
        return;
    }

    // Установка размера сегмента разделяемой памяти
    if (ftruncate(shm_fd, bytes_read) == -1) {
        std::cerr << "Ошибка при установке размера сегмента разделяемой памяти!" << std::endl;
        return;
    }

    // Отображение сегмента разделяемой памяти в адресное пространство процесса
    void* addr = mmap(NULL, bytes_read, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        std::cerr << "Ошибка при отображении памяти!" << std::endl;
        return;
    }

    // Запись данных в разделяемую память
    std::memcpy(addr, buffer, bytes_read);

    // Закрытие сегмента разделяемой памяти
    munmap(addr, bytes_read);
    close(shm_fd);

    std::cout << "Данные из файла " << file_name << " записаны в разделяемую память " << shm_name << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Использование: program1 <file1> <file2>" << std::endl;
        return 1;
    }

    // Получение имен файлов
    std::string file1 = argv[1];
    std::string file2 = argv[2];

    // Названия для разделяемой памяти
    std::string shm_name1 = "/shm_" + file1;
    std::string shm_name2 = "/shm_" + file2;

    // Запись данных в разделяемую память
    write_to_shm(file1, shm_name1);
    write_to_shm(file2, shm_name2);

    return 0;
}
