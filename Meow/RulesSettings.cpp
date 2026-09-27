#include<Windows.h>
#include<string>
#include"RulesSettings.h"
#include"variable.h"
#include "Config.h"

wchar_t dfutPunct[4096] = L"¡££¬¡¢£¡£¿£»£º£©¡¿~¡­ ¡ª";	//×¢Òâµ¹ÊýµÚ¶þ¸ö¿Õ¸ñÒ²ËãÒ»¸ö±êµã
wchar_t dfutMoodw[4096] = L"Å¶àÞÍÛàëºÇ°¦†ãàËºÙ°¥Î¹ÎûßöºßÅÞßõÓ´Å»àæÐê…½¹þà¸°¡Ñ½ÄÄÀ²à¶ßÂà£¿©ßÖ†ªà«àÈšGßí…Þ»£ßÀÚÀ¶îàÅ";
wchar_t dfutBanw[4096] = L"";

wchar_t newpunct[4096];
wchar_t newmoodword[4096];
wchar_t newbanword[4096];

void SetFontD(HWND hWnd){
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

	lf.lfHeight = 22;
	HFONT hNewFont = CreateFontIndirect(&lf);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hNewFont, TRUE);
}

INT_PTR CALLBACK SettingsProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam){
	switch (msgID){
		case WM_INITDIALOG:{
			RECT rcDlg;
			GetWindowRect(hWnd, &rcDlg);
			int x = (GetSystemMetrics(SM_CXSCREEN) - (rcDlg.right - rcDlg.left)) / 2;
			int y = (GetSystemMetrics(SM_CYSCREEN) - (rcDlg.bottom - rcDlg.top)) / 2;
			SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			
			HWND hTip = GetDlgItem(hWnd, IDC_STATIC_TIP1);
			SetFontD(hTip);

			SetDlgItemTextW(hWnd, IDC_EDIT_POLIST, punctlist);
			SetDlgItemTextW(hWnd, IDC_EDIT_MOODWLIST, moodwlist);
			SetDlgItemTextW(hWnd, IDC_EDIT_MACHLIST, banwlist);
			break;
		}
		case WM_CLOSE:
			EndDialog(hWnd, IDCANCEL);
			break;
		case WM_COMMAND:{
			int id = LOWORD(wParam);
			int code = HIWORD(wParam);
			if (id == IDC_EDIT_TTEXT && code == EN_CHANGE){
				wchar_t isNULL[4096];
				GetDlgItemTextW(hWnd, IDC_EDIT_TTEXT, isNULL, 4096);
				if (wcslen(isNULL) == 0) {
					SetDlgItemTextW(hWnd, IDC_STATIC_TRES, L"´Ë´¦½«ÓÃÓÚÏÔÊ¾½á¹û");
					break;
				}
				wchar_t text[4096];
				GetDlgItemTextW(hWnd, IDC_EDIT_TTEXT, text, 4096);
				std::wstring result = ProcessText(text);
				SetDlgItemTextW(hWnd, IDC_STATIC_TRES, result.c_str());
				break;
			}
			switch (LOWORD(wParam)){
				case IDC_BTN_SAVE:
					GetDlgItemTextW(hWnd, IDC_EDIT_POLIST, newpunct, 4096);
					wcscpy_s(punctlist, 4096, newpunct);
					
					GetDlgItemTextW(hWnd, IDC_EDIT_MOODWLIST, newmoodword, 4096);
					wcscpy_s(moodwlist, 4096, newmoodword);

					GetDlgItemTextW(hWnd, IDC_EDIT_MACHLIST, newbanword, 4096);
					wcscpy_s(banwlist, 4096, newbanword);
					
					Initialization_Variable();
					EndDialog(hWnd, IDOK);
					break;
				case IDC_BTN_USE:
					GetDlgItemTextW(hWnd, IDC_EDIT_POLIST, newpunct, 4096);
					wcscpy_s(punctlist, 4096, newpunct);

					GetDlgItemTextW(hWnd, IDC_EDIT_MOODWLIST, newmoodword, 4096);
					wcscpy_s(moodwlist, 4096, newmoodword);

					GetDlgItemTextW(hWnd, IDC_EDIT_MACHLIST, newbanword, 4096);
					wcscpy_s(banwlist, 4096, newbanword);

					Initialization_Variable();
					break; 
				case IDC_BTN_CANCEL:
					EndDialog(hWnd, IDCANCEL);
					break;
				case IDC_BTN_TCLEAR:
					SetDlgItemTextW(hWnd, IDC_EDIT_TTEXT, L"");
					break;
				case IDC_BTN_DEFAULT1:
					SetDlgItemTextW(hWnd, IDC_EDIT_POLIST, dfutPunct);
					wcscpy_s(punctlist, 4096, dfutPunct);

					Initialization_Variable();
					break;
				case IDC_BTN_DEFAULT2:
					SetDlgItemTextW(hWnd, IDC_EDIT_MOODWLIST, dfutMoodw);
					wcscpy_s(moodwlist, 4096, dfutMoodw);

					Initialization_Variable();
					break;
				case IDC_BTN_DEFAULT3:
					SetDlgItemTextW(hWnd, IDC_EDIT_MACHLIST, dfutBanw);
					wcscpy_s(banwlist, 4096, dfutBanw);

					Initialization_Variable();
					break;
			}
			break;
		}

	}
	return 0;
}
void ShowRulesWindow(HWND hWnd) {
	DebugLog("IDD_DIALOG1ÉèÖÃÅÐ¶¨´°¿ÚÒÑÏÔÊ¾\n");
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd,SettingsProc);
}