#include <iostream>
#include <vector>
#include <random>
#include <cstdint>
#include <chrono>
#include <immintrin.h> 
#include <iomanip> 

/*
    __m256 по сути
    ( lane3 | lane2 | lane1 | lane0 )
    здесь по 64 бита числа, 4 линии
    если рассматривать как 4 байта то:
    ( [ a3_hi | a3_lo ], [ a2_hi | a2_lo ], [ a1_hi | a1_lo ], [ a0_hi | a0_lo ])   
*/ 
const int N = 16;

inline __m256i _mm256_mullo_epi64_emu(const __m256i a, const __m256i b) {
    // a_lo * b_lo.
    // _mm256_mul_epu32 перемножает младшие 32-битные части в каждой 64-битной
    // полосе и возвращает полный 64-битный результат.
    __m256i lo_lo_prod = _mm256_mul_epu32(a, b);

    // a_hi * b_lo.
    // Маска 0xB1 (бинарно 10110001) меняет местами
    // 32-битные слова в каждой 64-битной паре.
    __m256i a_hi_shuf = _mm256_shuffle_epi32(a, 0xB1); // a_hi_shuf содержит 
    // ( [ a3_lo | a3_hi ], [ a2_lo | a2_hi], [ a1_lo | a1_hi ], [ a0_lo | a0_hi ]) 
    __m256i hi_lo_prod = _mm256_mul_epu32(a_hi_shuf, b);

    // произведение a_lo * b_hi.
    __m256i b_hi_shuf = _mm256_shuffle_epi32(b, 0xB1);
    __m256i lo_hi_prod = _mm256_mul_epu32(a, b_hi_shuf);
    // ( [ b3_lo | b3_hi ], [ b2_lo | b2_hi], [ b1_lo | b1_hi ], [ b0_lo | b0_hi ]) 
    
    // сложение перекрестных и сдвиг влево на 32 бита.
    __m256i cross_prod_sum = _mm256_add_epi64(hi_lo_prod, lo_hi_prod);
    __m256i cross_prod_shifted = _mm256_slli_epi64(cross_prod_sum, 32);

    // финальный результат младших 64 битов
    return _mm256_add_epi64(lo_lo_prod, cross_prod_shifted);
}

void print_matrix_portion(const std::vector<std::vector<int64_t>>& m, const std::string& title) {
    std::cout << "\n--- " << title << " ---" << std::endl;
    int print_size = std::min(8, N); // Печатаем не больше, чем 8x8
    for (int i = 0; i < print_size; ++i) {
        for (int j = 0; j < print_size; ++j) {
            std::cout << std::setw(8) << m[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "------------------------------------------------" << std::endl;
}

// Функция для генерации матрицы NxN с произвольными 64-битными целыми числами
void generate_matrix(std::vector<std::vector<int64_t>>& matrix) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int64_t> distrib(0, 200);

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = distrib(gen);
        }
    }
}

// Функция для скалярного умножения матриц
void scalar_matrix_multiplication(const std::vector<std::vector<int64_t>>& A,
                                  const std::vector<std::vector<int64_t>>& B,
                                  std::vector<std::vector<int64_t>>& C) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int64_t sum = 0;
            for (int k = 0; k < N; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

void avx2_matrix_multiplication(const std::vector<std::vector<int64_t>>& A, const std::vector<std::vector<int64_t>>& B, std::vector<std::vector<int64_t>>& C) {
    for (int i = 0; i < N; ++i) {
        std::fill(C[i].begin(), C[i].end(), 0);
    }
    
    for (int i = 0; i < N; ++i) {
        for (int k = 0; k < N; ++k) {
            // Установка в 4 полосы значения A[i][k]
            __m256i a_vec = _mm256_set1_epi64x(A[i][k]);
            
            // Обрабатываем строку B по 4 элемента за раз
            for (int j = 0; j < N; j += 4) {
                // Загрузка 4 элементов из B
                __m256i b_vec = _mm256_loadu_si256((__m256i*)&B[k][j]);
                // Загружаем 4 элементов из C
                __m256i c_vec = _mm256_loadu_si256((__m256i*)&C[i][j]);
                
                // Основная операция: c_vec += a_vec * b_vec
                c_vec = _mm256_add_epi64(c_vec, _mm256_mullo_epi64_emu(a_vec, b_vec));
                
                // res
                _mm256_storeu_si256((__m256i*)&C[i][j], c_vec);
            }
        }
    }
}

bool Sravnenie(const std::vector<std::vector<int64_t>>& A, const std::vector<std::vector<int64_t>>& B)
{
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (A[i][j] != B[i][j]) {
                return false;
            }
        }
    }
    return true;
}
size_t calculateMemory(int rows, int cols, size_t elementSize = sizeof(int64_t))
{
    return rows * cols * elementSize;
}



int main() {
    // Выделение памяти для матриц
    std::vector<std::vector<int64_t>> A(N, std::vector<int64_t>(N));
    std::vector<std::vector<int64_t>> B(N, std::vector<int64_t>(N));
    std::vector<std::vector<int64_t>> C_avx(N, std::vector<int64_t>(N));
    std::vector<std::vector<int64_t>> C_sca(N, std::vector<int64_t>(N));

    // Генерация матриц
    std::cout << "Generating matrices A and B (" << N << "x" << N << ")..." << std::endl;
    generate_matrix(A);
    generate_matrix(B);
    std::cout << "Generation complete." << std::endl;
    
    size_t theorySize = calculateMemory(N, N);
    std::cout << "Theory memory size one matrix: " << theorySize << std::endl;
    
    if (N <= 16)
    {
        print_matrix_portion(A, "Matrix A");
        print_matrix_portion(B, "Matrix B");
    }

    // Скалярное вычисление
    std::cout << "\nStarting scalar multiplication..." << std::endl;
    auto start_scalar = std::chrono::high_resolution_clock::now();
    scalar_matrix_multiplication(A, B, C_sca);
    auto end_scalar = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_scalar = end_scalar - start_scalar;
    std::cout << "Scalar multiplication finished in " << diff_scalar.count() << " s" << std::endl;

    // Вычисление с AVX2
    std::cout << "\nStarting AVX2 multiplication..." << std::endl;
    auto start_avx = std::chrono::high_resolution_clock::now();
    avx2_matrix_multiplication(A, B, C_avx);
    auto end_avx = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_avx = end_avx - start_avx;
    std::cout << "AVX2 multiplication finished in " << diff_avx.count() << " s" << std::endl;
    
    if (N <= 16)
    {
        print_matrix_portion(C_sca, "Scalar Result");
        print_matrix_portion(C_avx, "AVX2 Result");
    }


    return 0;
}

