#include <iostream>
#include <Windows.h>
#include <conio.h>

using namespace std;

void WINAPI Callback(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped) {
        cout  << "\nСообщение получено\n";
}

void PrintMenu() {
    cout << "1. Подключиться к каналу\n";
    cout << "2. Получить сообщение\n";
    cout << "3. Отключиться от канала\n";
    cout << "4. Выход\n";
}

bool ConnectToPipe(HANDLE& hPipe) {
    hPipe = CreateFile(
        "\\\\.\\pipe\\myPipe",
        GENERIC_READ,
        NULL,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );
    if (hPipe != INVALID_HANDLE_VALUE)
        return true;
    else {
        cout << GetLastError();
        return false;
    }
}

bool GetNewMessage(HANDLE& hPipe, char message[]) {
    OVERLAPPED lpOverlapped = OVERLAPPED();
    bool isMesssageGet = ReadFileEx(
        hPipe, 
        message, 
        512, 
        &lpOverlapped,
        Callback
    );
    SleepEx(
        INFINITE, //время блокировки
        true //опция досрочного отключения
    );
    return isMesssageGet;
}

int main() {
    system("chcp 1251");

    char menu_point;
    bool isConnected = false;
    HANDLE pipeHandle = nullptr;
    char message[512];

    do {
        system("cls");
        PrintMenu();
        menu_point = _getch();
        switch (menu_point)
        {
        case '1':
            if (isConnected) {
                cout << "\nПодключение уже выполнено";
                /*CloseHandle(pipeHandle);*/
            }
            else {
                isConnected = ConnectToPipe(pipeHandle);
                if (!isConnected)
                    cout << "\nПодключение не выполнено\n";
                else cout << "\nПодключение выполнено\n";
            }
            system("pause");
            break;
        case '2':
            if (!isConnected)
                cout << "\nПодключение еще не выполнено\n";
            else if (!GetNewMessage(pipeHandle, message))
                cout << "\nСообщение не получено\n";
            else cout << endl << message << endl;
            system("pause");
            break;
        case '3':
            if (!isConnected)
                cout << "\nКанал не был подключен\n";
            else if (!CloseHandle(pipeHandle)) 
                cout << "\nОтключение не выполнено\n";
            else {
                cout << "\nОтключение выполнено\n";
                isConnected = false;
            }
            system("pause");
            break;
        case '4':
            break;
        default:
            cout << "\nТакого пункта не существует\n";
            system("pause");
            break;
        }
    } while (menu_point != '4');

    if (isConnected) CloseHandle(pipeHandle);
    system("cls");

    return 0;
}
