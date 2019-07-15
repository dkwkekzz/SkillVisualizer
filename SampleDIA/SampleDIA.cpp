// SampleDIA.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "SymbolTable.h"

class CCallback :
	public IDiaLoadCallback2
{
public:
	virtual HRESULT STDMETHODCALLTYPE NotifyDebugDir(
		/* [in] */ BOOL fExecutable,
		/* [in] */ DWORD cbData,
		/* [size_is][in] */ BYTE *pbData) {
		//std::wcout << fExecutable << '\n';
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE NotifyOpenDBG(
		/* [in] */ LPCOLESTR dbgPath,
		/* [in] */ HRESULT resultCode) {
		//std::wcout << dbgPath << ' ' << resultCode << '\n';
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE NotifyOpenPDB(
		/* [in] */ LPCOLESTR pdbPath,
		/* [in] */ HRESULT resultCode) {
		OutputDebugStringW(L"[PDB] ");
		OutputDebugStringW(pdbPath);
		std::wostringstream os;
		os << std::hex << (DWORD)resultCode;
		OutputDebugStringW(os.str().c_str());
		//std::wcout << pdbPath << ' ' << resultCode << '\n';
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE RestrictRegistryAccess(void)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE RestrictSymbolServerAccess(void)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID rid, _Outptr_ void **ppUnk)
	{
		if (ppUnk == NULL) {
			return E_INVALIDARG;
		}
		if (rid == __uuidof(IDiaLoadCallback2))
			*ppUnk = (IDiaLoadCallback2 *)this;
		else if (rid == __uuidof(IDiaLoadCallback))
			*ppUnk = (IDiaLoadCallback *)this;
		else if (rid == __uuidof(IUnknown))
			*ppUnk = (IUnknown *)this;
		else
			*ppUnk = NULL;
		if (*ppUnk != NULL) {
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}
	ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}

	virtual HRESULT STDMETHODCALLTYPE RestrictOriginalPathAccess(void) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE RestrictReferencePathAccess(void) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE RestrictDBGAccess(void) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE RestrictSystemRootAccess(void) { return S_OK; }
};

void Fatal(const char* msg)
{
	LPVOID lpvMessageBuffer;
	CHAR szPrintBuffer[512];
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpvMessageBuffer, 0, NULL);

	sprintf(szPrintBuffer,
		"[Fatal] %s\n   error code = %d\n   message    = %s\n",
		msg, GetLastError(), (wchar_t*)lpvMessageBuffer);

	MessageBoxA(nullptr, szPrintBuffer, "Error", MB_OK);
}

std::string workingdir()
{
	char buf[256];
	GetCurrentDirectoryA(256, buf);
	return std::string(buf) + '\\';
}

int main()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	std::wstring wszFilename = L"SimpleGame.exe";
	SymbolTable st(wszFilename);
	st.EnumerateAll();
	st.EnumerateAllByAddr();

	//// constructor
	//CComPtr<IDiaDataSource> pSource;
	//HRESULT hr = CoCreateInstance(_uuidof(DiaSource),
	//	NULL,
	//	CLSCTX_INPROC_SERVER,
	//	__uuidof(IDiaDataSource),
	//	(void **)&pSource);
	//
	//if (FAILED(hr))
	//{
	//	Fatal("Could not CoCreate CLSID_DiaSource. Register msdia80.dll.");
	//}
	//
	//CCallback callback;
	//if (FAILED(pSource->loadDataFromPdb(wszFilename.c_str())))
	//{
	//	if (FAILED(pSource->loadDataForExe(wszFilename.c_str(), L"srv*.wdd\\_symbols*https://msdl.microsoft.com/download/symbols", &callback)))
	//	{
	//		Fatal("loadDataFromPdb/Exe");
	//	}
	//}
	//
	//CComPtr<IDiaSession> psession;
	//if (FAILED(pSource->openSession(&psession)))
	//{
	//	Fatal("openSession");
	//}
	//
	//CComPtr<IDiaEnumTables> pTables;
	//if (FAILED(psession->getEnumTables(&pTables)))
	//{
	//	Fatal("getEnumTables");
	//}
	//
	//CComPtr<IDiaSymbol> pglobal;
	//if (FAILED(psession->get_globalScope(&pglobal)))
	//{
	//	Fatal("get_globalScope");
	//}
	//
	//CComPtr< IDiaTable > pTable;
	//ULONG celt;
	//while (SUCCEEDED(hr = pTables->Next(1, &pTable, &celt)) && celt == 1)
	//{
	//	// Do something with each IDiaTable.
	//}

	return 0;
}