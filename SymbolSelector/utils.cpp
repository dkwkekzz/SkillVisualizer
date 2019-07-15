// utils.cpp
#include "pch.h"
#include "utils.h"

//
// process
//

void
open_process(const char * cmd, bool wait)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	DWORD op = 0;
	if (!wait)
		op |= CREATE_NEW_PROCESS_GROUP;

	if (!CreateProcess(NULL, (LPWSTR)cmd, NULL, NULL, FALSE, op, NULL, NULL, &si, &pi))
	{
		printf("[ctags] CreateProcess failed (%d)\n", GetLastError());
		return;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

void
change_dir(const char* targetdir)
{
	if (_chdir(targetdir))
	{
		switch (errno)
		{
		case ENOENT:
			printf("Unable to locate the directory: %s\n", targetdir);
			break;
		case EINVAL:
			printf("Invalid buffer.\n");
			break;
		default:
			printf("Unknown error.\n");
		}
	}
}

bool
is_file_exist(const char *fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

int
isFileOrDir(_finddata_t& fd)
{
	if (fd.attrib & _A_SUBDIR)
		return 0; // 디렉토리면 0 반환
	else
		return 1; // 그밖의 경우는 "존재하는 파일"이기에 1 반환
}

//
// string
//

char*
get_ext(const char* fileName)
{
	int file_name_len = strlen(fileName);
	fileName += file_name_len;

	char *file_ext = nullptr;
	for (int i = 0; i < file_name_len; i++)
	{
		if (*fileName == '.')
		{
			file_ext = const_cast<char*>(fileName + 1);
			break;
		}
		fileName--;
	}
	return file_ext;
}
