#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <strsafe.h> 
#include <Psapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <Wincrypt.h>
#include "strsafe.h"


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment(lib, "crypt32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5222"

SOCKET ConnectSocket = INVALID_SOCKET;

#define IDC_LIST_INCOMING 2001
#define IDC_LIST_CHAT 2002
#define IDC_SEND 2003
#define IDC_NEWRECIPIENT 2004
#define IDC_NEW 2005
#define IDC_INCOMING_TEXTS 2006
#define IDC_LOGIN1 2007
#define IDC_LOGINTXT 2008
#define IDC_LOGINPWD 2009
#define IDC_LOGIN 2010
#define IDC_LOGINREG 2011
#define IDC_LOGINREG2 2014
#define IDC_RECIPIENT 2012
#define IDC_CAPTCHA 2013

#define HIGH2BIT_MASK (0xc0)
#define HIGH4BIT_MASK (0xf0)
#define HIGH6BIT_MASK (0xfc)
#define LOW2BIT_MASK (0x03)
#define LOW4BIT_MASK (0x0f)
#define LOW6BIT_MASK (0x3f)
#define MSG_SIZE_IN_MB 5
#define BUFFER_SIZE 10240
#define SOCKET_READ_TIMEOUT_SEC 0.5;

// your name: your message. time. a long listbox.
// another listbox for incoming messages.

using namespace std;

LPHANDLE hJob = new HANDLE;
BOOL PreTranslateMessage(LPMSG lpMsg);
HWND hWnd;
BOOL flag;
LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMsg, WPARAM
	wParam, LPARAM lParam);
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void OnDestroy(HWND hwnd);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
LRESULT CALLBACK MyTextWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LPCREATESTRUCT lpCrStr;
INT Yasb = 10, Yscr = 0;
HWND hctl1;
VOID receive(TCHAR *buf);
BOOL incoming = FALSE;
TCHAR in_buf[10000];
void OnIdle(HWND hwnd);
INT FLAG = 0;

INT comp_str(TCHAR *a, TCHAR *b)
{
	INT i = 0;
	INT j = 0;
	while (a[i] != '\0')
	{
		while (a[i + j] == b[j] && (a[i + j] != '\0' && b[j] != '\0'))
			j++;
		if (b[j] == '\0')
			return i;
		i++;
		j = 0;
	}
	return -1;
}

