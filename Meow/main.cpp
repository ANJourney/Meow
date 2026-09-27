#include<Windows.h>
#include<stdio.h>
#include <commctrl.h>
#include<string>

#include "Hook.h"
#include "Log.h"
#include "resource.h"
#include "RulesSettings.h"
#include "ReplaceSettings.h"
#include "Config.h"

#define WM_INITIALIZE (WM_USER+1)
#define WM_TRAYICON (WM_USER + 2000)
#define ID_TRAYICON 3001

#define TIMER_REALTIME_CHECK 2002

#define IDC_BTN_ENABLE 1001				//启用程序按钮
#define IDC_BTN_DISABLE 1002			//禁用程序按钮
#define IDC_CRB_ENTER 1003				//enter统一触发格式单选框[1]
#define IDC_CRB_REALTIME 1004			//标点实时检查触发格式单选框[1]
#define IDC_CRB_MINIMIZE 1005			//点击关闭按钮退出程序单选框[2]
#define IDC_CRB_EXIT 1006				//点击关闭按钮最小化托盘单选框[2]
#define IDC_EDIT_SUFFIX 1007			//自定义后缀编辑框
#define IDC_BTN_DEWEIGHT_ON 1008		//开启后缀去重单选[3]
#define IDC_BTN_DEWEIGHT_OFF 1009		//关闭后缀去重单选[3]
#define IDC_STA_FUNSTATE 1010			//程序启用状态
#define IDC_BTN_SUFFIXMEOW 1011			//选择喵后缀
#define IDC_BTN_MOODWORD_JUMP_ON 1012	//开启单字语气词跳过
#define IDC_BTN_MOODWORD_JUMP_OFF 1013	//关闭单字语气词跳过
#define IDC_CRB_SHORTCUTS 1016			//快捷键触发格式单选框[1]
#define IDC_HOTKEY 1017					//快捷键输入框
#define IDC_BTN_ADDSETTINGS 1018		//判定设置按钮
#define IDC_BTN_REPSETTINGS 1019		//替换设置按钮
#define IDC_BTN_ABOUT 1020				//关于

#define IDC_BTN_DEBUG 1015			//Debug控制台开关
bool debtnflag = 0;					//debug按钮flag


bool g_DEBUG = 0;		//控制台，0为关闭，1为启用

bool isdiscard = 0;		//是否丢弃保存

HANDLE g_hOutput = 0;//接收标准输出句柄
HWND g_hWnd = NULL;
HHOOK g_hHook = NULL;

char g_suffix[4096] = "喵";		//自定义后缀
bool g_Program = 0;				//是否启用程序：0为禁用，1为启用
int g_trigger = 0;				//功能触发：0为enter触发，1为标点触发，2为快捷键触发
bool g_close = 0;				//程序关闭：0为退出程序，1为最小化托盘
bool g_deweigiht = 0;			//是否开启喵字去重：0为关闭，1为开启
bool g_jumpmood = 0;			//是否开启单字语气词跳过：0为关闭，1为开启

WORD g_hotkeyVk = 0;			//快捷键的虚拟键码
WORD g_hotkeyMod = 0;			//修时间的虚拟键码

bool ISFIRST = 1;				//是第一次打开吗？

NOTIFYICONDATA g_nid = { 0 };
bool TeordFlag = 1;		//4002文字与功能：0为禁用程序，1为启用程序
HMENU g_hMenu = NULL;

