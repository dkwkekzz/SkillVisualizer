// utils.h
#pragma once
#include "pch.h"

//
// process
//

void open_process(const char * cmd, bool wait);
void change_dir(const char* targetdir);
bool is_file_exist(const char *fileName);
int isFileOrDir(_finddata_t& fd);

template<typename C, typename Func>
void
file_search(const char root[], char file_path[], C& fvec, Func f)
{
	_finddata_t fd;

	intptr_t handle;
	int check = 0;
	char file_path2[_MAX_PATH];

	strcat(file_path, "\\");
	strcpy(file_path2, file_path);
	strcat(file_path, "*");

	if ((handle = _findfirst(file_path, &fd)) == -1)
	{
		//printf("No such file or directory\n");
		return;
	}

	while (_findnext(handle, &fd) == 0)
	{
		char file_pt[_MAX_PATH];
		strcpy(file_pt, file_path2);
		strcat(file_pt, fd.name);

		check = isFileOrDir(fd);
		check &= f(std::string(file_pt), std::string(fd.name));

		if (check == 0 && fd.name[0] != '.')
		{
			file_search(root, file_pt, fvec, f);    //하위 디렉토리 검색 재귀함수
		}
		else if (check == 1 && fd.size != 0 && fd.name[0] != '.')
		{
			//printf("파일명 : %s, 크기:%d\n", file_pt, fd.size);
			std::string fstr = file_pt;
			int rootlen = strlen(root) + 1;
			fvec.emplace_back(fstr.substr(rootlen, fstr.size() - rootlen));
		}
	}
	_findclose(handle);
}

template<typename Pred, typename Act>
void
file_foreach(const char root[], char file_path[], Pred pdt, Act act)
{
	_finddata_t fd;

	intptr_t handle;
	int check = 0;
	char file_path2[_MAX_PATH];

	strcat(file_path, "\\");
	strcpy(file_path2, file_path);
	strcat(file_path, "*");

	if ((handle = _findfirst(file_path, &fd)) == -1)
	{
		return;
	}

	while (_findnext(handle, &fd) == 0)
	{
		char file_pt[_MAX_PATH];
		strcpy(file_pt, file_path2);
		strcat(file_pt, fd.name);

		check = isFileOrDir(fd);
		check &= pdt(file_pt, fd.name);

		if (check == 0 && fd.name[0] != '.')
		{
			file_foreach(root, file_pt, pdt, act);    //하위 디렉토리 검색 재귀함수
		}
		else if (check == 1 && fd.size != 0 && fd.name[0] != '.')
		{
			act(file_path2, fd.name);
		}
	}
	_findclose(handle);
}

template<typename F>
void
parallel(int tc, F action)
{
	if (tc == 1)
	{
		action(0);
		return;
	}

	std::thread trs[8];
	for (int ti = 0; ti != tc; ti++)
	{
		int uti = ti;
		trs[ti] = std::thread([&, uti]()
		{
			action(uti);
		});
	}

	for (int ti = 0; ti != tc; ti++)
	{
		trs[ti].join();
	}
}

//
// string
//

char* get_ext(const char* fileName);