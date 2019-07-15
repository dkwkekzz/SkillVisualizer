#ifndef __SYMBOLSDLL__
#define __SYMBOLSDLL__
#include <vector>


#ifdef SYMBOLS_EXPORTS
#define MYEXPORT __declspec( dllexport )
#else
#define MYEXPORT __declspec( dllimport )
#endif

//
// export
//

extern"C" void MYEXPORT ReadTable(const std::wstring& wszFilename, const void* base, std::vector<std::pair<std::wstring, void*> >& symbols);

//
// static
//

bool ReadLines(const wchar_t* fileName, std::vector<std::wstring>& outlist);
bool Vaild(const std::wstring& src, const std::vector<std::wstring>& includes, const std::vector<std::wstring>& excludes);

#endif