void SetFont(HWND hWnd){
	/*
	HFONT hFont = CreateFont(
		25,
		0, 0, 0, FW_NORMAL,
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		"MS Shell Dlg"
	);
	*/
	HFONT hDefaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT lf = { 0 };
	GetObject(hDefaultFont, sizeof(LOGFONT), &lf);
	//memcpy(lf.lfFaceName, "Microsoft YaHei", strlen("Microsoft YaHei") + 1);

	//Microsoft YaHei
	#ifdef UNICODE
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Microsoft YaHei");
	#else
		strcpy_s(lf.lfFaceName, LF_FACESIZE, "Microsoft YaHei");
	#endif

	lf.lfHeight = 25;
	HFONT hNewFont = CreateFontIndirect(&lf);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hNewFont, TRUE);
}
void RefreshUIFromGlobals(HWND hWnd)
{
	bool isMeow = (strcmp(g_suffix, "喵") == 0);

	SendMessage(GetDlgItem(hWnd, IDC_BTN_SUFFIXMEOW),BM_SETCHECK, isMeow ? BST_CHECKED : BST_UNCHECKED, 0);

	SetWindowTextA(GetDlgItem(hWnd, IDC_EDIT_SUFFIX), g_suffix);
	EnableWindow(GetDlgItem(hWnd, IDC_EDIT_SUFFIX), !isMeow);

	WORD hk = MAKEWORD(g_hotkeyVk, g_hotkeyMod);
	SendMessage(GetDlgItem(hWnd, IDC_HOTKEY), HKM_SETHOTKEY, hk, 0);
}
void registerHotkey(HWND hWnd) {
	UnregisterHotKey(hWnd, 1);

	if (g_hotkeyVk == 0) {
		DebugLog("[INFO]没有设置快捷键\n");
		return;
	}

	if (g_hotkeyMod == 0) {
		DebugLog("[INFO]快捷键必须包含Ctrl/Alt/Shift/Win修饰键\n");
		MessageBox(hWnd, "快捷键必须包含Ctrl、Alt、Shift或Win键！", "提示", MB_ICONWARNING);
		return;
	}

	if (RegisterHotKey(hWnd, 1, g_hotkeyMod, g_hotkeyVk)) {
		char log[256];
		sprintf_s(log, "[INFO]全局热键注册成功: vk=%d, mod=%d\n", g_hotkeyVk, g_hotkeyMod);
		DebugLog(log);
	} else {
		DebugLog("[ERROR]全局热键注册失败，可能与其他程序冲突\n");
		MessageBox(hWnd, "快捷键注册失败，请更换其他组合键！", "错误", MB_ICONERROR);

		SendMessage(GetDlgItem(hWnd, IDC_HOTKEY), HKM_SETHOTKEY, 0, 0);
		g_hotkeyVk = 0;
		g_hotkeyMod = 0;
		UnregisterHotKey(hWnd, 1);
	}
}
void RemoveMeowMark(){
	Sleep(50);
	if (!OpenClipboard(NULL)){
		DebugLog("[DEBUG]剪贴板打开失败");
		return;
	}
	EmptyClipboard();
	CloseClipboard();
	DebugLog("[DEBUG]剪贴板已清空\n");
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam){
	switch (msgID){
		case WM_CREATE:
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONGREY16)));

			g_nid.cbSize = sizeof(NOTIFYICONDATA);
			g_nid.hWnd = hWnd;
			g_nid.uID = ID_TRAYICON;
			g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			g_nid.uCallbackMessage = WM_TRAYICON;
			g_nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONWHITE16));
			strcpy_s(g_nid.szTip, "打字时可以在句尾加喵的程序喵");
			Shell_NotifyIcon(NIM_ADD, &g_nid);
			
			PostMessage(hWnd, WM_INITIALIZE, 0, 0);

			DebugLog("\n");
			Sleep(10);
			DebugLog("                            _ooOoo_  \n");
			Sleep(10);
			DebugLog("                           o8888888o  \n");
			Sleep(10);
			DebugLog("                           88\" . \"88  \n");
			Sleep(10);
			DebugLog("                           (| -_- |)  \n");
			Sleep(10);
			DebugLog("                            O\\ = /O  \n");
			Sleep(10);
			DebugLog("                        ____/`---'\\____  \n");
			Sleep(10);
			DebugLog("                      .   ' \\\\| |// `.  \n");
			Sleep(10);
			DebugLog("                       / \\\\||| : |||// \\  \n");
			Sleep(10);
			DebugLog("                     / _||||| -:- |||||- \\  \n");
			Sleep(10);
			DebugLog("                       | | \\\\\\ - /// | |  \n");
			Sleep(10);
			DebugLog("                     | \\_| ''\\---/'' | |  \n");
			Sleep(10);
			DebugLog("                      \\ .-\\__ `-` ___/-. /  \n");
			Sleep(10);
			DebugLog("                   ___`. .' /--.--\\ `. . __  \n");
			Sleep(10);
			DebugLog("                .\"\" '< `.___\\_<|>_/___.' >'\"\".  \n");
			Sleep(10);
			DebugLog("               | | : `- \\`.;`\\ _ /`;.`/ - ` : | |  \n");
			Sleep(10);
			DebugLog("                 \\ \\ `-. \\_ __\\ /__ _/ .-` / /  \n");
			Sleep(10);
			DebugLog("         ======`-.____`-.___\\_____/___.-`____.-'======  \n");
			Sleep(10);
			DebugLog("                            `=---='  \n");
			Sleep(10);
			DebugLog("  \n");
			Sleep(10);
			DebugLog("         .............................................  \n");
			Sleep(10);
			DebugLog("                  佛祖保佑             永无BUG \n");
			Sleep(10);
			DebugLog("          佛曰:  \n");
			Sleep(10);
			DebugLog("                  写字楼里写字间，写字间里程序员；  \n");
			Sleep(10);
			DebugLog("                  程序人员写程序，又拿程序换酒钱。  \n");
			Sleep(10);
			DebugLog("                  酒醒只在网上坐，酒醉还来网下眠；  \n");
			Sleep(10);
			DebugLog("                  酒醉酒醒日复日，网上网下年复年。  \n");
			Sleep(10);
			DebugLog("                  但愿老死电脑间，不愿鞠躬老板前；  \n");
			Sleep(10);
			DebugLog("                  奔驰宝马贵者趣，公交自行程序员。  \n");
			Sleep(10);
			DebugLog("                  别人笑我忒疯癫，我笑自己命太贱；  \n");
			Sleep(10);
			DebugLog("                  不见满街漂亮妹，哪个归得程序员？  \n");
			Sleep(10);
			DebugLog("\n");
			break;
		case WM_INITIALIZE:
			Load();
			RefreshUIFromGlobals(hWnd);

			SendMessage(GetDlgItem(hWnd, IDC_BTN_SUFFIXMEOW), BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT_SUFFIX), FALSE);
			SendMessage(GetDlgItem(hWnd, IDC_CRB_ENTER), BM_SETCHECK, BST_CHECKED, 0);
			SendMessage(GetDlgItem(hWnd, IDC_CRB_EXIT), BM_SETCHECK, BST_CHECKED, 0);
			SendMessage(GetDlgItem(hWnd, IDC_BTN_DEWEIGHT_OFF), BM_SETCHECK, BST_CHECKED, 0);
			SendMessage(GetDlgItem(hWnd, IDC_BTN_MOODWORD_JUMP_OFF), BM_SETCHECK, BST_CHECKED, 0);

			EnableWindow(GetDlgItem(hWnd, IDC_BTN_DISABLE), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_ENABLE), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_HOTKEY), FALSE);

			SetWindowText(GetDlgItem(hWnd, IDC_STA_FUNSTATE), "程序状态：已关闭");

			//strcpy_s(g_suffix, sizeof(g_suffix), "喵");
			g_Program = 0;
			g_trigger = 0;
			g_close = 0;
			g_deweigiht = 0;
			g_jumpmood = 0;

			if (ISFIRST){
				MessageBox(hWnd, "一些Q&A：\n"
								"Q：为什么我在使用“标点实时触发”功能时不会检测我在“判定设置”自己定义的标点？\n"
								"A：在这一触发模式中，程序只会检测内置的常用的标点，关于您的设置程序仅仅是判断您设置的标点是不是含有检测到的。您可能使用了特殊的字符作为标点。\n"
								"\n此弹窗仅会在每个版本的第一次打开时显示。\n（如果程序找不到Config.save或发现Config.save内的第一个数字是1的话下次就会再显示一遍qwq）\nTip：每个版本都会更新一些常见的问题哦~"
								, "Q&A", MB_ICONQUESTION | MB_OK);
				ISFIRST = 0;
			}
			break;
		case WM_DESTROY:
			UninstallHook();
			Save();
			RemoveMeowMark();
			PostQuitMessage(0);
			break;
		case WM_TIMER:
			if (wParam == TIMER_REALTIME_CHECK) {
				KillTimer(hWnd, TIMER_REALTIME_CHECK);
				DebugLog("[INFO]实时触发，执行添加\n");
				AddSuffix_EnterKey();
			}
			break;
		case WM_SYSCOMMAND:
			if (wParam == SC_CLOSE){
				if (g_close == 0){
					int res = MessageBox(hWnd, "真的要退出吗？", "退出", MB_YESNO | MB_ICONQUESTION);
					if (res == IDYES){
						Shell_NotifyIcon(NIM_DELETE, &g_nid);
						break;
					} else if (res == IDNO){
						return 0;
					}
				} else if (g_close == 1){
					DebugLog("[INFO]窗口已缩小至托盘\n");
					ShowWindow(hWnd, SW_HIDE);
					return 0;
				}
			}
			break;
		case WM_HOTKEY: {
			if (wParam == 1 && g_Program && g_trigger == 2) {
				DebugLog("[INFO]快捷键触发，执行添加后缀\n");
				AddSuffix_EnterKey();
			}
			break;
		}
		case WM_COMMAND:{
			WORD id = LOWORD(wParam);
			WORD code = HIWORD(wParam);
			if (id == IDC_EDIT_SUFFIX && code == EN_CHANGE) {
				GetDlgItemText(hWnd, IDC_EDIT_SUFFIX, g_suffix, sizeof(g_suffix));

				DebugLog("suffix:");
				DebugLog(g_suffix);
				DebugLog("\n");

				break;
			} else if (id == IDC_HOTKEY && code == EN_CHANGE){
				HWND hHotkey = GetDlgItem(hWnd, IDC_HOTKEY);
				DWORD hotkey = SendMessage(hHotkey, HKM_GETHOTKEY, 0, 0);
				g_hotkeyVk = LOBYTE(LOWORD(hotkey));
				g_hotkeyMod = HIBYTE(LOWORD(hotkey));

				char log[256];
				sprintf_s(log, "[DEBUG]快捷键已保存: vk=%d, mod=%d\n", g_hotkeyVk, g_hotkeyMod);
				DebugLog(log);

				if (g_trigger == 2 && g_Program) {
					registerHotkey(hWnd);
				}
			}
			switch (LOWORD(wParam)){
				case IDC_BTN_SUFFIXMEOW:{
					HWND hCheck = GetDlgItem(hWnd, IDC_BTN_SUFFIXMEOW);
					bool isChecked = SendMessage(hCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
					if (isChecked){
						strcpy_s(g_suffix, sizeof(g_suffix), "喵");

						DebugLog("suffix:");
						DebugLog(g_suffix);
						DebugLog("\n");

						EnableWindow(GetDlgItem(hWnd, IDC_EDIT_SUFFIX), FALSE);
					} else{
						GetDlgItemText(hWnd, IDC_EDIT_SUFFIX, g_suffix, sizeof(g_suffix));

						DebugLog("suffix:");
						DebugLog(g_suffix);
						DebugLog("\n");

						EnableWindow(GetDlgItem(hWnd, IDC_EDIT_SUFFIX), TRUE);
					}
					break;
				}
				case IDC_BTN_ENABLE:
					InstallHook();
					SetWindowText(GetDlgItem(hWnd, IDC_STA_FUNSTATE), "程序状态：已开启");
					g_Program = 1;
					EnableWindow(GetDlgItem(hWnd, IDC_BTN_DISABLE), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_BTN_ENABLE), FALSE);
					TeordFlag = 0;
					EnableWindow(GetDlgItem(hWnd, IDC_BTN_ADDSETTINGS), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_BTN_REPSETTINGS), FALSE);
					
					if (g_trigger == 2) {
						registerHotkey(hWnd);
					}
					break;
				case IDC_BTN_DISABLE:
					UninstallHook();
					UnregisterHotKey(hWnd, 1);
					SetWindowText(GetDlgItem(hWnd, IDC_STA_FUNSTATE), "程序状态：已关闭");
					g_Program = 0;
					EnableWindow(GetDlgItem(hWnd, IDC_BTN_DISABLE), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_BTN_ENABLE), TRUE);
					TeordFlag = 1;
					EnableWindow(GetDlgItem(hWnd, IDC_BTN_ADDSETTINGS), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_BTN_REPSETTINGS), TRUE);
					RemoveMeowMark();
					break;
				case IDC_BTN_DEWEIGHT_ON:
					DebugLog("[INFO]后缀去重已开启\n");
					g_deweigiht = 1;
					break;
				case IDC_BTN_DEWEIGHT_OFF:
					DebugLog("[INFO]后缀去重已关闭\n");
					g_deweigiht = 0;
					break;
				case IDC_CRB_ENTER:
					DebugLog("[INFO]回车统一触发模式已开启\n");
					EnableWindow(GetDlgItem(hWnd, IDC_HOTKEY), FALSE);

					RemoveMeowMark();
					UnregisterHotKey(hWnd, 1);

					g_trigger = 0;
					break;
				case IDC_CRB_REALTIME:
					DebugLog("[INFO]标点实时触发模式已开启\n");
					EnableWindow(GetDlgItem(hWnd, IDC_HOTKEY), FALSE);

					UnregisterHotKey(hWnd, 1);

					g_trigger = 1;
					break;
				case IDC_CRB_SHORTCUTS:
					DebugLog("[INFO]快捷键触发模式已开启\n");
					EnableWindow(GetDlgItem(hWnd, IDC_HOTKEY), TRUE);

					registerHotkey(hWnd);

					RemoveMeowMark();
					g_trigger = 2;
					break;
				case IDC_CRB_MINIMIZE:
					DebugLog("[INFO]程序关闭时最小化托盘\n");
					g_close = 1;
					break;
				case IDC_CRB_EXIT:
					DebugLog("[INFO]程序关闭时直接退出\n");
					g_close = 0;
					break;
				case IDC_BTN_MOODWORD_JUMP_OFF:
					DebugLog("[INFO]单字语气词跳过已关闭\n");
					g_jumpmood = 0;
					break;
				case IDC_BTN_MOODWORD_JUMP_ON:
					DebugLog("[INFO]单字语气词跳过已开启\n");
					g_jumpmood = 1;
					break;
				case IDC_BTN_DEBUG:
					if (debtnflag == 0){
						debtnflag = 1;
						AllocConsole();
						g_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
						g_DEBUG = 1;
					} else if (debtnflag == 1){
						debtnflag = 0;
						FreeConsole();
						g_hOutput = NULL;
						g_DEBUG = 0;
					}
					break;
				case IDC_BTN_ABOUT:
					MessageBox(hWnd, "程序名称：Meow-打字时可以在句尾加喵的程序喵\n程序版本：1.0.0\n程序作者：新旅程/ANJourney\nbilibili：新_旅程\n\n程序使用C++Win32API编写", "关于", MB_OK | MB_ICONINFORMATION);
					break;
				case IDC_BTN_ADDSETTINGS:
					ShowRulesWindow(hWnd);
					break;
				case IDC_BTN_REPSETTINGS:
					ShowReplacesWindow(hWnd);
				case 4001:
					ShowWindow(hWnd, SW_SHOW);
					SetForegroundWindow(hWnd);
					break;
				case 4002:
					if (TeordFlag == 0){
						ModifyMenu(g_hMenu, 4002, MF_BYCOMMAND, 4002, "启用程序");
						UninstallHook();
						SetWindowText(GetDlgItem(hWnd, IDC_STA_FUNSTATE), "程序状态：已关闭");
						g_Program = 0;
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_DISABLE), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_ENABLE), TRUE);
						TeordFlag = 1;
					} else if (TeordFlag == 1){
						ModifyMenu(g_hMenu, 4002, MF_BYCOMMAND, 4002, "禁用程序");
						InstallHook();
						SetWindowText(GetDlgItem(hWnd, IDC_STA_FUNSTATE), "程序状态：已开启");
						g_Program = 1;
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_DISABLE), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_ENABLE), FALSE);
						TeordFlag = 0;
					}
					break;
				case 4003:
					Shell_NotifyIcon(NIM_DELETE, &g_nid);
					UninstallHook();
					PostQuitMessage(0);
					break;
			}	
			break;
		}
		case WM_TRAYICON:
			if (lParam == WM_LBUTTONDBLCLK) {
				ShowWindow(hWnd, SW_SHOW);
				SetForegroundWindow(hWnd);
			}
			if (lParam == WM_RBUTTONUP) {
				POINT pt;
				GetCursorPos(&pt);
				HMENU hMenu = CreatePopupMenu();
				g_hMenu = hMenu;
				AppendMenu(hMenu, MF_STRING, 4001, "显示窗口");
				if (TeordFlag == 0){
					AppendMenu(hMenu, MF_STRING, 4002, "禁用程序");
				} else if (TeordFlag == 1){
					AppendMenu(hMenu, MF_STRING, 4002, "启用程序");
				}
				AppendMenu(hMenu, MF_STRING, 4003, "退出");
				SetForegroundWindow(hWnd);
				TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
				PostMessage(hWnd, WM_NULL, 0, 0);
				DestroyMenu(hMenu);
			}
	}
	return DefWindowProc(hWnd, msgID, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hIns, HINSTANCE hPreIns, LPSTR IpCmdLine, int nCmdShow){
	int argc;
	bool isTop = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv != NULL) {
		for (int i = 1; i < argc; i++) {
			if (wcscmp(argv[i], L"--debug") == 0) {
				g_DEBUG = 1;
			} else if (wcscmp(argv[i], L"--top") == 0){
				isTop = 1;
			} else if (wcscmp(argv[i], L"--discard") == 0){
				isdiscard = 1;
			}
		}
		LocalFree(argv);
	}

	if (g_DEBUG){
		//debug
		AllocConsole();
		g_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	WNDCLASS wc = { 0 };
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.hCursor = NULL;
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONGREY256));;
	wc.hInstance = hIns;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "Main";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);

	HWND hWnd = CreateWindowEx(isTop ? WS_EX_TOPMOST : 0, "Main", "打字时可以在句尾加喵的程序喵 v1.0.0", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, 100, 100, 750, 800, NULL, NULL, hIns, NULL);
	/*CreateWindow(窗口类名称（已注册），窗口标题，窗口样式，位置坐标x，位置坐标y，窗口宽，窗口高，父窗口句柄，窗口菜单名称，该程序实例句柄（hIns），附带信息)*/
	g_hWnd = hWnd;

	CreateWindow("STATIC", "程序状态：已关闭", WS_CHILD | WS_VISIBLE | SS_LEFT, 595, 730, 250, 20, hWnd, (HMENU)IDC_STA_FUNSTATE, NULL, NULL);
	CreateWindow("BUTTON", "启用程序", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_BORDER, 10, 5, 100, 32, hWnd, (HMENU)IDC_BTN_ENABLE, NULL, NULL);
	CreateWindow("BUTTON", "禁用程序", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_BORDER, 112, 5, 100, 32, hWnd, (HMENU)IDC_BTN_DISABLE, NULL, NULL);
	HWND hwST1 = CreateWindow("STATIC", "后缀：", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 85, 180, 32, hWnd, NULL, NULL, NULL);
	HWND hwCB = CreateWindow("BUTTON", "喵", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 142, 84, 250, 32, hWnd, (HMENU)IDC_BTN_SUFFIXMEOW, NULL, NULL);
	HWND hwST2 = CreateWindow("STATIC", "自定义后缀：", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 122, 190, 32, hWnd, NULL, NULL, NULL);
	HWND hwED = CreateWindow("EDIT", g_suffix, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 140, 120, 200, 33, hWnd, (HMENU)IDC_EDIT_SUFFIX, NULL, NULL);
	HWND hwST3 = CreateWindow("STATIC", "后缀触发：", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 172, 100, 32, hWnd, NULL, NULL, NULL);
	HWND hwBCK1 = CreateWindow("BUTTON", "回车统一触发", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 115, 172, 150, 32, hWnd, (HMENU)IDC_CRB_ENTER, NULL, NULL);
	HWND hwBCK2 = CreateWindow("BUTTON", "标点实时触发", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 300, 172, 150, 32, hWnd, (HMENU)IDC_CRB_REALTIME, NULL, NULL);
	HWND hwBCK9 = CreateWindow("BUTTON", "快捷键触发", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 115, 212, 120, 32, hWnd, (HMENU)IDC_CRB_SHORTCUTS, NULL, NULL);
	HWND hwHEY = CreateWindow(HOTKEY_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 250, 212, 250, 32, hWnd, (HMENU)IDC_HOTKEY, NULL, NULL);
	/*↓四个数字第二个参数全部加一百后*/
	CreateWindow("STATIC", "Tips：回车统一触发将会在用户按下回车键时会统一在程序预设的标点前添加后缀；标点实时触发将会实时检查在输入标点时添加后缀；快捷键触发将会在用户按下设定的快捷键时添加后缀", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 259, 650, 40, hWnd, NULL, NULL, NULL);
	HWND hwST4 = CreateWindow("STATIC", "后缀去重：", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 310, 100, 32, hWnd, NULL, NULL, NULL);
	HWND hwBCK3 = CreateWindow("BUTTON", "启用", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 115, 310, 150, 32, hWnd, (HMENU)IDC_BTN_DEWEIGHT_ON, NULL, NULL);
	HWND hwBCK4 = CreateWindow("BUTTON", "禁用", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 300, 310, 150, 32, hWnd, (HMENU)IDC_BTN_DEWEIGHT_OFF, NULL, NULL);
	CreateWindow("STATIC", "Tips：当程序检测到需要加后缀的句子末尾与后缀一致时将不会添加", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 350, 550, 20, hWnd, NULL, NULL, NULL);
	HWND hwST5 = CreateWindow("STATIC", "单字语气词跳过：", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 385, 300, 32, hWnd, NULL, NULL, NULL);
	HWND hwBCK7 = CreateWindow("BUTTON", "启用", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 180, 385, 150, 32, hWnd, (HMENU)IDC_BTN_MOODWORD_JUMP_ON, NULL, NULL);
	HWND hwBCK8 = CreateWindow("BUTTON", "禁用", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 350, 385, 150, 32, hWnd, (HMENU)IDC_BTN_MOODWORD_JUMP_OFF, NULL, NULL);
	CreateWindow("STATIC", "Tips：当程序检测到有特定语气词单独用标点隔开时将不会在该字后添加后缀", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 430, 550, 40, hWnd, NULL, NULL, NULL);
	CreateWindow("BUTTON", "判定设置", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_BORDER, 10, 520, 700, 40, hWnd, (HMENU)IDC_BTN_ADDSETTINGS, NULL, NULL);
	CreateWindow("BUTTON", "文字替换", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_BORDER, 10, 470, 700, 40, hWnd, (HMENU)IDC_BTN_REPSETTINGS, NULL, NULL);
	/*↑四个数字第二个参数全部加一百后*/
	HWND hwST6 = CreateWindow("STATIC", "窗口关闭时：", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 630, 250, 32, hWnd, NULL, NULL, NULL);
	HWND hwBCK5 = CreateWindow("BUTTON", "退出程序", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 10, 660, 250, 32, hWnd, (HMENU)IDC_CRB_EXIT, NULL, NULL);
	HWND hwBCK6 = CreateWindow("BUTTON", "最小化到托盘", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 350, 660, 250, 32, hWnd, (HMENU)IDC_CRB_MINIMIZE, NULL, NULL);

	//CreateWindow("BUTTON", "Debug", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_BORDER, 595, 10, 100, 32, hWnd, (HMENU)IDC_BTN_DEBUG, NULL, NULL);
	CreateWindow("BUTTON", "关于", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_BORDER, 595, 10, 100, 32, hWnd, (HMENU)IDC_BTN_ABOUT, NULL, NULL);

	SetFont(hwST1);
	SetFont(hwED);
	SetFont(hwST2);
	SetFont(hwBCK1);
	SetFont(hwBCK2);
	SetFont(hwST3);
	SetFont(hwCB);
	SetFont(hwST4);
	SetFont(hwBCK3);
	SetFont(hwBCK4);
	SetFont(hwST5);
	SetFont(hwST6);
	SetFont(hwBCK5);
	SetFont(hwBCK6);
	SetFont(hwBCK7);
	SetFont(hwBCK8);
	SetFont(hwBCK9);
	SetFont(hwHEY);

	Load();
	RefreshUIFromGlobals(hWnd);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	if (isTop){
		SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	MSG nMsg = { 0 };
	while (GetMessage(&nMsg, NULL, 0, 0)) {
		TranslateMessage(&nMsg);
		DispatchMessage(&nMsg);
	}
	return 0;
}