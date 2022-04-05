#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <namedpipeapi.h>

using namespace std;

void WINAPI Callback(
    DWORD dwErrorCode, 
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped) {
    cout << "\nСообщение отправлено\n";
}

void PrintMenu() {
    cout << "1. Создать канал\n";
    cout << "2. Отправить сообщение\n";
    cout << "3. Закрыть канал\n";
    cout << "4. Выход\n";
}

bool ConnectToPipe(HANDLE& hPipe) {
    hPipe = CreateNamedPipeA(
        "\\\\.\\pipe\\myPipe",
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE |
        PIPE_READMODE_MESSAGE,
        PIPE_UNLIMITED_INSTANCES,
        512,
        512,
        0,
        NULL
    );
    if (hPipe != INVALID_HANDLE_VALUE)
        return true;
    else {
        cout << GetLastError();
        return false;
    }
}

bool SendNewMessage(HANDLE& hPipe, char message[]) {
    OVERLAPPED lpOverlapped = OVERLAPPED();
    bool isMessageGet = WriteFileEx(
        hPipe, 
        message, 
        512, 
        &lpOverlapped, 
        Callback
    );
    SleepEx(
        INFINITE,
        true
    );

    return isMessageGet;
}

int main() {
    system("chcp 1251");

    char menu_point;
    bool isConnected = false;
    HANDLE pipeHandle = nullptr;
    char message[512] = {'a', 'b', 'c'};

    do {
        system("cls");
        PrintMenu();
        menu_point = _getch();
        switch (menu_point)
        {
        case '1':
            if (isConnected)
                cout << "\nКанал уже создан";
            else {
                isConnected = ConnectToPipe(pipeHandle);
                if (!isConnected)
                    cout << "\nКанал не создан\n";
                else {
                    cout << "\nКанал создан\n";
                    ConnectNamedPipe(
                        pipeHandle,
                        NULL
                    );
                }
            }
            system("pause");
            break;
        case '2':
            if (!isConnected)
                cout << "\nКанал еще не создан\n"; 
            else if (!SendNewMessage(pipeHandle, message))
                cout << "\nСообщение не отправлено\n";
            else cout << endl << message << endl;  
            system("pause");
            break;
        case '3':
            if (!isConnected)
                cout << "\nКанал не был подключен\n";
            else if (!CloseHandle(pipeHandle)) 
                cout << "\nКанал не закрыт\n";
            else {
                cout << "\nКанал закрыт успешно\n";
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

    return 0;
}
