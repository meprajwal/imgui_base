#include "memory.h"
#include <TlHelp32.h>

Memory::Memory(const char* processName)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	while (Process32Next(snapshot, &entry))
	{
		if (!strcmp(processName, entry.szExeFile))
		{
			this->id = entry.th32ProcessID;
			this->process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->id);
			break;
		}
	}
	if(snapshot)
		CloseHandle(snapshot);

	std::cout << "Process not found" << std::endl;

}

Memory::~Memory()
{
	if(this->process)
		CloseHandle(this->process);
}

DWORD Memory::GetProcessId()
{
	return this->id;

}
DWORD Memory::GetProcsesHandle()
{
	return (DWORD)this->process;

}

uintptr_t Memory::GetModuleAddress(const char* moduleName)
{
	MODULEENTRY32 entry;
	entry.dwSize = sizeof(MODULEENTRY32);
	
	const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, this->id);

	uintptr_t result = 0;


	while (Module32Next(snapshot, &entry))
	{
		if (!strcmp(moduleName, entry.szModule))
		{
			result = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
			break;
		}
	}
	if (snapshot)
		CloseHandle(snapshot);
	return result;	

}