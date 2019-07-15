// ctags.cpp
#include "pch.h"
#include "ctags.h"
#include "utils.h"
#include "str_utils.h"
#include "jobpartitioner.h"

static std::atomic<int> generator = 0;

ctags::ctags(const char * root, const char * output, int tc) : rootPath(root), outputPath(output), threadCount(tc)
{
}

void
ctags::install()
{
	printf("[ctags] installing...\n");

	const char * ctf = "tags";
	if (_access(ctf, 0) == -1)
	{
		char curdir[_MAX_PATH];
		strcpy(curdir, rootPath);

		char targetdir[_MAX_PATH];
		strcpy(targetdir, rootPath);
		//strcat(targetdir, "\\Game");

		std::vector<std::string> filelist;
		file_search(rootPath, curdir, filelist, [&](std::string&& fpath, std::string&& fname)
		{
			return fpath.find(targetdir) != std::string::npos
				&& (fname.rfind(".h") != std::string::npos || 
					fname.rfind(".cpp") != std::string::npos || 
					fname.rfind(".hpp") != std::string::npos);
		});

		if (_access("filelist.txt", 0) == -1)
		{
			std::ofstream filelistf("filelist.txt", std::ios_base::out);
			for (auto iter = filelist.begin(); iter != filelist.end(); iter++)
			{
				filelistf << (*iter) << std::endl;
			}
			filelistf.close();
		}

		// <options>
		//	--fields=+iaS
		//  a   Access (or export) of class members
		//	i   Inheritance information
		//	S   Signature of routine(e.g.prototype or parameter list)
		//
		//	--c++-kinds=+p
		//	헤더파일에도 정의가 보이게 해주는 옵션
		//
		//	--extras=+q
		//	태그에 이름공간이 같이 표시되게 해준다.
		open_process("ctags -R --c++-kinds=+p --fields=+aS --language-force=C++ --extras=+q -L filelist.txt", true);
		remove("filelist.txt");
	}

	printf("[ctags] installed!\n");
}

void 
ctags::uninstall()
{
}

void 
ctags::flush()
{
	// read
	const char * ctf = "tags";
	std::ifstream infile(ctf);
	if (!infile.good())
		return;

	std::vector<std::string> linelist;
	while (!infile.eof())
	{
		std::string line;
		if (!std::getline(infile, line))
			continue;

		linelist.emplace_back(std::move(line));
	}

	jobpartitioner jp(threadCount, linelist.size());
	parallel(threadCount, [&](int uti)
	{
		std::pair<int, int> job;
		while (jp.get_job(job))
		{
			for (int i = job.first; i != job.second; i++)
			{
				std::string line = linelist[i];
				std::vector<token> tvec;
				int ps = 0;	// 0: tag / 1: path / 2: regex / 3: type / 4: hierarchy
				for (int i = 0, j = 0; i != line.size(); i++)
				{
					if ((ps == 0 && line[i] == '!') ||
						(ps == 0 && line[i] == '$'))
					{
						break;
					}

					switch (ps)
					{
					case 0:
					case 1:
					case 3:
					case 4:
					{
						if (line[i] == '\t')
						{
							tvec.emplace_back(token(j, i - j));
							j = i + 1;
							ps++;
						}
						break;
					}
					case 2:
					{
						if (line[i] == '\t')
						{
							if (line[i - 1] == '"' && line[i - 2] == ';')
							{
								tvec.emplace_back(token(j, i - j));
								j = i + 1;
								ps++;
							}
						}
						break;
					}
					}

					if (i >= line.size() - 1)
					{
						tvec.emplace_back(token(j, i - j + 1));
						_insert(line, tvec);
						break;
					}
				}
			}
		}
	});

	infile.close();

	// write
	change_dir(outputPath);

	std::ofstream output("symbols.txt", std::ios_base::out);
	for (auto iter = symbols.begin(); iter != symbols.end(); iter++ )
	{
		output << (*iter).tag << std::endl;
	}

	output.close();
}

void
ctags::_insert(const std::string & line, const std::vector<token> & tvec)
{
	if (tvec.size() < 4)
		return;

	// make symbol
	symbol s;
	s.uid = generator++;
	s.tag = std::move(tvec[0].to_string(line));
	s.type = tvec[3].to_string(line)[0];
	s.is_public = true;
	if (tvec.size() > 4)
	{
		for (int ti = 5; ti < tvec.size(); ti++)
		{
			const auto & tstr = tvec[ti].to_string(line);
			if (tstr.find("access:") != std::string::npos)
			{
				s.is_public = tstr.find("public") != std::string::npos;
			}
		}
	}

	// validate
	for (int i = 0; i != predictators.size(); i++)
	{
		if (!predictators[i](s))
			return;
	}

	lock.lock();
	symbols.emplace_back(std::move(s));
	lock.unlock();
}