DWORD ThreadProc(LPVOID lpdwThreadParam)
{

	// CreateWindowEx(0, TEXT("Static"), TEXT("one"),
		// WS_CHILD | WS_VISIBLE | WS_BORDER, 10, Yasb - Yscr, 289, 20 + 20, hctl1, NULL, lpCrStr->hInstance, NULL);


	// i should send a message here to be обработан
	for (;;)
	{
		// strcpy(in_buf, "");

		TCHAR buf[1024];
		if (!incoming)
			receive(in_buf);

		if (strlen(in_buf) > 0)
		{
			// format the message.
			// strcpy(in_buf, "one two three");
			INT r = comp_str(in_buf, "message");
			r = r + 1;
			if (FLAG == 0)
			{
				INT beg = comp_str(in_buf, "<body>") + 6;
				INT end = comp_str(in_buf, "</body>");
				if (end - beg > 0)
					{
						TCHAR *buf2 = new TCHAR[end - beg + 1];
						for (int i = 0; i < end - beg + 1; i++)
							buf2[i] = in_buf[beg + i];
						for (int i = 0; i < end - beg + 1; i++)
							in_buf[i] = buf2[i];
						in_buf[end - beg] = '\0';
						incoming = TRUE;
					}
			}
			// login
			if (FLAG == 1)
				if (comp_str(in_buf, "success") > -1)
				{
					HWND hctl = GetDlgItem(hWnd, IDC_LOGIN1);
					ShowWindow(hctl, 0);
					ShowWindow(GetDlgItem(hWnd, IDC_LOGIN), 0);
					ShowWindow(GetDlgItem(hWnd, IDC_LOGINREG), 0);
				}
				else {
					MessageBox(hWnd, "Error signing in", "Error!", 0);
				}
				
			// register
			if (FLAG == 2)
				if (comp_str(in_buf, "success") > -1)
				{
					HWND hctl = GetDlgItem(hWnd, IDC_LOGIN1);
					ShowWindow(hctl, 0);
					ShowWindow(GetDlgItem(hWnd, IDC_LOGIN), 0);
					ShowWindow(GetDlgItem(hWnd, IDC_LOGINREG), 0);
				}
				else {

				}

				INT beg = comp_str(in_buf, "max-age='0' type='image/png'>") + 29;
				if (beg > 29)
				{
					INT end = comp_str(in_buf, "</data>");
					TCHAR *buf2 = new TCHAR[end - beg + 1];
					for (int i = 0; i < end - beg + 1; i++)
						buf2[i] = in_buf[beg + i];
					buf2[end - beg] = '\0';

					// decode from base64, save to a temp file, open the temp file then voila!
					PBYTE buf3 = new BYTE[end - beg + 4];
					DWORD ccOut = end - beg + 4;
					CryptStringToBinary(buf2, end - beg, 1, buf3, &ccOut, NULL, NULL);
					HANDLE f = CreateFile("temp.png", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL);
					WriteFile(f, buf3, ccOut, NULL, NULL);
					CloseHandle(f);
					// iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
					// if (iResult) printf("c: %s\n", sendbuf);

					// strcpy(sendbuf, "</stream:stream>");
					// iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
					// if (iResult) printf("c: %s\n", sendbuf);

					in_buf[0] = '\b';

				}
			
			FLAG = 0;
		}
		// int n = strlen(buf) / 42;
		// n *= 20;
		// HWND hctl = CreateWindowEx(0, TEXT("Static"), buf,
			// WS_CHILD | WS_VISIBLE | WS_BORDER, 10, Yasb - Yscr, 289, 20 + n, hctl1, NULL, lpCrStr->hInstance, NULL);

		// RECT Rect;
		// GetWindowRect(hctl, &Rect);
		// Yasb += (Rect.bottom - Rect.top + 10);
	}
	return 0;
}

VOID receive(TCHAR *buf)
{
	char recvbuf[10000];
	int recvbuflen = 10000;
	strcpy(buf, "");
	int iResult;

	recvbuf[0] = '\0';
	// do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			recvbuf[iResult] = '\0';
			printf("\ns: %s\n", recvbuf);
			strcat(buf, recvbuf);
		}
		else
			printf("\ns: \n\n");

		// buf = "i";

	// } while (iResult > 0);
}

bool init()
{
	WSADATA wsaData;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;

	// Validate the parameters
	// address = "89.16.173.64"; // prosody.im
	char *address = "84.201.146.161"; // jabber.ru
									  // address = "64.233.161.83"; // google
	address = "188.244.33.77"; // creep.im
	address = "160.16.217.191"; // xmpp.jp
								// address = "213.180.193.48"; // lsd-25.ru

								// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		system("pause");
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(address, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		system("pause");
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			system("pause");
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		// system("pause");
		return 1;
	}

	char *sendbuf = "<?xml version='1.0'?> <stream:stream to='xmpp.jp' xmlns='jabber:client' "
		"xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>";
	char recvbuf[DEFAULT_BUFLEN + 1];
	int recvbuflen = DEFAULT_BUFLEN;

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult) printf("c: %s\n", sendbuf);

	// sendbuf = "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>";
	// iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	// if (iResult) printf("c: %s\n", sendbuf);

	return 0;
}

