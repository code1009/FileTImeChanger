#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

#include <Windows.h>
#include <strsafe.h>

void errorReport(const std::wstring& message)
{
	//-----------------------------------------------------------------------
	DWORD dwLastErrorCode;
	DWORD dwSystemLocale;
	DWORD dwFlags;
	HLOCAL hLocal;
	DWORD dwLength;


	dwLastErrorCode = GetLastError();
	dwSystemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
	dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	hLocal = NULL;
	dwLength = FormatMessageW(dwFlags, NULL, dwLastErrorCode, dwSystemLocale, (LPWSTR)&hLocal, 0, NULL);


	//-----------------------------------------------------------------------
	if (!dwLength)
	{
		ExitProcess(dwLastErrorCode);
		return;
	}


	//-----------------------------------------------------------------------
	std::wstring errorString = ((const WCHAR*)hLocal);


	//-----------------------------------------------------------------------
	WCHAR* errorMessageString;
	std::size_t errorMessageStringLength;


	errorMessageStringLength = message.size() + errorString.size() + 40;
	errorMessageString = new WCHAR[errorMessageStringLength];

	StringCchPrintfW(errorMessageString,
		errorMessageStringLength,
		L"%s failed with error %d: %s\r\n",
		message.c_str(),
		dwLastErrorCode,
		errorString.c_str()
	);
	OutputDebugStringW(errorMessageString);
	//MessageBoxW(NULL, errorMessageString, L"Error", MB_ICONEXCLAMATION | MB_OK);

	delete[] errorMessageString;


	//-----------------------------------------------------------------------
	LocalFree(hLocal);


	//-----------------------------------------------------------------------
//	ExitProcess(dwLastErrorCode);
}

class Element
{
public:
	std::wstring _path;
	bool _dir;

	FILETIME _creationFileTime;
	FILETIME _lastWriteFileTime;
	FILETIME _lastAccessFileTime;

	SYSTEMTIME _creationSystemTime;
	SYSTEMTIME _lastWriteSystemTime;
	SYSTEMTIME _lastAccessSystemTime;

public:
	void fileTimeToSystemTime(void)
	{
		FileTimeToSystemTime(&_creationFileTime, &_creationSystemTime);
		FileTimeToSystemTime(&_lastWriteFileTime, &_lastWriteSystemTime);
		FileTimeToSystemTime(&_lastAccessFileTime, &_lastAccessSystemTime);
	}

	void systemTimeToFileTime(void)
	{
		SystemTimeToFileTime(&_creationSystemTime, &_creationFileTime);
		SystemTimeToFileTime(&_lastWriteSystemTime, &_lastWriteFileTime);
		SystemTimeToFileTime(&_lastAccessSystemTime, &_lastAccessFileTime);
	}

	bool setFileTime(void)
	{
		//--------------------------------------------------------------------
		systemTimeToFileTime();


		//--------------------------------------------------------------------
		HANDLE hHandle;


		hHandle = CreateFileW(_path.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}


		//--------------------------------------------------------------------
		bool result;


		result = true;


		//--------------------------------------------------------------------
		BOOL rv;


		rv = SetFileTime(hHandle, &_creationFileTime, &_lastAccessFileTime, &_lastWriteFileTime);
		if (FALSE == rv)
		{
			result = false;
		}


		//--------------------------------------------------------------------
		rv = CloseHandle(hHandle);


		return result;
	}

	bool getFileTime(void)
	{
		//--------------------------------------------------------------------
		fileTimeToSystemTime();


		//--------------------------------------------------------------------
		HANDLE hHandle;


		hHandle = CreateFileW(_path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}


		//--------------------------------------------------------------------
		bool result;


		result = true;


		//--------------------------------------------------------------------
		BOOL rv;


		rv = GetFileTime(hHandle, &_creationFileTime, &_lastAccessFileTime, &_lastWriteFileTime);
		if (FALSE == rv)
		{
			result = false;
		}


		//--------------------------------------------------------------------
		rv = CloseHandle(hHandle);


		return result;
	}

	bool getDirTime(void)
	{
		//--------------------------------------------------------------------
		fileTimeToSystemTime();


		//--------------------------------------------------------------------
		HANDLE hHandle;
		WIN32_FIND_DATAW data;
		

		hHandle = FindFirstFileW(_path.c_str(), &data);
		if (hHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}


		//--------------------------------------------------------------------
		memcpy(&_creationFileTime, &data.ftCreationTime, sizeof(_creationFileTime));
		memcpy(&_lastWriteFileTime,  &data.ftLastAccessTime, sizeof(_lastWriteFileTime));
		memcpy(&_lastAccessFileTime, &data.ftLastWriteTime, sizeof(_lastAccessFileTime));


		//--------------------------------------------------------------------
		BOOL rv;


		//--------------------------------------------------------------------
		rv = FindClose(hHandle);


		return true;
	}

