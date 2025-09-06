#include <iostream>
#include <cmath>

double geometricSeriesSum(double b1, double q, int n) {
    if (n <= 0) {
        return 0.0;
    }

    double sum = 0.0;
    double current = b1;
    
    for (int i = 0; i < n; i++) {
        sum += current;
        std::cout << current << " ";
        current *= q;
    }
    std::cout << std::endl << "Равна: ";
    return sum;
}

int main() 
{
    int N;
    std::cout << "Введите количество членов геометрической прогрессии: ";
    std::cin >> N;
    std::cout << "Сумма N членов: " 
         << geometricSeriesSum(2, 3, N) << std::endl;

}