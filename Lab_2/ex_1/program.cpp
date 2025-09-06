#include <iostream>

int main() 
{
    int value_c;
    int value_asm;
    std::cout << "Введите число: ";
    std::cin >> value_c;
    value_asm = value_c;

    value_c = value_c*value_c;

    asm(
        "imull %[v], %[v]"
        : [v] "+r" (value_asm) // value — вход и выход
    );

    std::cout << "Значение C++: " << value_c << std::endl;
    std::cout << "Значение ASM: " << value_asm << std::endl;
}