void login()
{
	char *sendbuf = "";
	TCHAR buf1[4096];
	BYTE *buf;
	int iResult;
	FLAG = 1;
	DWORD num = 2;
	
	HWND hctl1 = GetDlgItem(hWnd, IDC_LOGIN1);
	HWND hctl = GetDlgItem(hctl1, IDC_LOGINPWD);
	GetWindowText(hctl, buf1, 2048);
	num += strlen(buf1); 
	hctl = GetDlgItem(hctl1, IDC_LOGINTXT);
	GetWindowText(hctl, buf1, 2048);
	num += strlen(buf1);

	buf = new BYTE[num];
	buf[0] = (BYTE) '\0';
	for (int i = 0; i < strlen(buf1); i++)
		buf[i + 1] = buf1[i];
	buf[strlen(buf1) + 1] = '\0';
	num = strlen(buf1) + 2;
	hctl = GetDlgItem(hctl1, IDC_LOGINPWD);
	GetWindowText(hctl, buf1, 2048);
	for (int i = 0; i < strlen(buf1); i++)
		buf[i + num] = buf1[i];
	num += strlen(buf1);

 	DWORD nDestinationSize;
	if (CryptBinaryToString(reinterpret_cast<const BYTE*> (buf), num, CRYPT_STRING_BASE64, nullptr, &nDestinationSize))
    {
        LPTSTR pszDestination = static_cast<LPTSTR> (HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE, nDestinationSize * sizeof(TCHAR)));
        if (pszDestination)
        {
            if (CryptBinaryToString(reinterpret_cast<const BYTE*> (buf), num, CRYPT_STRING_BASE64, pszDestination, &nDestinationSize))
            {
				sendbuf = pszDestination;
            }
            // HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pszDestination);
        }
    }
	sendbuf[(int)strlen(sendbuf) - 2] = '\0';
	strcpy(buf1, "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>");
	strcat(buf1, sendbuf);
	strcat(buf1, "</auth>");
	sendbuf = buf1;
	// sendbuf = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>AG1lb18xMTcAVW11bnlhcndhbmRha2F6aTE=</auth>"; // \0meo_117\0Umunyarwandakazi1
																															  // sendbuf = "<iq type='get' id='auth1'> <query xmlns = 'jabber:iq:auth'/> </iq>";
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult) printf("c: %s\n", sendbuf);

	Sleep(200);

	sendbuf = "<stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>";
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult) printf("c: %s\n", sendbuf);
	Sleep(200);

	sendbuf = "<iq id = 'bind_1' type = 'set'> <bind xmlns = 'urn:ietf:params:xml:ns:xmpp-bind'><resource>home</resource></bind></iq>";
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult) printf("c: %s\n", sendbuf);
	Sleep(200);

	sendbuf = "<iq type='set' id='sess_1'> <session xmlns = 'urn:ietf:params:xml:ns:xmpp-session'/></iq>";
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult) printf("c: %s\n", sendbuf);
	Sleep(200);

	// sendbuf = "<presence type='probe' to='meo_117@creep.im'/>";
	// iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	//if (iResult) printf("c: %s\n", sendbuf);
	// Sleep(200);

	// receive();
}

