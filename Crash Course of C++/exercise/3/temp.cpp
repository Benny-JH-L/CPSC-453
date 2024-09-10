#include <iostream>

using namespace std;

// converts Fahrenheit to Celcius
int main(int argc, char* argv[])
{
    cout << "Please enter fahrenheit to be converted: ";    // std::cout << "Please enter fahrenheit to be converted: ";  
    double fahrenheit = 0;
    cin >> fahrenheit;                                      // std::cin >> fahrenheit;     
    double celcuis = (fahrenheit - 32) * (double(5)/9); // if i do (5/9) i get 0, need one of the numbers to be a 'double'
    cout << "Celcuis = " << celcuis;                        // std::cout << "Celcuis = " << celcuis;

    return 0;
}
