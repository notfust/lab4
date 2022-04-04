#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>

using namespace std;

HANDLE CreateNewProcess(const string& exe_name) {
	STARTUPINFO startup_info; 
	PROCESS_INFORMATION process_information;

	ZeroMemory( //заполняет блок памяти нулями
		&startup_info, //указатель на блок памяти
		sizeof(startup_info) //размер блока памяти
	);
	startup_info.cb = sizeof(startup_info); //устанавливает размер структуры (байты)

	ZeroMemory( //заполняет блок памяти нулями
		&process_information, //указатель на блок памяти
		sizeof(process_information) //размер блока памяти
	);

	if (CreateProcess(
		exe_name.c_str(), //имя исполняемого модуля
		NULL, //имя с командной строки
		NULL, //дескриптор безопасности (процесс)
		NULL, //дескр. безоп. (поток)
		TRUE, //дескриптор парам. наследования
		NULL, //флаг создания
		NULL, //конф. процесса (конф. вызывающего процесса)
		NULL, //имя каталога
		&startup_info, //инфо предустановки
		&process_information //инфо о процессе
	)) return process_information.hProcess;
	
	return nullptr;
}

int main()
{
	cout << "start" << endl; //уведомление о начале работы
	
	const int page_size = 4096; //размер страницы
	const int page_count = 4; //число страниц
	const int process_count = 8; //число процессов

	HANDLE write_semaphores[page_count];//массив семафоров писателя
	HANDLE read_semaphores[page_count];//массив семафоров читателя
	HANDLE mtx = CreateMutex(
		NULL, //атрибуты защиты
		false, //начальное состояние
		"IOMutex" //имя объекта
	);
	HANDLE file_handle = CreateFile(
		"C:\\lab4_1\\text.txt", //имя объекта
		GENERIC_READ | GENERIC_WRITE, //режим доступа
		0, //совместный доступ
		NULL, //дескр. защиты
		CREATE_ALWAYS, //как действовать
		FILE_ATTRIBUTE_NORMAL, //атрибуты файла
		NULL //дескр. шаблона файла
	);
	HANDLE map_file = CreateFileMapping(
		file_handle, //дескриптор файла
		NULL, //защита
		PAGE_READWRITE, //атрибуты защиты
		0, //число старшего порядка максимального объекта
		page_size * page_count, //число младшего порядка максимального объекта
		"MAPPING" //имя объекта
	);
	HANDLE process_handles[process_count]{};

	for (int i = 0; i < page_count; i++) {
		write_semaphores[i] = CreateSemaphore(
			NULL, //аттрибуты защиты
			1, //начальное значение счетчика
			1, //максимальное значение счетчика
			("write_semaphore_" + to_string(i)).c_str() //имя 
		);
		read_semaphores[i] = CreateSemaphore(
			NULL, //аттрибуты защиты
			0, //начальное значение счетчика
			1, //максимальное значение счетчика
			("read_semaphore_" + to_string(i)).c_str() //имя 
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
		process_count, //кол-во объектов
		process_handles, //указатель на массив дескрипторов объектов
		true, //режим ожидания (ожидание всех объектов)
		INFINITE //время ожидания
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