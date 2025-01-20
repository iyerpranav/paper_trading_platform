#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <thread>
using namespace std;
    
int main(){
    int returnCode = system("python stockdb.py");
            if (returnCode == 0) {
                cout << "\tPython portfolio program executed successfully." << endl;
            } else {
                cerr << "Error executing Python portfolio program." << endl;
            }    
    
}
