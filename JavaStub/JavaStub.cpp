// JavaStub.cpp : Definisce il punto di ingresso dell'applicazione.
//

#include "framework.h"
#include "JavaStub.h"
#include "JavaStubApp.h"

#define MAX_LOADSTRING 100
#define ScreenX GetSystemMetrics(SM_CXSCREEN)
#define ScreenY GetSystemMetrics(SM_CYSCREEN)
#define IDM_START				1001

#define WIN_WIDTH 400
#define WIN_HEIGHT 150

// Variabili globali:
HINSTANCE hInst;                                // istanza corrente
HWND hWnd;		                                // finestra principale
WCHAR szTitle[MAX_LOADSTRING];                  // Testo della barra del titolo
WCHAR szWindowClass[MAX_LOADSTRING];            // nome della classe di finestre principale
WCHAR szProduct[MAX_LOADSTRING];				// nome del prodotto
WCHAR szMessage[MAX_LOADSTRING];				// messaggio principale
int valProgress = 10;							// percentuale di completamento (0-100)
HANDLE hTread = 0;								// handle del tread di lavorazione
int verbose = 10;								// livello di logging

// Dichiarazioni con prototipo delle funzioni incluse in questo modulo di codice:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void paintWindow(HDC hdc);
void fatalError(WCHAR* errorMsg);
DWORD WINAPI startOperazioni(LPVOID lpParam);

void fatalError(const WCHAR* errorMsg);
void updateCount(int val);
void setTitle(const WCHAR* title);
void setMessage(const WCHAR* message);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: inserire qui il codice.

	// Inizializzare le stringhe globali
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_JAVASTUB, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Eseguire l'inizializzazione dall'applicazione:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JAVASTUB));

	MSG msg;

	// Ciclo di messaggi principale:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNZIONE: MyRegisterClass()
//
//  SCOPO: registra la classe di finestre.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JAVASTUB));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0; // MAKEINTRESOURCEW(IDC_JAVASTUB);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNZIONE: InitInstance(HINSTANCE, int)
//
//   SCOPO: salva l'handle di istanza e crea la finestra principale
//
//   COMMENTI:
//
//        In questa funzione l'handle di istanza viene salvato in una variabile globale e
//        la finestra di programma principale viene creata e visualizzata.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Memorizzare l'handle di istanza nella variabile globale

	//HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	//   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	wcscpy_s(szProduct, TEXT("Java application loader"));
	wcscpy_s(szMessage, TEXT("Loading ..."));

	int posx = (ScreenX - WIN_WIDTH) / 2;
	int posy = (ScreenY - WIN_HEIGHT) / 2;

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_VISIBLE,
		posx, posy, WIN_WIDTH, WIN_HEIGHT, nullptr, nullptr, hInstance, nullptr);
	SetWindowLong(hWnd, GWL_STYLE, WS_BORDER);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	PostMessage(hWnd, WM_COMMAND, IDM_START, 0);

	return TRUE;
}

//
//  FUNZIONE: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  SCOPO:  elabora i messaggi per la finestra principale.
//
//  WM_COMMAND - elabora il menu dell'applicazione
//  WM_PAINT - disegna la finestra principale
//  WM_DESTROY - inserisce un messaggio di uscita e restituisce un risultato
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Analizzare le selezioni di menu:
		switch (wmId)
		{
		case IDM_START:
			hTread = CreateThread(NULL, 0, startOperazioni, NULL, 0, NULL);
			if (hTread == NULL)
				fatalError(TEXT("Errore fatale: non posso avviare il thread di servizio."));
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		paintWindow(hdc);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Gestore dei messaggi della finestra Informazioni su.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void paintWindow(HDC hdc)
{
	RECT msgRect, prodRect, progresRect, valueRect;
	GetClientRect(hWnd, &msgRect);
	progresRect = prodRect = msgRect;
	prodRect.top = 20;
	prodRect.bottom = 40;
	msgRect.top = 60;
	msgRect.bottom = 80;
	progresRect.left = 50;
	progresRect.right -= 50;
	progresRect.top = 100;
	progresRect.bottom = 110;
	valueRect = progresRect;
	valueRect.right = valueRect.left + (((valueRect.right - valueRect.left) * valProgress) / 100);

	// messaggi
	DrawText(hdc, szProduct, -1, &prodRect, DT_CENTER);
	DrawText(hdc, szMessage, -1, &msgRect, DT_CENTER);

	// progres bar
	Rectangle(hdc, progresRect.left, progresRect.top, progresRect.right, progresRect.bottom);
	FillRect(hdc, &valueRect, (HBRUSH)(COLOR_WINDOWTEXT + 1));
}

void fatalError(const WCHAR* errorMsg)
{
	LOG(1, L"ERRORE FATALE: %s\n", errorMsg);
	MessageBox(hWnd, errorMsg, TEXT("Errore fatale"), 0);
	PostQuitMessage(0);
}

void updateCount(int val)
{
	valProgress = val;
	InvalidateRect(hWnd, NULL, TRUE);
}

void setTitle(const WCHAR* title)
{
	wcscpy_s(szProduct, title);
	InvalidateRect(hWnd, NULL, TRUE);
}

void setMessage(const WCHAR* message)
{
	wcscpy_s(szMessage, message);
	InvalidateRect(hWnd, NULL, TRUE);
}

DWORD WINAPI startOperazioni(LPVOID lpParam)
{
	try
	{
		JavaStubApp app;
		if (app.startOperation())
		{
			// aspetta l'avvio di java per 3 secondi
			Sleep(3000);
		}
	}
	catch (...)
	{
		// errore fatale: un messaggio è stato probabilmente già visualizzato
		_asm nop
	}

	PostMessage(hWnd, WM_DESTROY, 0, 0);
	return 0;
}