void newaccount() // registration is great!
{
	// char *sendbuf;
	TCHAR sendbuf[4096];
	int iResult;
	char recvbuf[DEFAULT_BUFLEN + 1];
	int recvbuflen = DEFAULT_BUFLEN;
	char buf[64];
	FLAG = 2;

	// logging in
	// sendbuf = "<iq type='get' id='reg1'> <query xmlns = 'jabber:iq:register'/></iq>"; // request
	strcpy(sendbuf, "<iq type='get' id='reg1'> <query xmlns = 'jabber:iq:register'/></iq>");
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult) printf("c: %s\n", sendbuf);

	// new bind
	HWND hctl = GetDlgItem(hWnd, IDC_LOGIN1);
	HWND hctl2 = GetDlgItem(hctl, IDC_LOGINTXT);
	GetWindowText(hctl2, buf, 64);
	strcpy(sendbuf, "<iq type='set' id='reg2'><query xmlns = 'jabber:iq:register'><username>");
	strcat(sendbuf, buf);
	strcat(sendbuf, "</username><password>");
	hctl2 = GetDlgItem(hctl, IDC_LOGINPWD);
	GetWindowText(hctl2, buf, 64);
	strcat(sendbuf, buf);
	strcat(sendbuf, "</password></query></iq>");
	// iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult) printf("c: %s\n", sendbuf);

	// strcpy(sendbuf, "</stream:stream>");
	// iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	// if (iResult) printf("c: %s\n", sendbuf);

	// make a question about its successful case right here
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR
	lpszCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) }; //fill the properties of this given window class structure
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // check the documentation to see more styles.
	wcex.lpfnWndProc = MyWindowProc; // оконная процедура. performed to every message the window receives
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("MyWindowClass"); // имя класса
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (0 == RegisterClassEx(&wcex)) // then register the window class
	{
		return -1;
	}

	//fill the properties of this given window class structure
	WNDCLASSEX wcex1 = { sizeof(WNDCLASSEX) }; //fill the properties of this given window class structure
	wcex1.style = 0; // check the documentation to see more styles.
	wcex.hInstance = hInstance;
	wcex1.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex1.lpszClassName = TEXT("Msg1"); // имя класса

	if (0 == RegisterClassEx(&wcex1)) // then register the window class
	{
		return -1;
	}
	// библиотека элементов управления общего пользования
	LoadLibrary(TEXT("ComCtl32.dll"));

	// создаем главное окно на основе нового оконного класса
	HWND hWnd = CreateWindowEx(0, TEXT("MyWindowClass"),
		TEXT("Сеть ИТешника в реальном времени"), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX , CW_USEDEFAULT, 0,
		412, 550, NULL, NULL, hInstance, NULL);
	if (NULL == hWnd)
	{
		return -1;
	} // if
	ShowWindow(hWnd, nCmdShow);
	MSG msg;
	BOOL bRet;
	for (;;)
	{
		while (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			OnIdle(hWnd);
		}
		bRet = GetMessage(&msg, NULL, 0, 0);
		if (bRet == -1)
		{
		} // if
		else if (FALSE == bRet)
		{
			break; // получено WM_QUIT, выход из цикла
		} // if
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} // for
	return (int)msg.wParam; // завершаем работу приложения


} // _tWinMain

LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMsg, WPARAM
	wParam, LPARAM lParam)
{
	SCROLLINFO si;

	HWND hctl = GetDlgItem(hWnd, 101);
	hctl = GetDlgItem(hctl, IDC_NEW);
	si.nMax = Yasb; // the maximum potential length
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE;
	si.nMin = 0;
	si.nPage = 80; // the page portion, taken after the length of the window
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

	switch (uMsg)
	{
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);

		// return 0;

	case WM_VSCROLL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);

		Yscr = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_LINEUP:
			si.nPos -= 1;
			break;
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);

		if (si.nPos != Yscr)
		{
			ScrollWindow(hctl, 0, Yscr - si.nPos, NULL, NULL);
			UpdateWindow(hWnd);
		}

		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam); // window procedure by default
} // MyWindowProc

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	flag = TRUE;
	*hJob = NULL;
	hWnd = hwnd;
	HWND hctl;
	lpCrStr = lpCreateStruct;
	// создаем список
	CreateWindowEx(0, TEXT("Static"), TEXT("Receiver:"),
		WS_CHILD | WS_VISIBLE, 5, 10, 379, 50, hwnd, NULL, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Edit"), TEXT("meo_117@creep.im"),
		WS_CHILD | WS_VISIBLE | WS_BORDER, 120, 10, 200, 20, hwnd, (HMENU)IDC_RECIPIENT, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(WS_EX_STATICEDGE, TEXT("Static"), NULL, WS_CHILD | WS_VISIBLE, 10, 50,
		359, 360, hwnd, (HMENU)101, lpCreateStruct->hInstance, NULL);
	hctl = GetDlgItem(hwnd, 101);

	hctl1 = CreateWindowEx(0, TEXT("Static"), NULL, WS_CHILD | WS_VISIBLE, 0, 0,
		357, 338, hctl, (HMENU)IDC_NEW, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL, 10, 420,
		279, 70, hwnd, (HMENU)IDC_NEWRECIPIENT, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Button"), TEXT("Send"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY, 300, 420,
		70, 30, hwnd, (HMENU)IDC_SEND, lpCreateStruct->hInstance, NULL);

	hctl = GetDlgItem(hwnd, IDC_INCOMING_TEXTS);
	ListBox_AddString(hctl, TEXT("message\n somehow long, with \t special characters, new lines, tabulations etc."));

	hctl = CreateWindowEx(0, TEXT("Static"), TEXT(""),
		WS_CHILD | WS_VISIBLE, 0, 0, 412, 550, hwnd, (HMENU)IDC_LOGIN1, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Static"), TEXT("Сеть ИТешника в реальном времени.\n\nдобро пожаловать!"),
		WS_CHILD | WS_VISIBLE, 100, 50, 212, 100, hctl, NULL, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Static"), TEXT("логин"),
		WS_CHILD | WS_VISIBLE, 100, 170, 212, 20, hctl, NULL, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Edit"), TEXT("user1234"),
		WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 200, 212, 20, hctl, (HMENU)IDC_LOGINTXT, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Static"), TEXT("пароль"),
		WS_CHILD | WS_VISIBLE, 100, 250, 212, 20, hctl, NULL, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Edit"), TEXT("Umunyarwandakazi1"),
		WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 280, 212, 20, hctl, (HMENU)IDC_LOGINPWD, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Button"), TEXT("Login"),
		WS_CHILD | WS_VISIBLE | WS_BORDER, 222, 340, 90, 24, hWnd, (HMENU)IDC_LOGIN, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Static"), TEXT("Нтазинда Р."),
		WS_CHILD | WS_VISIBLE , 150, 492, 90, 20, hWnd, NULL, lpCreateStruct->hInstance, NULL);

	init();
	DWORD dwThreadId;
	HANDLE lph;
	lph = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadProc, NULL, 0, &dwThreadId);
	// _stprintf_s(buf1, _T("%s"), TEXT(""));

	return TRUE;
} // OnCreate

