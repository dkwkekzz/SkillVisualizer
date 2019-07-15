#pragma once

DWORD getPID(const wchar_t* target);
HMODULE GetModule(HANDLE handle, const std::wstring& target);
HMODULE GetProcessBaseAddressX64(DWORD processID);
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
MODULEENTRY32 GetModuleInfo(std::uint32_t ProcessID, const wchar_t* ModuleName);