#include <iostream>

using namespace std;

int main()
{
    double x = 0;
    double y = 0;
    cout << "Enter values for x and y: ";
    // same as:
    // cin >> x;
    // cin >> y;
    cin >> x >> y; 

    cout << "x + y = " << x + y << endl;

    return 0;
}
