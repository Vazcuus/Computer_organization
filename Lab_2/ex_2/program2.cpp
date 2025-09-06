#include <iostream>
#include <chrono>

// Функция для вычисления с использованием ассемблерной вставки
// Возвращает итоговую сумму для проверки корректности
long long run_asm_version(int N) {
    long long current = 1, sum = 0;
    int q = 2;
    for (int i = 0; i < N; i++) {
        asm (
            "add %[curr], %[s];"
            "imulq %[q_val], %[curr];"
            : [s] "+r" (sum), [curr] "+r" (current)
            : [q_val] "r" ((long long)q)
        );
    }
    return sum;
}

// Функция для вычисления с использованием чистого C++
// Возвращает итоговую сумму для проверки корректности
long long run_cpp_version(int N) {
    long long current = 1, sum = 0;
    int q = 2;
    for (int i = 0; i < N; i++) {
        sum += current;
        current *= q;
    }
    return sum;
}

int main() 
{
    int N;
    std::cout << "Введите количество членов геометрической прогрессии: ";
    std::cin >> N;

    // --- Вывод членов прогрессии (делаем один раз, не замеряем) ---
    long long current_display = 1;
    int q_display = 2;
    std::cout << "Члены прогрессии: ";
    for (int i = 0; i < N; i++) {
        std::cout << current_display << " ";
        current_display *= q_display;
    }
    std::cout << std::endl;

    // --- Замер производительности ---
    const int measurement_iterations = 100000; // Количество повторений для замера
    
    // Замер ассемблерной версии
    auto start_asm = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < measurement_iterations; ++i) {
        run_asm_version(N);
    }
    auto end_asm = std::chrono::high_resolution_clock::now();
    auto duration_asm = std::chrono::duration_cast<std::chrono::nanoseconds>(end_asm - start_asm);
    
    // Замер C++ версии
    auto start_cpp = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < measurement_iterations; ++i) {
        run_cpp_version(N);
    }
    auto end_cpp = std::chrono::high_resolution_clock::now();
    auto duration_cpp = std::chrono::duration_cast<std::chrono::nanoseconds>(end_cpp - start_cpp);

    // --- Вывод результатов ---
    std::cout << "\nРезультаты после " << measurement_iterations << " итераций:" << std::endl;
    std::cout << "Сумма (проверка ASM): " << run_asm_version(N) << std::endl;
    std::cout << "Сумма (проверка C++): " << run_cpp_version(N) << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Время выполнения (ASM): " << duration_asm.count() << " наносекунд" << std::endl;
    std::cout << "Время выполнения (C++): " << duration_cpp.count() << " наносекунд" << std::endl;
}