void OnDestroy(HWND hwnd)
{
	PostQuitMessage(0); // отправляем сообщение WM_QUIT
} // OnDestroy

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	HWND hctl = GetDlgItem(hwnd, IDC_SEND);
	TCHAR buf1[100];
	ULONG aProcessIds[1024];

	if (BN_CLICKED == codeNotify)
	{
		hctl = GetDlgItem(hWnd, IDC_NEWRECIPIENT);
		TCHAR buf[1024];
		switch (id)
		{
		case IDC_SEND:
			char sendbuf[1024];
			int iResult;

			strcpy(sendbuf, "<message to='");

			hctl = GetDlgItem(hWnd, IDC_RECIPIENT);
			GetWindowText(hctl, buf, 1024);
			strcat(sendbuf, buf);
			strcat(sendbuf, "'><body>");

			hctl = GetDlgItem(hWnd, IDC_NEWRECIPIENT);

			GetWindowText(hctl, buf, 1024);
			int n;
			n = strlen(buf) / 42;
			n *= 20;
			for (int i = 0; buf[i] != '\0'; i++)
				if (buf[i] == '\n')
					n += 20;
			hctl = GetDlgItem(hwnd, 101);
			hctl = GetDlgItem(hctl, IDC_NEW);
			hctl = CreateWindowEx(0, TEXT("Static"), buf,
				WS_CHILD | WS_VISIBLE | WS_BORDER, 10, Yasb - Yscr, 289, 20 + n, hctl1, NULL, lpCrStr->hInstance, NULL);

			RECT Rect;
			GetWindowRect(hctl, &Rect);
			Yasb += (Rect.bottom - Rect.top + 10);
			// Static_SetText(hctl, "here and there.");
			strcat(sendbuf, buf);
			strcat(sendbuf, "</body></message>");
			// sendbuf = "<message to='meo_117@creep.im'><body> this is a message. </body></message>";

			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult) printf("c: %s\n", sendbuf);
			// Sleep(200);
			ScrollWindow(hctl, 0, Yasb, NULL, NULL);
			break;
		case IDC_LOGIN:
			hctl = GetDlgItem(hwnd, IDC_LOGIN1);
			login();
			break;
		case IDC_LOGINREG:
			hctl = GetDlgItem(hwnd, IDC_LOGIN1);
			ShowWindow(GetDlgItem(GetDlgItem(hwnd, IDC_LOGIN1), IDC_CAPTCHA), 1);
			ShowWindow(GetDlgItem(hwnd, IDC_LOGINREG2), 1);
			// login();
			newaccount();
			break;
		case IDC_LOGINREG2:
			// hctl = GetDlgItem(hWnd, IDC_LOGIN1);
			hctl = GetDlgItem(GetDlgItem(hWnd, IDC_LOGIN1), IDC_CAPTCHA);
			GetWindowText(hctl, buf, 64);
			strcpy(sendbuf, "<iq type='set' xml:lang='en' id='reg2'><query xmlns='jabber:iq:register'><x xmlns = 'jabber:x:data' type = 'submit'>"
				"<field var = 'FORM_TYPE'><value>urn:xmpp:captcha</value></field><field var = 'ocr'><value>");
			// 7nHL3< / value>< / field>"
			strcat(sendbuf, buf);
			strcat(sendbuf, "</value></field><field var = 'username'><value>");
			hctl = GetDlgItem(GetDlgItem(hWnd, IDC_LOGIN1), IDC_LOGINTXT);
			GetWindowText(hctl, buf, 64);
			strcat(sendbuf, buf);
			strcat(sendbuf, "</value></field><field var = 'password'><value>");
			hctl = GetDlgItem(GetDlgItem(hWnd, IDC_LOGIN1), IDC_LOGINPWD);
			GetWindowText(hctl, buf, 64);
			strcat(sendbuf, buf);
			strcat(sendbuf, "</value></field></x></query></iq>");
			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);

			// hctl = GetDlgItem(hctl, IDC_LOGINPWD);
			// GetWindowText(hctl, buf, 64);
			// strcat(sendbuf, buf);
			// strcat(sendbuf, "</password></query></iq>");
			break;
		}
	}
}

void OnIdle(HWND hwnd)
{
	if (incoming)
	{
		//SetWindowText(hWnd, );
		int n = strlen(in_buf) / 42;
		n *= 20;
		if (in_buf[0] != '\b'){
			for (int i = 0; in_buf[i] != '\0'; i++)
				if (in_buf[i] == '\n')
					n += 20;
		}
		else
		{
			// redraw the form, introduce new panels to show the text and a box to enter what you see, plus a submit button.
			HWND hctl = CreateWindowEx(0, TEXT("Static"), TEXT(""),
				WS_CHILD | WS_VISIBLE, 0, 0, 412, 550, hWnd, (HMENU)IDC_CAPTCHA, lpCrStr->hInstance, NULL);

			CreateWindowEx(0, TEXT("Static"), TEXT("Enter the text you see"),
				WS_CHILD | WS_VISIBLE, 80, 40, 250, 40, hctl, (HMENU)IDC_CAPTCHA, lpCrStr->hInstance, NULL);

			CreateWindowEx(0, TEXT("Static"), TEXT(""),
				WS_CHILD | WS_VISIBLE | WS_BORDER, 80, 80, 250, 170, hctl, (HMENU)IDC_CAPTCHA, lpCrStr->hInstance, NULL);
		}

		HWND hctl = CreateWindowEx(0, TEXT("Static"), in_buf,
			WS_CHILD | WS_VISIBLE | WS_BORDER, 55, Yasb - Yscr, 289, 20 + n, hctl1, NULL, lpCrStr->hInstance, NULL);

		RECT Rect;
		GetWindowRect(hctl, &Rect);
		Yasb += (Rect.bottom - Rect.top + 10);
		incoming = FALSE;



	}
}