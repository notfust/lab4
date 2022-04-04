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
		MUTEX_MODIFY_STATE | SYNCHRONIZE, //����� �������
		/*��������� �������� �������� ReleaseMutex() �
		������������� ����������� ������� � ����� �� ������� ��������*/
		FALSE, //���� ������������
		"IOMutex" //���
	);
	HANDLE map_file = OpenFileMapping( /*��������� ����������� 
										������ "������������ ����"*/
		FILE_MAP_WRITE/*GENERIC_READ*/, //����� �������
		FALSE, //���� ������������
		"MAPPING" //���
	);
	LPVOID file_view = MapViewOfFile( /*���������� ������������� 
	������������� ����� � �������� ������������ ����������� ��������*/
		map_file, //�����. ������������� �����
		FILE_MAP_READ, //����� ������� (�������� ������-������)
		0, //������� DWORD �������� �����
		0, //������� DWORD �������� �����
		page_size * page_count //���-�� ���� �����������
	);

	DWORD page = 0;

	for (int i = 0; i < page_count; i++) {
		write_semaphores[i] = OpenSemaphore(
			SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, //����� �������
			/*��������� �������� �������� ReleaseSemaphore() �
			������������� ����������� ������� � ����� �� ������� ��������*/
			FALSE, //���� ������������
			("write_semaphore_" + to_string(i)).c_str() //���
		);
		read_semaphores[i] = OpenSemaphore(
			SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, //����� �������
			/*��������� �������� �������� ReleaseSemaphore() �
			������������� ����������� ������� � ����� �� ������� ��������*/
			FALSE, //���� ������������
			("read_semaphore_" + to_string(i)).c_str() //���
		);
	}

	VirtualLock( //���������� ������� �������� ������ � ����������� ������
		file_view, //��������� �� ������ ������� ������� ��� ����������
		page_size * page_count //������ �������
	);

	for (int i = 0; i < 2; i++) {
		page = WaitForMultipleObjects(
			page_count, //���-�� ������������ ��������
			read_semaphores, //��������� �� ������ ������������ �������� 
			FALSE, //����� �������� (����� ������)
			INFINITE //����� ��������
		);
		log(
			"take semaphore: " + to_string(GetTickCount() % n) + "\n",
			page
		);
		WaitForSingleObject(
			mtx, //������ ����
			INFINITE //����� ��������
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
		ReleaseMutex(mtx); //������������ �������
		log(
			"free mutex: " + to_string(GetTickCount() % n) + "\n",
			page
		);
		ReleaseSemaphore(
			write_semaphores[page], //�����. ��������
			1, //��������
			NULL //���������� ��� ������ ����.��������
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