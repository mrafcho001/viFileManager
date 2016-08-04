#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{

    auto lambda = [](auto a, auto b) { return a < b; };
    cout << "Hello World!" << endl;
    return 0;
}
