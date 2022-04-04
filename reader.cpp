#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>

using namespace std;

void log(string data, DWORD page_number) {
	string name = "C:\\lab4_1\\read_log_" + to_string(page_number) + ".txt";
	ofstream fout(name, ios_base::app);
	if (fout.is_open()) fout << data;
	fout.close();
}

int main()
{
	const int page_size = 4096;
	const int page_count = 4;
	const int n = 1000000;
	HANDLE write_semaphores[page_count];
	HANDLE read_semaphores[page_count];
	HANDLE mtx = OpenMutex(
		MUTEX_MODIFY_STATE | SYNCHRONIZE, //режим доступа
		/*изменение значения функцией ReleaseMutex() и
		использование дескриптора объекта в любой из функций ожидания*/
		FALSE, //флаг наследования
		"IOMutex" //имя
	);
	HANDLE map_file = OpenFileMapping( /*открывает именованный 
										объект "проецируемый файл"*/
		FILE_MAP_WRITE/*GENERIC_READ*/, //режим доступа
		FALSE, //флаг наследования
		"MAPPING" //имя
	);
	LPVOID file_view = MapViewOfFile( /*отображает представление 
	проецируемого файла в адресное пространство вызывающего процесса*/
		map_file, //декср. проецируемого файла
		FILE_MAP_READ, //режим доступа (операции чтения-записи)
		0, //старшее DWORD смещения файла
		0, //младшее DWORD смещения файла
		page_size * page_count //кол-во байт отображения
	);

	DWORD page = 0;

	for (int i = 0; i < page_count; i++) {
		write_semaphores[i] = OpenSemaphore(
			SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, //режим доступа
			/*изменение значения функцией ReleaseSemaphore() и
			использование дескриптора объекта в любой из функций ожидания*/
			FALSE, //флаг наследования
			("write_semaphore_" + to_string(i)).c_str() //имя
		);
		read_semaphores[i] = OpenSemaphore(
			SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, //режим доступа
			/*изменение значения функцией ReleaseSemaphore() и
			использование дескриптора объекта в любой из функций ожидания*/
			FALSE, //флаг наследования
			("read_semaphore_" + to_string(i)).c_str() //имя
		);
	}

	VirtualLock( //Блокировка страниц буферной памяти в оперативной памяти
		file_view, //указатель на начало области страниц для блокировки
		page_size * page_count //размер области
	);

	for (int i = 0; i < 2; i++) {
		page = WaitForMultipleObjects(
			page_count, //кол-во интересующих объектов
			read_semaphores, //указатель на массив дескрипторов объектов 
			FALSE, //режим ожидание (любой первый)
			INFINITE //время ожидания
		);
		log(
			"take semaphore: " + to_string(GetTickCount() % n) + "\n",
			page
		);
		WaitForSingleObject(
			mtx, //объект ядра
			INFINITE //время ожидания
		);
		log(
			"take mutex: " + to_string(GetTickCount() % n) + "\n",
			page
		);
		Sleep(500);
		log(
			"read page " + to_string(page) + ": " + to_string(GetTickCount() % n) + "\n",
			page
		);
		ReleaseMutex(mtx); //освобождение мютекса
		log(
			"free mutex: " + to_string(GetTickCount() % n) + "\n",
			page
		);
		ReleaseSemaphore(
			write_semaphores[page], //дескр. семафора
			1, //значение
			NULL //переменная для записи пред.значения
		);
		log(
			"free semaphore: " + to_string(GetTickCount() % n) + "\n\n",
			page
		);
	}

	CloseHandle(mtx);
	CloseHandle(map_file);
	CloseHandle(file_view);

	return 0;
}