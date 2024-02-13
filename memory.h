#pragma once

#include <Windows.h>
#include <iostream>

class Memory
{
private:
	DWORD id = 0;
	HANDLE process = NULL;
	
public:
	Memory(const char* processName); //constructor
	~Memory();//destructor

	DWORD GetProcessId();
	DWORD GetProcsesHandle();

	uintptr_t GetModuleAddress(const char* moduleName);


	template <typename T>
	T RPM(uintptr_t address)
	{
		T value; 
		ReadProcessMemory(this->process, (LPVOID)address, &value, sizeof(T), NULL);
		return value;
	}

	template <typename T>
	void WPM(uintptr_t address, T value)
	{
		WriteProcessMemory(this->process, (LPVOID)address, &value, sizeof(T), NULL);
	}

};

