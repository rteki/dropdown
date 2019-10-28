#include <string>
#include <vector>
#include <stdarg.h>
#include <iostream>
#include <algorithm>
#include <map>
#include <Windows.h>
#include <thread>

using std::vector;
using std::string;
using std::map;

struct Parameters {
	string cmd;
	string title = "";
	map<string, int> optional = {
		{ "-h", 600 },
		{ "-w", 1980 },
		{ "-k", 0x70 },
		{ "-x", 0 },
		{ "-y", 0 }
	};
};

void parseArgv(vector<string>);
void cutAndMoveWindow();
void registerHotKey();
BOOL CALLBACK findWindowCallback(HWND hwnd, LPARAM lParam);


const vector<string> FLAGS = {"-h", "-w", "-k", "-x", "-y" };

// GLOBALS
Parameters params;
HWND window;
DWORD process;
STARTUPINFO si;
PROCESS_INFORMATION pi;

int main(int argc, char** argv) {
	
	HWND serviceWindow = GetConsoleWindow();

	ShowWindow(serviceWindow, SW_HIDE);

	vector<string> args;

	for (int i = 0; i < argc; i++) {
		args.push_back(string(argv[i]));
	}

	parseArgv(args);

	std::thread t(registerHotKey);

	t.detach();


	while (true) {
		CreateProcessA(NULL, const_cast<char*>(params.cmd.c_str()), NULL, FALSE, 0, NULL, NULL, NULL, &si, &pi);

		if (params.title.size() > 0) {
			window = NULL;
			do {
				Sleep(100);
				window = FindWindowA(NULL, params.title.c_str());
			} while (!window);
		}
		else {
			do {
				Sleep(100);
				EnumWindows(findWindowCallback, pi.dwProcessId);
			} while (!window);
		}


		cutAndMoveWindow();

		DWORD id;

		GetWindowThreadProcessId(window, &id);

		HANDLE waitProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, id);

		WaitForSingleObject(waitProcessHandle, INFINITE);
	 }

	return 0;
}


void parseArgv(vector<string> args) {
	
	auto it = std::find(args.begin(), args.end(), "-c");

	if ( it < args.end() - 1 ) {
		params.cmd = *(it + 1);
	}
	else {
		std::cout << "Please specify cmd: -c cmd" << std::endl;
		exit(-1);
	}

	it = std::find(args.begin(), args.end(), "-t");

	if (it < args.end() - 1) {
		params.title = *(it + 1);
	}

	for (auto flag : FLAGS) {
		it = std::find(args.begin(), args.end(), flag);

		if (it < args.end() - 1) {
			int value = std::stoi(*(it + 1));
			params.optional[flag] = value;
		}

	}

}


void cutAndMoveWindow() {
	SetWindowLongA(window, GWL_STYLE, 0);
	SetWindowPos(window, HWND_TOPMOST, params.optional["-x"], params.optional["-y"], params.optional["-w"], params.optional["-h"], SWP_SHOWWINDOW);
}

BOOL CALLBACK findWindowCallback(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		window = hwnd;
		return FALSE;
	}
	return TRUE;
}

void ShowOrHideWindow();

void registerHotKey() {
	RegisterHotKey(0, 12345, 0, params.optional["-k"]);
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		if (msg.message == WM_HOTKEY)
		{
			ShowOrHideWindow();
		}
	}
	
}

BYTE state = 5;

void ShowOrHideWindow() {
	state ^= 5;
	ShowWindow(window, state);
}