#include<Windows.h>
#include<string>
#include"ReplaceSettings.h"
#include"variable.h"
#include"RaplaceRulesVectorUse.h"
#include "Config.h"

bool g_replace = 0;
bool IStextrevise = 0;
bool EnorDi = 1;
std::vector<ReplaceRule> g_ReplaceRules;

int lastSel = -1;

std::wstring ApplyReplaceRules(const std::wstring& text) {
	if (!g_replace) {
		return text;
	}

	if (g_ReplaceRules.empty()) {
		return text;
	}

	std::wstring result = text;

	for (const auto& rule : g_ReplaceRules) {
		bool Isen = rule.ENorDI;
		if (Isen){
			std::wstring src = rule.source;
			std::wstring tgt = rule.target;
			if (src.empty()){
				continue;
			}
			size_t pos = 0;
			while ((pos = result.find(src, pos)) != std::wstring::npos) {
				result.replace(pos, src.length(), tgt);
				pos += tgt.length();
			}
		}
	}

	wchar_t log[512];
	wsprintfW(log, L"[DEBUG]替换后文本: \"%s\"\n", result.c_str());
	DebugLogW(log);

	return result;
}

INT_PTR CALLBACK ReplaceProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam){
	switch (msgID){
		case WM_INITDIALOG:{
			RECT rcDlg;
			GetWindowRect(hWnd, &rcDlg);
			int x = (GetSystemMetrics(SM_CXSCREEN) - (rcDlg.right - rcDlg.left)) / 2;
			int y = (GetSystemMetrics(SM_CYSCREEN) - (rcDlg.bottom - rcDlg.top)) / 2;
			SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			if (g_replace){
				CheckDlgButton(hWnd, IDC_BTN_TITLE, BST_CHECKED);
			} else if (g_replace){
				CheckDlgButton(hWnd, IDC_BTN_TITLE, BST_UNCHECKED);
			}

			for (const auto& rule : g_ReplaceRules) {
				wchar_t result[8195];
				if (rule.ENorDI){
					swprintf(result, 8195, L"[启用]“%s” → “%s”", rule.source, rule.target);
				} else{
					swprintf(result, 8195, L"[禁用]“%s” → “%s”", rule.source, rule.target);
				}
				
				SendMessageW(GetDlgItem(hWnd, IDC_LIST), LB_ADDSTRING, 0, (LPARAM)result);
			}

			SendMessage(GetDlgItem(hWnd, IDC_RABTN_EN), BM_SETCHECK, BST_CHECKED, 0);
			EnorDi = 1;

			if ((IsDlgButtonChecked(hWnd, IDC_BTN_TITLE) == BST_CHECKED) && g_replace){
				EnableWindow(GetDlgItem(hWnd, IDC_LIST), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_EDIT1_OLD), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_EDIT2_NEW), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_ADD), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTUP), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTCLEAR), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTDOWN), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_RABTN_EN), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_RABTN_DIS), TRUE);
			} else {
				EnableWindow(GetDlgItem(hWnd, IDC_LIST), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_EDIT1_OLD), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_EDIT2_NEW), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_ADD), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTUP), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTCLEAR), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTDOWN), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_RABTN_EN), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_RABTN_DIS), FALSE);

			}
			break;
		}
		case WM_CLOSE:
			EndDialog(hWnd, IDCANCEL);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)){
				case IDC_BTN_TITLE:
					if ((IsDlgButtonChecked(hWnd, IDC_BTN_TITLE) == BST_CHECKED)){
						EnableWindow(GetDlgItem(hWnd, IDC_LIST), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_EDIT1_OLD), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_EDIT2_NEW), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_ADD), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTUP), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTCLEAR), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTDOWN), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_RABTN_EN), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_RABTN_DIS), TRUE);
						g_replace = 1;
					} else {
						EnableWindow(GetDlgItem(hWnd, IDC_LIST), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_EDIT1_OLD), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_EDIT2_NEW), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_ADD), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTUP), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTCLEAR), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_BTN_LISTDOWN), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_RABTN_EN), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_RABTN_DIS), FALSE);
						g_replace = 0;
					}
					break;
				case IDC_BTN_LISTCLEAR:
					ReplaceRules_Clear();
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_RESETCONTENT, 0, 0);
					DebugLog("[INFO]列表已清空\n");
					break;
				case IDC_BTN_LISTUP:{
					int sel = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCURSEL, 0, 0);
					if (sel <= 0){
						break;
					}

					ReplaceRule_Up(sel);

					wchar_t text[4096];
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETTEXT, sel, (LPARAM)text);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_DELETESTRING, sel, 0);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_INSERTSTRING, sel - 1, (LPARAM)text);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_SETCURSEL, sel - 1, 0);
					break;
				}
				case IDC_BTN_LISTDOWN:{
					int count = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCOUNT, 0, 0);
					int sel2 = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCURSEL, 0, 0);
					if (sel2 < 0 || sel2 >= count-1){
						break;
					}

					ReplaceRule_Down(sel2);

					wchar_t text2[4096];
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETTEXT, sel2, (LPARAM)text2);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_DELETESTRING, sel2, 0);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_INSERTSTRING, sel2 + 1, (LPARAM)text2);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_SETCURSEL, sel2+1, 0);
					break;
				}
				case IDC_BTN_DEL: {
					int sel3 = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCURSEL, 0, 0);
					if (sel3 == LB_ERR) {
						MessageBox(hWnd, "请先选中要删除的项目", "提示", MB_OK | MB_ICONWARNING);
						break;
					}

					SetDlgItemText(hWnd, IDC_BTN_ADD, "添加判定");
					SetDlgItemTextW(hWnd, IDC_EDIT1_OLD, L"");
					SetDlgItemTextW(hWnd, IDC_EDIT2_NEW, L"");
					CheckDlgButton(hWnd, IDC_RABTN_EN, BST_CHECKED);
					CheckDlgButton(hWnd, IDC_RABTN_DIS, BST_UNCHECKED);
					IStextrevise = 0;
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_SETCURSEL, -1, 0);
					lastSel = -1;
					ReplaceRule_Delete(sel3);

					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_DELETESTRING, sel3, 0);
					DebugLog("[INFO]已删除选中项\n");
					break;
				}
				case IDC_LIST: {
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						int curSel = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCURSEL, 0, 0);
						if (curSel == lastSel && curSel != -1) {
							SetDlgItemText(hWnd, IDC_BTN_ADD, "添加判定");
							SetDlgItemTextW(hWnd, IDC_EDIT1_OLD, L"");
							SetDlgItemTextW(hWnd, IDC_EDIT2_NEW, L"");
							CheckDlgButton(hWnd, IDC_RABTN_EN, BST_CHECKED);
							CheckDlgButton(hWnd, IDC_RABTN_DIS, BST_UNCHECKED);
							IStextrevise = 0;
							SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_SETCURSEL, -1, 0);
							lastSel = -1;
						} else {
							lastSel = curSel;

							const auto& rule = g_ReplaceRules[curSel];
							SetDlgItemTextW(hWnd, IDC_EDIT1_OLD, rule.source);
							SetDlgItemTextW(hWnd, IDC_EDIT2_NEW, rule.target);
							if (rule.ENorDI) {
								CheckDlgButton(hWnd, IDC_RABTN_EN, BST_CHECKED);
								CheckDlgButton(hWnd, IDC_RABTN_DIS, BST_UNCHECKED);
								EnorDi = 1;
							} else {
								CheckDlgButton(hWnd, IDC_RABTN_EN, BST_UNCHECKED);
								CheckDlgButton(hWnd, IDC_RABTN_DIS, BST_CHECKED);
								EnorDi = 0;
							}
							SetDlgItemText(hWnd, IDC_BTN_ADD, "修改判定");
							IStextrevise = 1;
						}
					}
					break;
				}
				case IDC_BTN_ADD:{
					int reviseSel = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCURSEL, 0, 0);
					if (reviseSel == LB_ERR) {
						IStextrevise = 0;
					}
					bool isen = 1;
					wchar_t isens[16];
					if (IsDlgButtonChecked(hWnd, IDC_RABTN_EN) == BST_CHECKED) {
						isen = 1;
						swprintf(isens, 16, L"%s", L"启用");
					} else{
						isen = 0;
						swprintf(isens, 16, L"%s", L"禁用");
					}
					wchar_t newtext[4096];
					wchar_t oldtext[4096];
					GetDlgItemTextW(hWnd, IDC_EDIT1_OLD, oldtext, 4096);
					GetDlgItemTextW(hWnd, IDC_EDIT2_NEW, newtext, 4096);

					if (!wcslen(newtext) || !wcslen(oldtext)) {
						MessageBox(hWnd, "输入框请填写完整", "提示", MB_OK | MB_ICONWARNING);
						break;
					}
					if (IStextrevise == 0){
						ReplaceRule_Add(oldtext, newtext, isen);

						wchar_t result[8195];
						swprintf(result, 8195, L"[%s]“%s” → “%s”", isens, oldtext, newtext);

						SendMessageW(GetDlgItem(hWnd, IDC_LIST), LB_ADDSTRING, 0, (LPARAM)result);

						SetDlgItemTextW(hWnd, IDC_EDIT1_OLD, L"");
						SetFocus(GetDlgItem(hWnd, IDC_EDIT1_OLD));
						SetDlgItemTextW(hWnd, IDC_EDIT2_NEW, L"");
						SetFocus(GetDlgItem(hWnd, IDC_EDIT2_NEW));

						DebugLog("[INFO]已添加: ");
						DebugLogW(result);
						DebugLog("\n");
					} else if (IStextrevise == 1){
						int reviseSel = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCURSEL, 0, 0);
						SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_DELETESTRING, reviseSel, 0);

						bool isen = 1;
						wchar_t isens[16];
						if (IsDlgButtonChecked(hWnd, IDC_RABTN_EN) == BST_CHECKED) {
							isen = 1;
							swprintf(isens, 16, L"%s", L"启用");
						} else{
							isen = 0;
							swprintf(isens, 16, L"%s", L"禁用");
						}

						wchar_t NewTextbuffer[4096];
						wchar_t OldTextbuffer[4096];
						GetDlgItemTextW(hWnd, IDC_EDIT1_OLD, OldTextbuffer, 4096);
						GetDlgItemTextW(hWnd, IDC_EDIT2_NEW, NewTextbuffer, 4096);

						if (reviseSel < 0 || reviseSel >= (int)g_ReplaceRules.size()) {
							MessageBox(hWnd, "选中的项目无效！", "提示", MB_OK | MB_ICONWARNING);
							break;
						}

						auto& rule = g_ReplaceRules[reviseSel];
						wcscpy_s(rule.source, OldTextbuffer);
						wcscpy_s(rule.target, NewTextbuffer);
						rule.ENorDI = isen;

						wchar_t revResult[8195];
						swprintf(revResult, 8195, L"[%s]“%s” → “%s”", isens, OldTextbuffer, NewTextbuffer);
						SendMessageW(GetDlgItem(hWnd, IDC_LIST), LB_INSERTSTRING, reviseSel, (LPARAM)revResult);
						SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_SETCURSEL, reviseSel, 0);

						IStextrevise = 1;
						SetDlgItemTextW(hWnd, IDC_EDIT1_OLD, L"");
						SetFocus(GetDlgItem(hWnd, IDC_EDIT1_OLD));
						SetDlgItemTextW(hWnd, IDC_EDIT2_NEW, L"");
						SetFocus(GetDlgItem(hWnd, IDC_EDIT2_NEW));
						SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_SETCURSEL, -1, 0);
						IStextrevise = 0;
						SetDlgItemText(hWnd, IDC_BTN_ADD, "添加判定");
						CheckDlgButton(hWnd, IDC_RABTN_EN, BST_CHECKED);
						CheckDlgButton(hWnd, IDC_RABTN_DIS, BST_UNCHECKED);
						lastSel = -1;
					}
					break;
				}
				case IDC_RABTN_EN:{
					if (IStextrevise == 0){
						break;
					}
					int reviseSel1 = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCURSEL, 0, 0);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_DELETESTRING, reviseSel1, 0);
					if (reviseSel1 < 0 || reviseSel1 >= (int)g_ReplaceRules.size()) {
						MessageBox(hWnd, "选中的项目无效！", "提示", MB_OK | MB_ICONWARNING);
						break;
					}

					wchar_t NewTextbuffer[4096];
					wchar_t OldTextbuffer[4096];
					GetDlgItemTextW(hWnd, IDC_EDIT1_OLD, OldTextbuffer, 4096);
					GetDlgItemTextW(hWnd, IDC_EDIT2_NEW, NewTextbuffer, 4096);

					wchar_t revResult[8195];
					swprintf(revResult, 8195, L"[启用]“%s” → “%s”", OldTextbuffer, NewTextbuffer);
					SendMessageW(GetDlgItem(hWnd, IDC_LIST), LB_INSERTSTRING, reviseSel1, (LPARAM)revResult);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_SETCURSEL, reviseSel1, 0);

					auto& rule = g_ReplaceRules[reviseSel1];
					rule.ENorDI = 1;
					break;
				}
				case IDC_RABTN_DIS:{
					if (IStextrevise == 0){
						break;
					}

					int reviseSel2 = SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_GETCURSEL, 0, 0);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_DELETESTRING, reviseSel2, 0);
					if (reviseSel2 < 0 || reviseSel2 >= (int)g_ReplaceRules.size()) {
						MessageBox(hWnd, "选中的项目无效！", "提示", MB_OK | MB_ICONWARNING);
						break;
					}

					wchar_t NewTextbuffer[4096];
					wchar_t OldTextbuffer[4096];
					GetDlgItemTextW(hWnd, IDC_EDIT1_OLD, OldTextbuffer, 4096);
					GetDlgItemTextW(hWnd, IDC_EDIT2_NEW, NewTextbuffer, 4096);

					wchar_t revResult[8195];
					swprintf(revResult, 8195, L"[禁用]“%s” → “%s”", OldTextbuffer, NewTextbuffer);
					SendMessageW(GetDlgItem(hWnd, IDC_LIST), LB_INSERTSTRING, reviseSel2, (LPARAM)revResult);
					SendMessage(GetDlgItem(hWnd, IDC_LIST), LB_SETCURSEL, reviseSel2, 0);

					auto& rule = g_ReplaceRules[reviseSel2];
					rule.ENorDI = 0;
					break;
				}
			}
	}
	return 0;
}
void ShowReplacesWindow(HWND hWnd) {
	DebugLog("IDD_DIALOG2文字替换窗口已显示\n");
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hWnd, ReplaceProc);
}