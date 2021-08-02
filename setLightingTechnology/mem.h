#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <string>

HANDLE hProcess = NULL;

#define pmread	 ReadProcessMemory
#define pmwrite	 WriteProcessMemory
#define vcast(x) reinterpret_cast<void*>(x)

void mem_init() {
	DWORD dwProcessId = NULL;
	HWND hWnd = FindWindow(0, "Roblox");
	if (!hWnd) {
		MessageBox(0, "Roblox is not open", "Error", MB_ICONERROR);
		exit(0);
	}
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwProcessId);
}

template <class T>
T read(uintptr_t addr) {
	T temp;
	pmread(hProcess, vcast(addr), &temp, sizeof T, NULL);
	return temp;
}
#define readloc(addr) (read<uintptr_t>(addr))

template <class T>
void write(uintptr_t addr, T x) {
	pmwrite(hProcess, vcast(addr), &x, sizeof(x), NULL);
}

std::string read_string(DWORD addr) {
	std::string ret = "";
	int index = 0;

	while (BYTE c = read<BYTE>(readloc(addr) + index++))
		ret += c;

	return ret;
}