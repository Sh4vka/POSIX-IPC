#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>

const size_t BUFFER_SIZE = 1024;

void read_shm_and_xor(const std::string& shm_name1, const std::string& shm_name2, const std::string& output_file) {
    // Открытие первого сегмента разделяемой памяти
    int shm_fd1 = shm_open(shm_name1.c_str(), O_RDONLY, 0666);
    if (shm_fd1 == -1) {
        std::cerr << "Ошибка при открытии разделяемой памяти 1!" << std::endl;
        return;
    }

    // Открытие второго сегмента разделяемой памяти
    int shm_fd2 = shm_open(shm_name2.c_str(), O_RDONLY, 0666);
    if (shm_fd2 == -1) {
        std::cerr << "Ошибка при открытии разделяемой памяти 2!" << std::endl;
        return;
    }

    // Отображаем оба сегмента в память
    void* addr1 = mmap(NULL, BUFFER_SIZE, PROT_READ, MAP_SHARED, shm_fd1, 0);
    if (addr1 == MAP_FAILED) {
        std::cerr << "Ошибка при отображении памяти 1!" << std::endl;
        return;
    }

    void* addr2 = mmap(NULL, BUFFER_SIZE, PROT_READ, MAP_SHARED, shm_fd2, 0);
    if (addr2 == MAP_FAILED) {
        std::cerr << "Ошибка при отображении памяти 2!" << std::endl;
        return;
    }

    // Выполнение XOR дважды
    char result[BUFFER_SIZE];
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        result[i] = static_cast<char>(reinterpret_cast<char*>(addr1)[i] ^ reinterpret_cast<char*>(addr2)[i]);
    }

    // Второй XOR с тем же файлом
    char final_result[BUFFER_SIZE];
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        final_result[i] = static_cast<char>(result[i] ^ reinterpret_cast<char*>(addr2)[i]);
    }

    // Запись результата в файл
    std::ofstream out(output_file, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Ошибка при создании выходного файла!" << std::endl;
        return;
    }
    out.write(final_result, BUFFER_SIZE);

    // Закрытие файлов и отображенной памяти
    out.close();
    munmap(addr1, BUFFER_SIZE);
    munmap(addr2, BUFFER_SIZE);

    std::cout << "Операция XOR дважды завершена, результат сохранен в файл " << output_file << std::endl;
}

int main() {
    std::string file1 = "text1.txt";
    std::string file2 = "text2.txt";  // Файл с случайной последовательностью

    // Запуск программы 1 дважды
    std::string shm_name1 = "/shm_" + file1;
    std::string shm_name2 = "/shm_" + file2;

    // Запуск программы 1 для первого шифрования (text1.txt с random_sequence.txt)
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execl("./program1", "program1", file1.c_str(), file2.c_str(), nullptr);
        return 0;
    }

    // Ожидаем завершения процессов
    waitpid(pid1, nullptr, 0);

    // После первого шифрования, выполняем второй XOR с результатом и тем же файлом случайной последовательности
    read_shm_and_xor(shm_name1, shm_name2, "result_after_xor.txt");

    // Теперь файл result_after_xor.txt содержит данные, зашифрованные дважды
    std::cout << "Готово. Результат зашифрованного файла сохранен как result_after_xor.txt" << std::endl;

    return 0;
}
