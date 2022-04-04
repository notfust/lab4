#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>

using namespace std;

HANDLE CreateNewProcess(const string& exe_name) {
	STARTUPINFO startup_info; 
	PROCESS_INFORMATION process_information;

	ZeroMemory( 
		&startup_info, 
		sizeof(startup_info)
	);
	startup_info.cb = sizeof(startup_info);

	ZeroMemory( 
		&process_information,
		sizeof(process_information)
	);

	if (CreateProcess(
		exe_name.c_str(),
		NULL, 
		NULL, 
		NULL, 
		TRUE, 
		NULL, 
		NULL, 
		NULL, 
		&startup_info, 
		&process_information 
	)) return process_information.hProcess;
	
	return nullptr;
}

int main()
{
	cout << "start" << endl; 
	
	const int page_size = 4096;
	const int page_count = 4; 
	const int process_count = 8;

	HANDLE write_semaphores[page_count];
	HANDLE read_semaphores[page_count];
	HANDLE mtx = CreateMutex(
		NULL,
		false,
		"IOMutex"
	);
	HANDLE file_handle = CreateFile(
		"C:\\lab4_1\\text.txt", 
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	HANDLE map_file = CreateFileMapping(
		file_handle, 
		NULL, 
		PAGE_READWRITE, 
		0, 
		page_size * page_count, 
		"MAPPING" 
	);
	HANDLE process_handles[process_count]{};

	for (int i = 0; i < page_count; i++) {
		write_semaphores[i] = CreateSemaphore(
			NULL, 
			1, 
			1, 
			("write_semaphore_" + to_string(i)).c_str() 
		);
		read_semaphores[i] = CreateSemaphore(
			NULL, 
			0, 
			1,
			("read_semaphore_" + to_string(i)).c_str() 
		);
	}

	for (int i = 0; i < process_count / 2; i++) {
		process_handles[i] = CreateNewProcess(
			"C:\\Users\\book\\source\\repos\\4reader\\x64\\Release\\4reader.exe"
		);
		process_handles[i + process_count / 2] = CreateNewProcess(
			"C:\\Users\\book\\source\\repos\\4writer\\x64\\Release\\4writer.exe"
		);
	}

	cout << "work" << endl;
		
	WaitForMultipleObjects(
		process_count,
		process_handles, 
		true, 
		INFINITE 
	);

	cout << "end" << endl;

	CloseHandle(mtx);
	CloseHandle(map_file);
	CloseHandle(file_handle);
	for (int i = 0; i < page_count; i++) {
		CloseHandle(write_semaphores[i]);
		CloseHandle(read_semaphores[i]);
	}

	return 0;
}
