#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Hello World!" << std::endl;
    std::cout << "Hello World!2\n";
    std::cout << "Hello World!3\n";

    int x = 10;

    std::cout << "Hello world! " + x << std::endl;   // will print all the characters at the 10th index and after
    std::cout << "Hello world! " << x;

    return 0;
}
