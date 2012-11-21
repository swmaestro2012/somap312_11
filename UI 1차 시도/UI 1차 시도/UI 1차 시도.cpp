// UI 1�� �õ�.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "UI 1�� �õ�.h"
#include <shlobj.h>
#include"parser.h"


#define MAX_LOADSTRING		100
#define ID_BUTTON_RUN		9 
#define ID_COMBOBOX_SCOPE	10 
#define ID_COMBOBOX_NAME	11
#define ID_COMBOBOX_ROW		12
#define ID_SCR_X			13
#define ID_SCR_Y			13

// ���� ����:
HINSTANCE hInst;								// ���� �ν��Ͻ��Դϴ�.
TCHAR szTitle[MAX_LOADSTRING];					// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR szWindowClass[MAX_LOADSTRING];			// �⺻ â Ŭ���� �̸��Դϴ�.
Parser parser;									//���α׷� �۵� Ŭ����

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL BrowseFolder(HWND hParent, LPCTSTR szTitle, LPCTSTR StartPath, TCHAR *szFolder);
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: ���⿡ �ڵ带 �Է��մϴ�.
	MSG msg;
	HACCEL hAccelTable;

	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_UI1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UI1));

	// �⺻ �޽��� �����Դϴ�.
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
//  ����:
//
//    Windows 95���� �߰��� 'RegisterClassEx' �Լ����� ����
//    �ش� �ڵ尡 Win32 �ý��۰� ȣȯ�ǵ���
//    �Ϸ��� ��쿡�� �� �Լ��� ����մϴ�. �� �Լ��� ȣ���ؾ�
//    �ش� ���� ���α׷��� �����
//    '�ùٸ� ������' ���� �������� ������ �� �ֽ��ϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UI1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_UI1);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����: �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND	- ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT	- �� â�� �׸��ϴ�.
//  WM_DESTROY	- ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static TCHAR StartPath[MAX_PATH];
	TCHAR Folder[MAX_PATH];
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	static HWND hScroll[2];						//��ũ�ѹ�

	int i,j;									//���ڸ� ������� ��� ���

	static HWND hCombo[3];
	static int scopeType;						//ù��° �޺� �ڽ� ��

	static std::string className;				//���� ���õ� ���� �̸� �Ǵ� Ŭ���� �̸�
	static std::list<std::string> selectedRow;			//������ ���� ������ ���ڵ�

	std::list<std::string>::const_iterator tableList[2];

	//�׷����� ���� ����
	HDC memDC;
	HBITMAP hBit, OldBit;
	HBRUSH indexBrush,unusedBrush,readBrush,writeBrush,oldBrush;
	RECT rt;

	static int scrollX,scrollY,scrollW,scrollH;
	static RECT clientRect;

	switch (message)
	{
	case WM_CREATE:
		CreateWindow("static","Scope Type :",WS_CHILD | WS_VISIBLE | SS_CENTER,
			10,10,150,25,hWnd,(HMENU) 0, hInst,NULL);
		hCombo[0]=CreateWindow("combobox",NULL,WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			160,10,200,200,hWnd,(HMENU)ID_COMBOBOX_SCOPE,hInst,NULL);
		SendMessage(hCombo[0],CB_ADDSTRING,0,(LPARAM)"Project");
		SendMessage(hCombo[0],CB_ADDSTRING,0,(LPARAM)"File");
		SendMessage(hCombo[0],CB_ADDSTRING,0,(LPARAM)"Class");
		SendMessage(hCombo[0],CB_SETCURSEL,0,0);

		CreateWindow("static","Scope Name :",WS_CHILD | WS_VISIBLE | SS_CENTER,
			370,10,150,25,hWnd,(HMENU) 0, hInst,NULL);

		hCombo[1]=CreateWindow("combobox",NULL,WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			520,10,200,200,hWnd,(HMENU)ID_COMBOBOX_NAME,hInst,NULL);
		EnableWindow(hCombo[1],FALSE);

		CreateWindow("static","Row :",WS_CHILD | WS_VISIBLE | SS_CENTER,
			730,10,150,25,hWnd,(HMENU) 0, hInst,NULL);

		hCombo[2]=CreateWindow("combobox",NULL,WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			890,10,200,200,hWnd,(HMENU)ID_COMBOBOX_ROW,hInst,NULL);

		SendMessage(hCombo[2],CB_ADDSTRING,0,(LPARAM)"Variable");
		SendMessage(hCombo[2],CB_ADDSTRING,0,(LPARAM)"Function");
		SendMessage(hCombo[2],CB_SETCURSEL,1,0);
		EnableWindow(hCombo[2],FALSE);

		hScroll[0]=CreateWindow("scrollbar",NULL,WS_CHILD | WS_VISIBLE | SBS_HORZ,
			clientRect.left,clientRect.bottom-10,clientRect.right,clientRect.bottom,hWnd,(HMENU)ID_SCR_X,hInst,NULL);
		hScroll[1]=CreateWindow("scrollbar",NULL,WS_CHILD | WS_VISIBLE | SBS_VERT,
			clientRect.right-10,clientRect.top,clientRect.right,clientRect.bottom,hWnd,(HMENU)ID_SCR_Y,hInst,NULL);
		SetScrollRange(hScroll[0],SB_CTL,0,clientRect.right,TRUE);
		SetScrollRange(hScroll[1],SB_CTL,0,clientRect.bottom,TRUE);
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �޴� ������ ���� �м��մϴ�.
		switch (wmId)
		{
		case ID_NEW:
			if(BrowseFolder(hWnd,TEXT("������ �����Ͻÿ�"),StartPath,Folder))
			{
				parser.LoadFile(Folder);
				lstrcpy(StartPath,Folder);
				SendMessage(hWnd,WM_COMMAND,(ID_COMBOBOX_SCOPE)+(CBN_SELCHANGE<<16),0);
			}
			break;

		case ID_COMBOBOX_SCOPE:
			switch(HIWORD(wParam))
			{	
			case CBN_SELCHANGE:
				scopeType=SendMessage(hCombo[0], CB_GETCURSEL,0,0);
				switch(scopeType)
				{
				case 0:
					while(SendMessage(hCombo[1],CB_DELETESTRING,0,0)!=-1);
					EnableWindow(hCombo[1],FALSE);
					SendMessage(hCombo[2],CB_SETCURSEL,1,0);
					EnableWindow(hCombo[2],FALSE);
					className=std::string("project");
					break;
				case 1:
					while(SendMessage(hCombo[1],CB_DELETESTRING,0,0)!=-1);					
					if(!parser.fileList.empty())
					{
						EnableWindow(hCombo[1],TRUE);
						for(tableList[0]=parser.fileList.begin();
							tableList[0]!=parser.fileList.end();
							++tableList[0])
						{
							for(i=tableList[0]->size()-1;i>0;--i)
							{
								if(tableList[0]->begin()._Ptr[i]=='/')
									break;
							}
							SendMessage(hCombo[1],CB_ADDSTRING,0,(LPARAM)&(tableList[0]->begin()._Ptr[i+1]));
						}
						SendMessage(hCombo[1],CB_SETCURSEL,0,0);
						SendMessage(hCombo[2],CB_SETCURSEL,0,0);
						EnableWindow(hCombo[2],FALSE);
						className=std::string(parser.fileList.begin()->begin()._Ptr);
					}
					else
					{
						EnableWindow(hCombo[1],FALSE);
						SendMessage(hCombo[2],CB_SETCURSEL,0,0);
						EnableWindow(hCombo[2],FALSE);
					}
					break;

				case 2:
					while(SendMessage(hCombo[1],CB_DELETESTRING,0,0)!=-1);
					if(!parser.classList.empty())
					{
						EnableWindow(hCombo[1],TRUE);
						for(tableList[0]=parser.classList.begin();
							tableList[0]!=parser.classList.end();
							++tableList[0])
						{
							SendMessage(hCombo[1],CB_ADDSTRING,0,(LPARAM)(tableList[0]->begin()._Ptr));
						}
						SendMessage(hCombo[1],CB_SETCURSEL,0,0);
						SendMessage(hCombo[2],CB_SETCURSEL,0,0);
						EnableWindow(hCombo[2],TRUE);
						className=std::string(parser.classList.begin()->begin()._Ptr);
					}
					else
					{
						EnableWindow(hCombo[1],FALSE);
						SendMessage(hCombo[2],CB_SETCURSEL,0,0);
						EnableWindow(hCombo[2],FALSE);
					}					
					break;
				}
				if(SendMessage(hCombo[2],CB_GETCURSEL,0,0))
					scrollH=parser.masterList[className].second.size()*50;
				else
					scrollH=parser.masterList[className].first.size()*50;
				scrollW=parser.masterList[className].second.size()*200;
				SetScrollRange(hScroll[0],SB_CTL,0,scrollW-clientRect.right,TRUE);
				SetScrollRange(hScroll[1],SB_CTL,0,scrollH-clientRect.bottom,TRUE);
				selectedRow.clear();
				InvalidateRect(hWnd,NULL,TRUE);
				break;
			}
			break;
			
		case ID_COMBOBOX_NAME:
			switch(HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				switch(scopeType)
				{
				case 0:
					className=std::string("project");
					break;
				case 1:
					tableList[0]=parser.fileList.begin();
					for(i=0;i<SendMessage(hCombo[1],CB_GETCURSEL,0,0);i++)
					{
						tableList[0]++;
					}
					className=std::string(tableList[0]->begin()._Ptr);
					break;
				case 2:
					tableList[0]=parser.classList.begin();
					for(i=0;i<SendMessage(hCombo[1],CB_GETCURSEL,0,0);i++)
					{
						tableList[0]++;
					}
					className=std::string(tableList[0]->begin()._Ptr);
					break;
				}
				if(SendMessage(hCombo[2],CB_GETCURSEL,0,0))
					scrollH=parser.masterList[className].second.size()*50;
				else
					scrollH=parser.masterList[className].first.size()*50;
				scrollW=parser.masterList[className].second.size()*200;
				SetScrollRange(hScroll[0],SB_CTL,0,scrollW/clientRect.right,TRUE);
				SetScrollRange(hScroll[1],SB_CTL,0,scrollH/clientRect.bottom,TRUE);
				selectedRow.clear();
				InvalidateRect(hWnd,NULL,TRUE);
				break;
			}
			break;

		case ID_COMBOBOX_ROW:
			switch(HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				if(SendMessage(hCombo[2],CB_GETCURSEL,0,0))
					scrollH=parser.masterList[className].second.size()*50;
				else
					scrollH=parser.masterList[className].first.size()*50;
				scrollW=parser.masterList[className].second.size()*200;
				SetScrollRange(hScroll[0],SB_CTL,0,scrollW/clientRect.right,TRUE);
				SetScrollRange(hScroll[1],SB_CTL,0,scrollH/clientRect.bottom,TRUE);
				selectedRow.clear();
				InvalidateRect(hWnd,NULL,TRUE);
				break;
			}
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
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.
		GetClientRect(hWnd,&rt);
		hBit=CreateCompatibleBitmap(hdc,clientRect.right,clientRect.bottom);
		memDC=CreateCompatibleDC(hdc);
		OldBit=(HBITMAP)SelectObject(memDC,hBit);

		indexBrush=CreateSolidBrush(RGB(255,200,0));
		unusedBrush=CreateSolidBrush(RGB(250,220,50));
		readBrush=CreateSolidBrush	(RGB(250,100,50));
		writeBrush=CreateSolidBrush	(RGB(255,100,50));


		//DrawRowIndex
		if(selectedRow.empty())
		{
			i=0;
			FillRect(memDC,&clientRect,HBRUSH(RGB(100,100,100)));//(HBRUSH)(COLOR_WINDOW+1));
			for(tableList[1]=parser.masterList[className].second.begin();tableList[1]!=parser.masterList[className].second.end();++tableList[1])
			{
				rt.left=200*i+1-scrollX;
				rt.right=200*(i+1)-1-scrollX;
				rt.bottom=25;
				rt.top=0;
				DrawText(memDC,tableList[1]->begin()._Ptr,tableList[1]->size(),&rt,DT_CENTER);
				++i;
			}
			BitBlt(hdc,200,75,clientRect.right,300,memDC,0,0,SRCCOPY);

			FillRect(memDC,&clientRect,HBRUSH(RGB(100,100,100)));//(HBRUSH)(COLOR_WINDOW+1));
			switch(SendMessage(hCombo[2],CB_GETCURSEL,0,0))
			{
			case 0:
				i=0;
				for(tableList[0]=parser.masterList[className].first.begin();tableList[0]!=parser.masterList[className].first.end();++tableList[0])
				{
					rt.left=1;
					rt.right=200-1;
					rt.bottom=50*(i+1)-1-scrollY;
					rt.top=50*i+1-scrollY;
					DrawText(memDC,tableList[0]->begin()._Ptr,tableList[0]->size(),&rt,DT_VCENTER);
					++i;
				}
				break;

			case 1:
				i=0;
				for(tableList[0]=parser.masterList[className].second.begin();tableList[0]!=parser.masterList[className].second.end();++tableList[0])
				{
					rt.left=1;
					rt.right=200-1;
					rt.bottom=50*(i+1)-1-scrollY;
					rt.top=50*i+1-scrollY;
					DrawText(memDC,tableList[0]->begin()._Ptr,tableList[0]->size(),&rt,DT_VCENTER);
					++i;
				}
				break;
			}
			BitBlt(hdc,10,100,210,clientRect.bottom,memDC,0,0,SRCCOPY);

			//Draw Table
			FillRect(memDC,&clientRect,HBRUSH(RGB(100,100,100)));//(HBRUSH)(COLOR_WINDOW+1));
			if(parser.masterList[className].second.empty())
			{
				GetClientRect(hWnd,&rt);
				rt.left=rt.right/2-300;
				rt.right=rt.right/2;
				rt.top=rt.bottom/2;
				rt.bottom=rt.bottom/2;
				DrawText(memDC,"ǥ�� ��Ÿ�� ��Ұ� �����ϴ�.",28,&rt,DT_CENTER);
			}
			else
			{
				switch(scopeType)
				{
				case 0:
					i=0;
					for(tableList[0]=parser.masterList[className].second.begin();tableList[0]!=parser.masterList[className].second.end();++tableList[0])
					{
						rt.bottom=50*(i+1)-1-scrollY;
						rt.top=50*i+1-scrollY;
						if(rt.bottom>clientRect.bottom)
							break;
						j=0;
						for(tableList[1]=parser.masterList[className].second.begin();tableList[1]!=parser.masterList[className].second.end();++tableList[1])
						{
							rt.left=200*j+1-scrollX;
							rt.right=200*(j+1)-1-scrollX;
							if(rt.right>clientRect.right)
								break;
							switch(parser.masterTable[className].second[*tableList[0]][*tableList[1]])
							{
							case 0:
								oldBrush=(HBRUSH)SelectObject(hdc,unusedBrush);
								FillRect(memDC,&rt,unusedBrush);
								(HBRUSH)SelectObject(hdc,oldBrush);
								break;
							case 1:
								oldBrush=(HBRUSH)SelectObject(hdc,readBrush);
								FillRect(memDC,&rt,readBrush);
								(HBRUSH)SelectObject(hdc,oldBrush);
								break;
							case 2:
								oldBrush=(HBRUSH)SelectObject(hdc,writeBrush);
								FillRect(memDC,&rt,writeBrush);
								(HBRUSH)SelectObject(hdc,oldBrush);
								break;
							}
							++j;
						}
						++i;
					}
					break;
				case 1:
					i=0;
					for(tableList[0]=parser.masterList[className].first.begin();tableList[0]!=parser.masterList[className].first.end();++tableList[0])
					{
						rt.bottom=50*(i+1)-1-scrollY;
						rt.top=50*i+1-scrollY;
						if(rt.bottom>clientRect.bottom)
							break;
						j=0;
						for(tableList[1]=parser.masterList[className].second.begin();tableList[1]!=parser.masterList[className].second.end();++tableList[1])
						{
							rt.left=200*j+1-scrollX;
							rt.right=200*(j+1)-1-scrollX;
							if(rt.right>clientRect.right)
								break;
							switch(parser.masterTable[className].first[*tableList[0]][*tableList[1]])
							{
							case 0:
								oldBrush=(HBRUSH)SelectObject(hdc,unusedBrush);
								FillRect(memDC,&rt,unusedBrush);
								(HBRUSH)SelectObject(hdc,oldBrush);
								break;
							case 1:
								oldBrush=(HBRUSH)SelectObject(hdc,readBrush);
								FillRect(memDC,&rt,readBrush);
								(HBRUSH)SelectObject(hdc,oldBrush);
								break;
							case 2:
								oldBrush=(HBRUSH)SelectObject(hdc,writeBrush);
								FillRect(memDC,&rt,writeBrush);
								(HBRUSH)SelectObject(hdc,oldBrush);
								break;
							}
							++j;
						}
						++i;
					}
					break;
				case 2:
					switch(SendMessage(hCombo[2],CB_GETCURSEL,0,0))
					{
					case 0:
						if(parser.masterList[className].first.empty() || parser.masterList[className].second.empty())
						{
							GetClientRect(hWnd,&rt);
							rt.left=rt.right/2-300;
							rt.right=rt.right/2;
							rt.top=rt.bottom/2;
							rt.bottom=rt.bottom/2;
							DrawText(memDC,"ǥ�� ��Ÿ�� ��Ұ� �����ϴ�.",28,&rt,DT_CENTER);
							break;
						}
						i=0;
						for(tableList[0]=parser.masterList[className].first.begin();tableList[0]!=parser.masterList[className].first.end();++tableList[0])
						{
							rt.bottom=50*(i+1)-1-scrollY;
							rt.top=50*i+1-scrollY;
							if(rt.bottom>clientRect.bottom)
								break;
							j=0;
							for(tableList[1]=parser.masterList[className].second.begin();tableList[1]!=parser.masterList[className].second.end();++tableList[1])
							{
								rt.left=200*j+1-scrollX;
								rt.right=200*(j+1)-1-scrollX;
							if(rt.right>clientRect.right)
								break;
								switch(parser.masterTable[className].first[*tableList[0]][*tableList[1]])
								{
								case 0:
									oldBrush=(HBRUSH)SelectObject(hdc,unusedBrush);
									FillRect(memDC,&rt,unusedBrush);
									(HBRUSH)SelectObject(hdc,oldBrush);
									break;
								case 1:
									oldBrush=(HBRUSH)SelectObject(hdc,readBrush);
									FillRect(memDC,&rt,readBrush);
									(HBRUSH)SelectObject(hdc,oldBrush);
									break;
								case 2:
									oldBrush=(HBRUSH)SelectObject(hdc,writeBrush);
									FillRect(memDC,&rt,writeBrush);
									(HBRUSH)SelectObject(hdc,oldBrush);
									break;
								}
								++j;
							}
							++i;
						}
						break;
					case 1:

						i=0;
						for(tableList[0]=parser.masterList[className].second.begin();tableList[0]!=parser.masterList[className].second.end();++tableList[0])
						{
							j=0;
							for(tableList[1]=parser.masterList[className].second.begin();tableList[1]!=parser.masterList[className].second.end();++tableList[1])
							{
								rt.left=200*j+1-scrollX;
								rt.right=200*(j+1)-1-scrollX;
								rt.bottom=50*(i+1)-1-scrollY;
								rt.top=50*i+1-scrollY;
								switch(parser.masterTable[className].second[*tableList[0]][*tableList[1]])
								{
								case 0:
									oldBrush=(HBRUSH)SelectObject(hdc,unusedBrush);
									FillRect(memDC,&rt,unusedBrush);
									(HBRUSH)SelectObject(hdc,oldBrush);
									break;
								case 1:
									oldBrush=(HBRUSH)SelectObject(hdc,readBrush);
									FillRect(memDC,&rt,readBrush);
									(HBRUSH)SelectObject(hdc,oldBrush);
									break;
								case 2:
									oldBrush=(HBRUSH)SelectObject(hdc,writeBrush);
									FillRect(memDC,&rt,writeBrush);
									(HBRUSH)SelectObject(hdc,oldBrush);
									break;
								}
								++j;
							}
							++i;
						}
						break;
					}
					break;
				}
			}
			BitBlt(hdc,210,100,clientRect.right,clientRect.bottom,memDC,0,0,SRCCOPY);
		}
		else
		{
			}
		DeleteObject(readBrush);
		DeleteObject(unusedBrush);
		DeleteObject(writeBrush);
		DeleteObject(indexBrush);
		(HBITMAP)SelectObject(memDC,OldBit);
		DeleteDC(memDC);
		DeleteObject(hBit);

		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		GetClientRect(hWnd,&clientRect);
		MoveWindow(hScroll[0],clientRect.left,clientRect.bottom-20,clientRect.right-20,clientRect.bottom-5,TRUE);
		MoveWindow(hScroll[1],clientRect.right-20,clientRect.top,clientRect.right-5,clientRect.bottom-20,TRUE);
		SetScrollRange(hScroll[0],SB_CTL,0,scrollW,TRUE);
		SetScrollRange(hScroll[1],SB_CTL,0,scrollH,TRUE);
		break;

	case WM_HSCROLL:
		clientRect.left+=200;
		clientRect.top+=50;
		InvalidateRect(hWnd,&clientRect,FALSE);
		clientRect.left-=200;
		clientRect.top-=50;
		switch(LOWORD(wParam))
		{
		case SB_LINELEFT:
			if(scrollX > 0)
			scrollX -= 1;
			break;
		case SB_LINERIGHT:
			if(scrollX < scrollW-clientRect.right)
			scrollX += 1;
			break;
		case SB_PAGELEFT:
			if(scrollX > 0)
			scrollX -= 10;
			break;
		case SB_PAGERIGHT:
			if(scrollX < scrollW-clientRect.right)
				scrollX += 10;
			break;
		case SB_THUMBTRACK:
			scrollX = HIWORD(wParam);
			break;
	   }
	   SetScrollPos((HWND)lParam, SB_CTL, scrollX, true);
		break;

	case WM_VSCROLL:
		clientRect.top+=100;
		InvalidateRect(hWnd,&clientRect,TRUE);
		clientRect.top-=100;
		switch(LOWORD(wParam))
		{
		case SB_LINEDOWN:
			if(scrollY > 0)
			scrollY -= 1;
			break;
		case SB_LINEUP:
			if(scrollY < scrollH-clientRect.bottom)
			scrollY += 1;
			break;
		case SB_PAGEDOWN:
			if(scrollY > 0)
			scrollY -= 10;
			break;
		case SB_PAGEUP:
			if(scrollY < scrollH-clientRect.bottom)
				scrollY += 10;
			break;
		case SB_THUMBTRACK:
			scrollY = HIWORD(wParam);
			break;
		}
		SetScrollPos((HWND)lParam, SB_CTL, scrollY, true);
		break;

	case WM_LBUTTONDOWN:
/*
		if(LOWORD(lParam)<200 && HIWORD(lParam)>200)
		{
			if(SendMessage(hCombo[2],CB_GETCURSEL,0,0))
				tableList[0]=parser.masterList[className].second.begin();
			else
				tableList[0]=parser.masterList[className].first.begin();
			for(i=0;50*(i+1)<HIWORD(lParam)-200+scrollY;i++)
			{
				++tableList[0];
			}
			for(tableList[1]=selectedRow.begin();tableList[1]!=selectedRow.end();++tableList[1])
			{
				if(tableList[0]->find(tableList[1]->begin()._Ptr)!=std::string::npos)
					break;
			}
			if(tableList[1]!=selectedRow.end())
			{
				selectedRow.push_back(tableList[0]->begin()._Ptr);
			}
			else
			{
				selectedRow.remove(*tableList[1]);
			}
		}
//*/
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}




// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
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


BOOL BrowseFolder(HWND hParent, LPCTSTR szTitle, LPCTSTR StartPath, TCHAR *szFolder)
{
	LPMALLOC pMalloc;
	LPITEMIDLIST pidl;
	BROWSEINFO bi;
	bi.hwndOwner = hParent;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = szTitle;
	bi.ulFlags = 0;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)StartPath;
 
	pidl = SHBrowseForFolder(&bi);
	if(pidl == NULL) return FALSE;
	SHGetPathFromIDList(pidl,szFolder);
	if(SHGetMalloc(&pMalloc) != NOERROR)
		return FALSE;
	pMalloc->Free(pidl);
	pMalloc->Release();
	return TRUE;
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch(uMsg)
	{
		case BFFM_INITIALIZED:
		if(lpData != NULL)
		{
			SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)lpData);
		}
		break;
	}
	return 0;
}
