// Win32Project1.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Win32Project1.h"
#include "vg\openvg.h"
#include "egl\egl.h"
#include "vgLoadImage.h"
#include "DXUTsound.h"
#include <time.h>

#define KEY_DOWN(code) (GetAsyncKeyState(code)&0x8000)

EGLDisplay display;
EGLSurface surface;
EGLContext context;

VGImage image;


void timerProc();
struct Plane {
	int x;
	int y;
}plane;

struct Enemy {
	int x, y, playing, type;
}enemies[100];

VGImage missileImg, planeImg, background, enemyimg;



#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

												// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.


	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

	MSG msg;

	// 기본 메시지 루프입니다.
	DWORD lastTime = GetTickCount();
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, 1))
		{
			if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (msg.message == WM_QUIT) break;
		}
		DWORD curTime = GetTickCount();
		if (curTime - lastTime>32) // 30 frame per second
		{
			lastTime = lastTime + 33;
			timerProc();
		}
	}



	return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 300, 600, nullptr, nullptr, hInstance, nullptr);


	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택을 구문 분석합니다.
		switch (wmId)
		{
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
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CREATE:
	{
		display = eglGetDisplay(GetDC(hWnd));
		eglInitialize(display, NULL, NULL);
		eglBindAPI(EGL_OPENVG_API);

		EGLint conf_list[] = { EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_NONE };
		EGLConfig config;
		EGLint    num_config;

		srand((unsigned)time(NULL));

		eglChooseConfig(display, conf_list, &config, 1, &num_config);
		surface = eglCreateWindowSurface(display, config, hWnd, NULL);
		context = eglCreateContext(display, 0, NULL, NULL);

		planeImg = vgLoadImage(TEXT("character.png"));
		background = vgLoadImage(TEXT("background.png"));
		enemyimg = vgLoadImage(TEXT("poo.png"));

		plane.x = 10;
		plane.y = 10;

		for (int i = 0; i < 100; i++) enemies[i].playing = false;

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

// 정보 대화 상자의 메시지 처리기입니다.
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



void Draw(void) {
	eglMakeCurrent(display, surface, surface, context);

	vgClear(0, 0, 10000, 10000);
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);

	vgLoadIdentity();
	vgDrawImage(background);
	vgLoadIdentity();
	vgTranslate(plane.x, plane.y);
	vgDrawImage(planeImg);
	vgLoadIdentity();

	for (int i = 0; i < 100; i++) {
		if (enemies[i].playing) {
			vgLoadIdentity();
			vgTranslate(enemies[i].x, enemies[i].y);
			vgDrawImage(enemyimg);
		}
	}
	eglSwapBuffers(display, surface);
}

#define KEY_DOWN(code) (GetAsyncKeyState(code) & 0x8000)

void timerProc()
{
	if (KEY_DOWN(VK_LEFT) && plane.x >= 0)  plane.x -= 10;
	if (KEY_DOWN(VK_RIGHT) && plane.x < 200)  plane.x += 10;


	for (int i = 0; i<100; i++) {
		if (enemies[i].playing) {
			enemies[i].y -= 5;
			if (enemies[i].y<-100) enemies[i].playing = false;
		}
	}

	if (rand() % 30 == 0) {  // about one per 30 frames: about one per second.
		for (int i = 0; i<100; i++) {
			if (enemies[i].playing == false) {
				enemies[i].playing = true;
				enemies[i].y = 800;
				enemies[i].x = (rand() % 300) + 1;
				break;
			}
		}
	}

	for (int i = 0; i < 100; i++) {
		if (enemies[i].playing) {
			if ((enemies[i].x - 10< plane.x + 50) && (plane.x - 10< enemies[i].x + 10)
				&& (plane.y + 60> enemies[i].y - 10) && (enemies[i].y + 10>plane.y - 40)) {                                   
				exit(0);
			}
		}
	}

	Draw();
}
