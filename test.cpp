#include <iostream>
#include <string>
#include "boost/lexical_cast.hpp"
using namespace std;
int main()
{
    cout << "123456" << endl;
    cout << "Enter your weight: ";
    float weight;
    std::cin >> weight;
    string gain = "A 10% increase raises ";
    string wt = boost::lexical_cast<string>(weight);
    gain = gain + wt + " to "; // string operator()
    weight = 1.1 * weight;
    gain = gain + boost::lexical_cast<string>(weight) + ".";
    cout << gain << endl;
    system("pause");
    return 0;
}