#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <Psapi.h>

#include "mem.h"


std::string read_file(std::string file_path) {
	std::ifstream ifs;
	ifs.open(file_path, std::ifstream::in);
	std::string fdata = "";
	char c = ifs.get();
	while (ifs.good()) {
		fdata += c;
		c = ifs.get();
	}
	ifs.close();
	return fdata;
}

char* get_handle_path() {
	char szPath[MAX_PATH];
	GetModuleFileNameEx(hProcess, NULL, szPath, MAX_PATH);
	return szPath;
}

std::string getlogspath() {
	char* path = get_handle_path();
	int runtwice = 0;
	for (int i = strlen(path) - 1; i > 0; --i)
	{
		if (path[i] == '\\' && runtwice != 3)
		{
			path[i] = '\0';
			runtwice++;
		}
	}

	return std::string(path) + "\\logs";
}

std::string getrecentlog(std::string logspath = getlogspath()) {
	WIN32_FIND_DATAA FindFileData;
	FILETIME bestDate = { 0, 0 };
	std::string fullpath = "";
	FILETIME curDate;

	char patter[254];
	memset(patter, 0x00, 254);
	sprintf_s(patter, "%s\\*.log", logspath.c_str());
	HANDLE hFind = FindFirstFileA(patter, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) printf("invalid %d", GetLastError()); else {
		do { // ignore current and parent directories
			if (strcmp(FindFileData.cFileName, ".") == 0 || strcmp(FindFileData.cFileName, "..") == 0)
				continue;
			// dont list directories
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				//list the Files
				curDate = FindFileData.ftCreationTime;
				if (CompareFileTime(&curDate, &bestDate) > 0) {
					bestDate = curDate;
					fullpath = (logspath + "\\" + FindFileData.cFileName);
				}
			}
		} while (FindNextFileA(hFind, &FindFileData));
		FindClose(hFind);
	}

	return fullpath;
}

DWORD to_addr(std::string str) {
	DWORD dw = NULL;
	std::istringstream ss("0x" + str);
	ss >> std::hex >> dw;
	return dw;
}

// roblox class stuff (wont include my full one)

#define add(name, type, offset) \
auto name() { \
	return read<type>(this->ptr() + offset); \
} 

#define addptr(name, type, offset) \
auto name() { \
	return type(readloc(this->ptr() + offset)); \
} 

#define addstr(name, offset) \
auto name(bool ptr = false) { \
	if (ptr) return read_string(readloc(this->ptr() + offset)); \
	return read_string(this->ptr() + offset); \
}

class AVInstance {
public:
	AVInstance() {}
	~AVInstance() { instance = NULL; }
	AVInstance(void* p) { instance = reinterpret_cast<uintptr_t>(p); }
	AVInstance(uintptr_t p) { instance = static_cast<uintptr_t>(p); }

	AVInstance(const AVInstance& n) : instance(n.instance) {}

	addptr(parent, AVInstance, 0x34);

	addstr(name, 0x28);

	std::vector<AVInstance> getchildren() {
		std::vector<AVInstance> children;

		uintptr_t pChildArray = readloc(this->ptr() + 0x2C);
		uintptr_t pEndArray = readloc(pChildArray + 0x4); // child_start and child_end

		for (uintptr_t ptr = readloc(pChildArray); ptr != pEndArray; ptr += 8)
			children.emplace_back(readloc(ptr));

		return children;
	}

	AVInstance findfirstchild(std::string childname) {
		for (AVInstance child : this->getchildren())
			if (child.name() == childname)
				return child;
	}

	uintptr_t ptr() { return instance; }
	void set(uintptr_t p) { instance = p; }

	uintptr_t instance = NULL;
};

AVInstance get_client_replicator() { // to support teleporting just list out each time the string is found in the log file and use the last one
	std::string log = read_file(getrecentlog());
	return AVInstance(to_addr(log.substr(log.find("Replicator created: ") + 20, 8)));
}