#pragma warning(disable : 4996)
#include <iostream>
#include <windows.h>
#include <string>
using std::cout;
using std::cin;
using std::endl;
using std::string;
struct employee
{
    int num = -1;
    char name[10] = "";
    double hours = 0;
};
int main(int argc, char* argv[]) {
    HANDLE EvReady = OpenEventA(EVENT_MODIFY_STATE, FALSE, argv[1]);
    HANDLE EvStart = OpenEventA(SYNCHRONIZE, FALSE, "START_ALL");
    SetEvent(EvReady);
    WaitForSingleObject(EvStart, INFINITE);
    HANDLE hPipe;
    while (true) {
        hPipe = CreateFile("\\\\.\\pipe\\nPipe", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE) {
            break;
        }
    }
    while (true) {
        string temp;
        cout << "Input 1 if you want to read, 2 if you want to write ID or 3 if you want to exit" << endl;
        cin >> temp;
        if (temp == "3") {
            break;
        }
        DWORD writeByte;
        bool isSend;        
        isSend = WriteFile(hPipe, temp.c_str(), 10, &writeByte, NULL);
        if (!isSend) {
            cout << "Message can't be send" << endl;
            return 0;
        }
        employee emp;
        if (emp.num < 0) {
            cout << "Employee not found" << endl;
            continue;
        }
        else {
            cout << emp.num << " " << emp.name << " " << emp.hours << endl;
            if (temp == "2") {
                cout << "Input ID, name and hours" << endl;
                cin >> emp.num >> emp.name >> emp.hours;          
                isSend = WriteFile(hPipe, &emp, sizeof(emp), &writeByte, NULL);
                if (!isSend) {
                    cout << "Error in sending." << endl;
                }             
            }
        }
    }
    return 0;
}