	bool setDirTime(void)
	{
		//--------------------------------------------------------------------
		systemTimeToFileTime();


		//--------------------------------------------------------------------
#if 0
		HANDLE hHandle;
		WIN32_FIND_DATAW data;


		hHandle = FindFirstFileW(_path.c_str(), &data);
		if (hHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}
#endif


		//--------------------------------------------------------------------
		HANDLE hHandle;

		
		hHandle = CreateFileW(_path.c_str(), 
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NORMAL | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | FILE_FLAG_BACKUP_SEMANTICS,
			NULL
		);
		if (hHandle == INVALID_HANDLE_VALUE)
		{
			errorReport(L"CreateFileW()");
			return false;
		}


		//--------------------------------------------------------------------
		bool result;


		result = true;


		//--------------------------------------------------------------------
		BOOL rv;


		rv = SetFileTime(hHandle, &_creationFileTime, &_lastAccessFileTime, &_lastWriteFileTime);
		if (FALSE == rv)
		{
			errorReport(L"SetFileTime()");
			result = false;
		}


		//--------------------------------------------------------------------
#if 0
		rv = FindClose(hHandle);
#endif


		//--------------------------------------------------------------------
		rv = CloseHandle(hHandle);


		return result;
	}

	bool getTime(void)
	{
		bool rv;


		if (_dir)
		{
			rv = getDirTime();
		}
		else
		{
			rv = getFileTime();
		}

		return rv;
	}

	bool setTime(void)
	{
		bool rv;


		if (_dir)
		{
			rv = setDirTime();
		}
		else
		{
			rv = setFileTime();
		}


		return rv;
	}

	void changeTime(void)
	{
		//------------------------------------------------------------------------
		if (!getTime())
		{
			std::wcout << _path << L" getTime() failed" << std::endl;
			return;
		}


		//------------------------------------------------------------------------
		GetSystemTime(&_creationSystemTime);
		GetSystemTime(&_lastWriteSystemTime);
		GetSystemTime(&_lastAccessSystemTime);


		//------------------------------------------------------------------------
		if (!setTime())
		{
			std::wcout << _path << L" setTime() failed" << std::endl;
			return;
		}


		//------------------------------------------------------------------------
		std::wcout << _path << L" changeTime() ok" << std::endl;
	}
};

void push_Element(std::vector<Element*>& collection, std::wstring path, bool dir)
{
	Element* o = new Element();


	o->_path = path;
	o->_dir = dir;

	collection.push_back(o);
}

void enum_directory (std::filesystem::path _directory_path, std::vector<Element*>& collection)
{
	//------------------------------------------------------------------------
	std::wstring _directory_path_string;
	
	
	_directory_path_string = _directory_path.generic_wstring();
	push_Element(collection, _directory_path_string, true);


	//------------------------------------------------------------------------
	std::cout << "dir  = " << _directory_path << std::endl;


	//------------------------------------------------------------------------
	std::filesystem::directory_iterator it(_directory_path);


	while (it != std::filesystem::end(it))
	{
		const std::filesystem::directory_entry& _directory_entry = *it;


		if (_directory_entry.is_directory())
		{
			std::filesystem::path _sub_directory_path;


			_sub_directory_path = _directory_entry.path();


			enum_directory(_sub_directory_path, collection);
		}
		else
		{
			std::cout << "file = " << _directory_entry.path() << std::endl;


			std::filesystem::path _directory_file_path;


			_directory_file_path = _directory_entry.path();


			_directory_path_string = _directory_file_path.generic_wstring();
			push_Element(collection, _directory_path_string, false);
		}


		it++;
	}
}

int main()
{
	//------------------------------------------------------------------------
	//setlocale(LC_ALL, "");
	std::wcout.imbue(std::locale(""));


	//------------------------------------------------------------------------
	std::vector<Element*> collection;


	//------------------------------------------------------------------------
	std::filesystem::path _current_path;

	
	_current_path = std::filesystem::current_path();
	_current_path = std::filesystem::path(L"D:/aaa");
	enum_directory(_current_path, collection);


	//------------------------------------------------------------------------
	for (auto& element : collection)
	{
		element->changeTime();
		delete element;
	}


	return 0;
}
