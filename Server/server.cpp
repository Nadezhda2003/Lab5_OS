#pragma warning(disable : 4996)
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::string;
CRITICAL_SECTION cs;
struct employee
{
	int num = -1;
	char name[10] = "";
	double hours = 0;
};
employee* arr;
bool* mod;
int numOfEntries;
DWORD WINAPI mes(LPVOID p) {
	HANDLE hPipe = (HANDLE)p;	
	employee* error = new employee;
	while (true) {
		bool isRead = false;
		char message[10];
		DWORD readBytes;
		isRead = ReadFile(hPipe, message, 10, &readBytes, NULL);
		if (!isRead) {
			if (ERROR_BROKEN_PIPE == GetLastError()) {
				cout << "Client disconected." << endl;
				return 0;
			}
			else {
				cout << "Error in reading." << endl;
				return 0;
			}
		}
		if (strlen(message) > 0) {
			char command = message[0];
			message[0] = ' ';
			int id = atoi(message);
			DWORD bytesWritten;
			EnterCriticalSection(&cs);
			employee* toSend = new employee;
			for (int i = 0; i < numOfEntries; i++) {
				if (id == arr[i].num) {
					toSend->hours = arr[i].hours;
					toSend->num = arr[i].num;
					for(int j = 0; j< 10; j++){
						toSend->name[j] = arr[i].name[j];
					}
					break;
				}
			}			
			LeaveCriticalSection(&cs);
			if (NULL == toSend) {
				toSend = error;
			}
			else {
				int ind = toSend - arr;
				switch (command) {
				case '2':
					cout << "ID for modifing: " << id << endl;
					mod[ind] = true;
					break;
				case '1':
					cout << "ID for reading: " << id << endl;
					break;
				default:
					cout << "Error ID." << endl;
				}
			}
			bool isSent = WriteFile(hPipe, toSend, sizeof(employee), &bytesWritten, NULL);
			if (!isSent) {
				cout << "Error in sending answer." << endl;
			}
			if ('2' == command && toSend != error) {
				DWORD readBytes;
				isRead = ReadFile(hPipe, message, numOfEntries, &readBytes, NULL);
				if (!isRead) {
					if (ERROR_BROKEN_PIPE == GetLastError()) {
						cout << "Disconnected." << endl;
						return 0;
					}
					else {
						cout << "Error in reading." << endl;
						return 0;
					}
				}				
				if (isRead) {
					mod[toSend - arr] = false;
					EnterCriticalSection(&cs);
					LeaveCriticalSection(&cs);
				}
				else {
					cout << "Error in reading." << endl;
				}
				break;
			}
		}
	}
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	delete error;
	return 0;
}
int main() {
	char fileName[66];
	cout << "Input name of the file:" << endl;
	cin >> fileName;
	cout << "Input the number of entries: " << endl;
	cin >> numOfEntries;
	cout << "Input ID, name and number of hours worked:" << endl;
	arr = new employee[numOfEntries];
	for (int i = 0; i < numOfEntries; i++) {		
		cout << "Input the information of " << i + 1 << " worker:" << endl;
		cin >> arr[i].num >> arr[i].name >> arr[i].hours;
	}
	ifstream in;
	ofstream out;
	out.open(fileName, ofstream::binary);
	out.write((const char*)arr, sizeof(employee) * numOfEntries);
	out.close();
	cout << "Initial data:" << endl;
	for (int i = 0; i < numOfEntries; i++) {
		cout << arr[i].num << " " << arr[i].name << " " << arr[i].hours << endl;
	}
	cout << "Input number of processes: " << endl;
	int numOfProcesses;
	cin >> numOfProcesses;
	InitializeCriticalSection(&cs);
	HANDLE hStartALL = CreateEvent(NULL, TRUE, FALSE, "START_ALL");
	mod = new bool[numOfEntries];
	for (int i = 0; i < numOfEntries; i++) {
		mod[i] = false;
	}
	HANDLE* hEvents = new HANDLE[numOfProcesses];
	for (int i = 0; i < numOfProcesses; i++) {
		string processName = "Client.exe";
		string eventName = "EVENT_";
		eventName = eventName + std::to_string(i + 1);
		processName = processName + " " + eventName;
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		char* _eventName = new char(eventName.size()+1); 
		_eventName = strcpy(_eventName, eventName.c_str());
		char* _processName = new char(processName.size() + 1);
		_processName = strcpy(_processName, processName.c_str());
		hEvents[i] = CreateEvent(NULL, TRUE, FALSE, _eventName);
		if (!CreateProcess(NULL, _processName, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
			GetLastError();
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
	WaitForMultipleObjects(numOfProcesses, hEvents, TRUE, INFINITE);	
	SetEvent(hStartALL);
	HANDLE hPipe;
	HANDLE* hThreads = new HANDLE[numOfProcesses];
	for (int i = 0; i < numOfProcesses; i++) {
		hPipe = CreateNamedPipe("\\\\.\\pipe\\nPipe", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0, INFINITE, NULL);
		if (INVALID_HANDLE_VALUE == hPipe) {
			cout << "Creating named pipe ended with error." << endl;
			break;
		}
		hThreads[i] = CreateThread(NULL, 0, mes, (LPVOID)hPipe, 0, NULL);
	}
	WaitForMultipleObjects(numOfProcesses, hThreads, TRUE, INFINITE);
	cout << "File after changings:" << endl;
	for (int i = 0; i < numOfEntries; i++) {
		cout << arr[i].num << " " << arr[i].name << " " << arr[i].hours << endl;
	}
	cout << "Press any key to end: "<< endl;
	getch();
	DeleteCriticalSection(&cs);
	delete[] hThreads;
	delete[] hEvents;
	delete[] mod;
	delete[] arr;
	return 0;
}