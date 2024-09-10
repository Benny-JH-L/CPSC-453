#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    const double sales = 95000;       // $95000
    const double stateTax = 0.04;      // 4%
    const double countyTax = 0.02;    // 2%

    double stateTaxOwed = sales * stateTax;
    double countyTaxOwed = sales * countyTax;
    double totalTax = stateTaxOwed + countyTaxOwed;

    cout << "Total sales = $" << sales << endl
         << "State tax owed = $" << stateTaxOwed << endl
         << "County tax owed = $" << countyTaxOwed << endl
         << "Total tax owed = $" << totalTax << endl;

    return 0;
}
