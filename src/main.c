#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

#include "speach.h"

DWORD GetProcessId(const wchar_t* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = (CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					break;
				}

			} while (Process32Next(hSnap, &procEntry));
		}
	}

	CloseHandle(hSnap);
	return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = (CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId));
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}

			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

int main(int argc, char* argv[])
{
	DWORD procId = GetProcessId(L"tf_win64.exe");
	if (!procId)
	{
		printf("Failed to find process.");
		return 0;
	}

	HANDLE procHandle = OpenProcess(PROCESS_VM_READ, FALSE, procId);
	if (!procHandle)
	{
		printf("Failed to get handle to process.");
		return 0;
	}

	uintptr_t tier0 = GetModuleBaseAddress(procId, L"tier0_s64.dll");
	if (!tier0)
	{
		printf("Failed to get base address of tier0_s64.dll");
		return 0;
	}

	char lastMsg[255];
	lastMsg[0] = 0;

	while (1) 
	{
		uintptr_t addr = 0;
		ReadProcessMemory(procHandle, tier0 + 0x61990, &addr, sizeof(uintptr_t), 0);

		char msg[255];
		ReadProcessMemory(procHandle, addr + 0x8, msg, 255, 0);
		msg[254] = 0;

		if (strcmp(lastMsg, msg) == 0) 
		{
			continue;
		}

		memcpy(lastMsg, msg, 255);

		unsigned char gotMsg = 0;
		unsigned char foundColon = 0;
		int msgStartIndex = 0;
		int charsSinceColon = 0;
		while (msgStartIndex < 252)
		{
			if (foundColon)
			{
				if (charsSinceColon > 3) 
				{
					break;
				}
				else if (msg[msgStartIndex] >= '!' && msg[msgStartIndex] <= 'z') 
				{
					gotMsg = 1;
					break;
				}
				
				charsSinceColon++;
			}
			
			if (!foundColon && msg[msgStartIndex] == ':' && msg[msgStartIndex+1] == ' ')
			{
				foundColon = 1;
				msgStartIndex++;
			}
			
			msgStartIndex++;
		}

		if (!gotMsg)
		{
			continue;
		}

		printf("Original text: %s\n", msg);
		printf("Message start index: %d\n", msgStartIndex);
		printf("Message: %s\n", (msg + msgStartIndex));

		wchar_t wc[255];
		mbstowcs(wc, msg + msgStartIndex, 255);

		speak(wc);

		Sleep(1000);
	}
	

	return 0;
}