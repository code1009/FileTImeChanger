#include <iostream>
#include <filesystem>
#include <string>

#include <Windows.h>

class FileTimeInfo
{
public:
	std::wstring path;
	FILETIME creationTime;
	FILETIME lastWriteTime;
	FILETIME lastAccessTime;
};

bool setFileTime(FileTimeInfo* fti)
{
	HANDLE hHandle;


	hHandle = CreateFileW(fti->path.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	
	
	bool result;


	result = true;


	BOOL rv;


	rv = SetFileTime(hHandle, &fti->creationTime, &fti->lastAccessTime, &fti->lastWriteTime);
	if (FALSE == rv)
	{
		result = false;
	}

	rv = CloseHandle(hHandle);


	return result;
}

bool getFileTime(FileTimeInfo* fti)
{
	HANDLE hHandle;


	hHandle = CreateFileW(fti->path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}


	bool result;


	result = true;


	BOOL rv;


	rv = GetFileTime(hHandle, &fti->creationTime, &fti->lastAccessTime, &fti->lastWriteTime);
	if (FALSE == rv)
	{
		result = false;
	}

	rv = CloseHandle(hHandle);


	return result;
}

void enum_directory (std::filesystem::path _directory_path)
{
	//------------------------------------------------------------------------
	std::string _directory_path_string;
	
	
	_directory_path_string = _directory_path.generic_string();


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


			enum_directory(_sub_directory_path);
		}
		else
		{
			std::cout << "file = " << _directory_entry.path() << std::endl;


			std::filesystem::path _directory_file_path;


			_directory_file_path = _directory_entry.path();


			_directory_path_string = _directory_file_path.generic_string();
		}


		it++;
	}

}

int main()
{

	//------------------------------------------------------------------------
	std::filesystem::path _current_path;

	
	_current_path = std::filesystem::current_path();
	enum_directory(_current_path);

	return 0;
}
