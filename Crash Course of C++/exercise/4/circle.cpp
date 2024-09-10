#include <iostream>
#include <cmath>

// calculate the area of a circle from the user inputting the radius
using namespace std;

int main()
{
    cout << "Enter a radius: ";
    double radius = 0;
    cin >> radius;
    double pi = 3.141592653589793;
    double area = pi * (pow(radius, 2));
    cout << "Area = " << area << endl;

    return 0;
}
