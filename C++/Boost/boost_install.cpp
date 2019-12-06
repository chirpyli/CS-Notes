#include <iostream>
#include<boost/version.hpp>
#include<boost/config.hpp>

using namespace std;

int main() {
    cout << BOOST_VERSION << endl;
    cout << BOOST_LIB_VERSION << endl;
    cout << BOOST_PLATFORM << endl;
    cout << BOOST_COMPILER << endl;
    cout << BOOST_STDLIB << endl;

  return 0;
}
