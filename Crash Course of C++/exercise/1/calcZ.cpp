#include <iostream>

int main(int argc, char* argv[])
{
    double x = 10;
    double y = 5;
    double z = 0;

    z = (x + 10) / (3 * y);

    std::cout << "z = " << z;   // expected result is z = 1.3333...

    return 0;
}
