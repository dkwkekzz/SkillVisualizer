// ProcessManager.cpp
#include "pch.h"
#include "ProcessManager.h"

DWORD getPID(const wchar_t* target)
{
	DWORD pid = _wtoi(target);
	if (pid != 0)
		return pid;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	bool exist = false;
	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_wcsicmp(target, entry.szExeFile) == 0)
			{
				pid = entry.th32ProcessID;
				break;
			}
		}
	}

	CloseHandle(snapshot);
	return pid;
}

//You read module information like this..
MODULEENTRY32 GetModuleInfo(std::uint32_t ProcessID, const wchar_t* ModuleName)
{
	void* hSnap = nullptr;
	MODULEENTRY32 Mod32 = { 0 };

	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, NULL)) == INVALID_HANDLE_VALUE)
		return Mod32;

	Mod32.dwSize = sizeof(MODULEENTRY32);
	while (Module32Next(hSnap, &Mod32))
	{
		if (!wcscmp(ModuleName, Mod32.szModule))
		{
			CloseHandle(hSnap);
			return Mod32;
		}
	}

	CloseHandle(hSnap);
	return { 0 };
}

HMODULE GetProcessBaseAddressX64(DWORD processID)
{
	HANDLE processHandle = NULL;
	if (!(processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID)))
	{
		printf("[ERROR] OpenProcess(%d) failed!!! [%d]\n", processID, GetLastError());
		return FALSE;
	}

	HMODULE		baseAddress = 0;
	HMODULE     *moduleArray;
	LPBYTE      moduleArrayBytes;
	DWORD       bytesRequired;
	LPVOID		pRemoteBuf = NULL;

	if (processHandle)
	{
		if (EnumProcessModulesEx(processHandle, NULL, 0, &bytesRequired, LIST_MODULES_ALL))
		{
			if (bytesRequired)
			{
				moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

				if (moduleArrayBytes)
				{
					unsigned int moduleCount;

					moduleCount = bytesRequired / sizeof(HMODULE);
					moduleArray = (HMODULE *)moduleArrayBytes;

					if (EnumProcessModulesEx(processHandle, moduleArray, bytesRequired, &bytesRequired, LIST_MODULES_ALL))
					{
						baseAddress = (HMODULE)moduleArray[0];
					}

					LocalFree(moduleArrayBytes);
				}
			}
		}
		else
		{
			printf("[ERROR] EnumProcessModulesEx(%d) failed!!! [%d]\n", processID, GetLastError());
		}

		CloseHandle(processHandle);
	}

	return baseAddress;
}

HMODULE GetModule(HANDLE handle, const std::wstring& target)
{	// not working...
	HMODULE hMods[1024];
	HANDLE pHandle = handle;
	DWORD cbNeeded;
	unsigned int i;

	if (EnumProcessModules(pHandle, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(pHandle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				std::wstring wstrModName = szModName;
				if (wstrModName.find(target) != std::string::npos)
				{
					CloseHandle(pHandle);
					return hMods[i];
				}
			}
		}
	}
	return nullptr;
}

BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&hToken))
	{
		_tprintf(L"OpenProcessToken error: %u\n", GetLastError());
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL,             // lookup privilege on local system
		lpszPrivilege,    // privilege to lookup 
		&luid))          // receives LUID of privilege
	{
		_tprintf(L"LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.
	if (!AdjustTokenPrivileges(hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		_tprintf(L"AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		_tprintf(L"The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}
