#include <Windows.h>
#include <string>
#include <vector>
#include "Hook.h"
#include "Log.h"
#include "variable.h"
#include "ReplaceSettings.h"
#include "Config.h"

static bool g_Shift = false;
wchar_t punctlist[4096] = L"。，、！？；：）】~… —";	//注意倒数第二个空格也算一个标点
wchar_t moodwlist[4096] = L"哦噢哇嚯呵唉嗐嗨嘿哎喂嘻喏哼呸啧哟呕噫嘘吔哈喔啊呀哪啦喽呗啵咯咧啰喃嗳欸唔呣唬呃诶额嗯";
wchar_t banwlist[4096] = L"";

HANDLE BackupClipBoard(){
	if (!OpenClipboard(NULL)){
		DebugLog("[ERROR]Failed to open clipboard for backup!\n");
		return NULL;
	}
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	HANDLE hBackup = NULL;
	if (hData != NULL){
		wchar_t* src = (wchar_t*)GlobalLock(hData);
		if (src != NULL){
			SIZE_T size = (wcslen(src) + 1) * sizeof(wchar_t);
			hBackup = GlobalAlloc(GMEM_MOVEABLE, size);
			if (hBackup != NULL){
				wchar_t* dst = (wchar_t*)GlobalLock(hBackup);
				if (dst != NULL){
					memcpy(dst, src, size);
					GlobalUnlock(hBackup);
					DebugLog("[DEBUG]Clipboard backup successful!\n");
				} else {
					GlobalFree(hBackup);
					hBackup = NULL;
					DebugLog("[ERROR]Failed to lock backup memory");
				}
			} else{
				DebugLog("[ERROR]Failed to allocate backup memory");
			}
			GlobalUnlock(hData);
		}
	} else{
		DebugLog("[DEBUG]Clipboard is empty\n");
	}
	CloseClipboard();
	return hBackup;
	/*
	if (!OpenClipboard(NULL)){
		DebugLog("[ERROR]Failed to open clipboard for backup!\n");
		return NULL;
	}
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	HANDLE hBackup = NULL;
	if (hData != NULL){
		SIZE_T size = GlobalSize(hData);
		hBackup = GlobalAlloc(GMEM_MOVEABLE, size);
		if (hBackup != NULL){
			wchar_t* src = (wchar_t*)GlobalLock(hData);
			wchar_t* dst = (wchar_t*)GlobalLock(hBackup);
			memcpy(dst, src, size);
			GlobalUnlock(hData);
			GlobalUnlock(hBackup);
			DebugLog("[DEBUG]Clipboard backup successful!\n");
		} else{
			DebugLog("[ERROR]Failed to allocate backup memory");
		}
	} else{
		DebugLog("[DEBUG]Clipboard is empty\n");
	}
	CloseClipboard();
	return hBackup;
	*/
}
void RestoreClipBoard(HANDLE hBp){
	if (hBp == NULL){
		DebugLog("[DEBUG]No backup to restore, clearing clipboard\n");
		for (int i = 0; i < 10; ++i) {
			if (OpenClipboard(NULL)) {
				EmptyClipboard();
				CloseClipboard();
				return;
			}
			Sleep(20);
		}
		DebugLog("[ERROR]Failed to clear clipboard\n");
		return;
	}
	if (!OpenClipboard(NULL)){
		DebugLog("[ERROR]Failed to open clipboard for restore\n");
		return;
	}
	Sleep(50);
	EmptyClipboard();
	Sleep(50);
	SIZE_T size = GlobalSize(hBp);
	HANDLE hNew = GlobalAlloc(GMEM_MOVEABLE, size);
	if (hNew != NULL){
		void* src = GlobalLock(hBp);
		void* dst = GlobalLock(hNew);
		if (src && dst){
			memcpy(dst, src, size);
		}
		if (src) GlobalUnlock(hBp);
		if (dst) GlobalUnlock(hNew);

		if (SetClipboardData(CF_UNICODETEXT, hNew) == NULL) {
			char errorlog[512];
			sprintf_s(errorlog, "[ERROR]SetClipboardData failed, error=%d\n", GetLastError());
			DebugLog(errorlog);
			GlobalFree(hNew);
		}
	}
	CloseClipboard();
	DebugLog("[DEBUG]Clipboard restored\n");
	/*
	if (hBp == NULL){
		DebugLog("[DEBUG]No backup to restore\n");
		return;
	}
	if (!OpenClipboard(NULL)){
		DebugLog("[ERROR]Failed to open clipboard for restore\n");
		return;
	}
	Sleep(50);
	EmptyClipboard();
	Sleep(50);

	if (SetClipboardData(CF_UNICODETEXT, hBp) == NULL) {
		char errorlog[512];
		sprintf_s(errorlog, "[ERROR]SetClipboardData failed, error=%d\n", GetLastError());
		DebugLog(errorlog);
		GlobalFree(hBp);
	}
	//SetClipboardData(CF_UNICODETEXT, hBp);
	CloseClipboard();
	DebugLog("[DEBUG]Clipboard restored\n");
	*/
}
std::wstring GetClipBoardText(){
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
		DebugLog("[DEBUG]No Unicode text in clipboard\n");
		return L"";
	}
	if (!OpenClipboard(NULL)){
		DebugLog("[ERROR]Failed to open clipboard for reading\n");
		return L"";
	}
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	if (hData == NULL){
		CloseClipboard();
		DebugLog("[DEBUG]No text data in clipboard\n");
		return L"";
	}
	wchar_t* pT = (wchar_t*)GlobalLock(hData);
	if (pT == NULL){
		CloseClipboard();
		DebugLog("[ERROR]Failed to lock clipboard data\n");
		return L"";
	}
	std::wstring result(pT);
	GlobalUnlock(hData);
	CloseClipboard();
	
	wchar_t log[512];
	wsprintfW(log, L"[DEBUG]Clipboard text read: \"%s\"\n", result.c_str());
	DebugLogW(log);

	return result;
}
void SetClipBoardText(const std::wstring& text){
	if (!OpenClipboard(NULL)){
		DebugLog("[ERROR]Failed to open clipboard for writing\n");
		return;
	}
	EmptyClipboard();
	SIZE_T size = (text.length() + 1) * sizeof(wchar_t);
	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, size);
	if (hData == NULL){
		DebugLog("[ERROR]Failed to allocate clipboard memory\n");
		CloseClipboard();
		return;
	}
	wchar_t* pData = (wchar_t*)GlobalLock(hData);
	if (pData == NULL){
		DebugLog("[ERROR]Failed to lock clipboard memory\n");
		GlobalFree(hData);
		CloseClipboard();
		return;
	}
	wcscpy_s(pData, text.length() + 1, text.c_str());
	GlobalUnlock(hData);
	if (SetClipboardData(CF_UNICODETEXT, hData) == NULL){
		DebugLog("[ERROR]SetClipboardData in SetCilipBoardText failed\n");
		GlobalFree(hData);
	}
	CloseClipboard();

	wchar_t log[512];
	wsprintfW(log, L"[DEBUG]Clipboard text written: \"%s\"\n", text.c_str());
	DebugLogW(log);
}
void SimulateKey(BYTE vk){
	INPUT ip[2] = { 0 };
	ip[0].type = INPUT_KEYBOARD;
	ip[0].ki.wVk = VK_CONTROL;
	ip[0].ki.dwFlags = 0;
	
	ip[1].type = INPUT_KEYBOARD;
	ip[1].ki.wVk = vk;
	ip[1].ki.dwFlags = 0;

	SendInput(2, ip, sizeof(INPUT));

	ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
	ip[0].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(2, ip, sizeof(INPUT));
}
bool HasBanword(const std::wstring& text) {
	if (wcslen(banwlist) == 0){
		return 0;
	}
	for (size_t i = 0; i < text.length(); i++) {
		wchar_t ch = text[i];
		for (const wchar_t* p = banwlist; *p; p++) {
			if (ch == *p) {
				wchar_t log[512];
				wsprintfW(log, L"[DEBUG]检测到屏蔽词: '%c'\n", ch);
				DebugLogW(log);
				return 1;
			}
		}
	}
	return 0;
}
std::wstring GetTrailingPunct(const std::wstring& text) {
	if (text.empty()){
		return L"";
	}
	std::wstring str = text;

	std::wstring punct;
	for (int i = (int)str.length() - 1; i >= 0; i--) {
		bool isPunct = false;
		for (const wchar_t* p = punctlist; *p; p++) {
			if (str[i] == *p){
				isPunct = true; 
				break; 
			}
		}
		wchar_t log[256];
		wsprintfW(log, L"[DEBUG]GetTrailingPunct: 检查字符 '%c' (0x%04X), isPunct=%d\n",text[i], text[i], isPunct);
		DebugLogW(log);

		if (isPunct) {
			punct = text[i] + punct;
		} else {
			break;
		}
	}
	wchar_t log2[256];
	wsprintfW(log2, L"[DEBUG]GetTrailingPunct: 结果 = \"%s\"\n", punct.c_str());
	DebugLogW(log2);
	return punct;
}
bool IsMoodWord(wchar_t ch) {
	for (const wchar_t* p = moodwlist; *p; p++) {
		if (ch == *p) {
			return true;
		}
	}
	return false;
}
bool ShouldSkipMoodWord(const std::wstring& text) {
	std::wstring str = text;
	while (!str.empty() && iswspace(str.back())) {
		str.pop_back();
	}
	while (!str.empty()) {
		wchar_t ch = str.back();
		bool isPunct = false;
		for (const wchar_t* p = punctlist; *p; p++) {
			if (ch == *p) {
				isPunct = true;
				break;
			}
		}
		if (isPunct) {
			str.pop_back();
		} else {
			break;
		}
	}
	if (str.length() != 1) {
		return false;
	}
	for (const wchar_t* p = moodwlist; *p; p++) {
		if (str[0] == *p) {
			return true;
		}
	}
	return false;
}
std::vector<std::wstring> SplitText(std::wstring text){
	std::vector<std::wstring> result;
	std::wstring token;
	size_t i = 0;
	const size_t len = text.size();
	while (i < len){
		wchar_t ch = text[i];
		bool isPunct = false;
		for (const wchar_t* p = punctlist; *p; p++){
			if (ch == *p){
				isPunct = true;
				break;
			}
		}
		if (!isPunct){
			token.push_back(ch);
			i++;
		} else{
			while (i < len){
				wchar_t c2 = text[i];
				bool p2 = false;
				for (const wchar_t* p = punctlist; *p; p++){
					if (c2 == *p){ 
						p2 = true; break; 
					}
				}
				if (!p2){
					break;
				}
				token.push_back(c2);
				i++;
			}
			if (!token.empty()){
				result.push_back(token);
				token.clear();
			}
		}
	}
	if (!token.empty()){
		result.push_back(token);
	}
	return result;
}
std::wstring GetSuffix(){
	wchar_t suffixW[Nchar] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, g_suffix, -1, suffixW, Nchar);
	return std::wstring(suffixW);
}
bool RemovePunctuaionAndDuplicateCheck_Suffix(const std::wstring& text){
	std::wstring str = text;
	while (!str.empty() && iswspace(str.back())){
		str.pop_back();
	}
	std::wstring punct = GetTrailingPunct(str);
	if (!punct.empty()) {
		str = str.substr(0, str.length() - punct.length());
	}
	std::wstring suffix = GetSuffix();
	if (suffix.empty()){
		return 0;
	}
	if (str.length() < suffix.length()){
		return 0;
	}
	size_t pos = str.length() - suffix.length();
	return str.compare(pos, suffix.length(), suffix) == 0;
}
std::wstring SetSuffix(const std::wstring& text){
	std::wstring result = text;
	while (!result.empty()){
		wchar_t ch = result.back();
		if (ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r'){
			result.pop_back();
		} else{
			break;
		}
	}
	if (result.empty()){
		return text;
	}

	if (HasBanword(result)) {
		DebugLog("[DEBUG]检测到屏蔽词，不加后缀\n");
		return text;
	}

	result = ApplyReplaceRules(result);

	if (g_jumpmood && ShouldSkipMoodWord(result)) {
		DebugLog("[DEBUG]单字语气词，不加后缀\n");
		return result;
	}
	if (g_deweigiht && RemovePunctuaionAndDuplicateCheck_Suffix(result)){
		DebugLog("[DEBUG]Text already has suffix, skipping\n");
		return result;
	}

	std::wstring suffix = GetSuffix();
	std::wstring punct = GetTrailingPunct(result);

	wchar_t log[512];
	wsprintfW(log, L"[DEBUG]SetSuffix: 原始文本 = \"%s\"\n", result.c_str());
	DebugLogW(log);
	wsprintfW(log, L"[DEBUG]SetSuffix: 检测到的连续标点 = \"%s\"\n", punct.c_str());
	DebugLogW(log);

	if (!punct.empty()) {
		std::wstring beforeP = result.substr(0, result.length() - punct.length());
		result = beforeP + suffix + punct;
		DebugLog("[DEBUG]有连续标点，在标点前插入\n");
	} else {
		result = result + suffix;
		DebugLog("[DEBUG]无标点，末尾添加\n");
	}
	return result;
}
std::wstring ProcessText(const std::wstring& text) {
	std::vector<std::wstring> parts = SplitText(text);
	std::wstring result;
	for (size_t i = 0; i < parts.size(); i++) {
		std::wstring processed = SetSuffix(parts[i]);
		result += processed;
	}
	return result;
}
void AttachThreadInput2(DWORD attachTo, BOOL attach) {
	__try {
		::AttachThreadInput(GetCurrentThreadId(), attachTo, attach);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		DebugLog("[ERROR]AttachThreadInput 异常\n");
	}
}
void AddSuffix_EnterKey(){
	DebugLog("[DEBUG]AddSuffix_EnterKey called\n");

	keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_RWIN, 0, KEYEVENTF_KEYUP, 0);
	Sleep(30);

	HWND hFg = GetForegroundWindow();
	if (!hFg) {
		DebugLog("[ERROR]没有前台窗口\n");
		return;
	}
	HWND hFocus = GetFocus();
	DWORD focusThreadId = GetWindowThreadProcessId(hFocus, NULL);
	DWORD currentThreadId = GetCurrentThreadId();
	if (focusThreadId != currentThreadId) {
		AttachThreadInput2(focusThreadId, TRUE);
	}

	HANDLE hBp = BackupClipBoard();

	SetForegroundWindow(hFg);
	SetFocus(hFocus);
	Sleep(100);

	SimulateKey('A');
	Sleep(30);
	DebugLog("[DEBUG]Ctrl+A sent\n");
	SimulateKey('C');
	Sleep(50);
	DebugLog("[DEBUG]Ctrl+C sent\n");

	std::wstring oriText = GetClipBoardText();

	wchar_t log[512];
	wsprintfW(log, L"[DEBUG]Original text: \"%s\"\n", oriText.c_str());
	DebugLogW(log);

	if (oriText.empty()){
		DebugLog("[DEBUG]Clipboard is empty, restoring and exiting\n");
		RestoreClipBoard(hBp);
		if (hBp){
			GlobalFree(hBp);
		}
		return;
	}
	std::wstring resText = ProcessText(oriText);

	wsprintfW(log, L"[DEBUG]处理后文本: \"%s\"\n", resText.c_str());
	DebugLogW(log);

	if (resText == oriText){
		DebugLog("[DEBUG]Text unchanged, restoring\n");
		SimulateKey('V');
		Sleep(50);
		RestoreClipBoard(hBp);
		if (hBp){
			GlobalFree(hBp);
		}
		return;
	}

	wsprintfW(log, L"[DEBUG]New text: \"%s\"\n", resText.c_str());
	DebugLogW(log);

	SetClipBoardText(resText);
	Sleep(30);
	DebugLog("[DEBUG]Clipboard updated\n");
	SimulateKey('V');
	Sleep(50);
	DebugLog("[DEBUG]Ctrl+V sent\n");
	
	Sleep(50);

	RestoreClipBoard(hBp);
	if (hBp){
		GlobalFree(hBp);
	}
	DebugLog("[DEBUG]Clipboard restored\n");
	
	char output[512] = { 0 };
	sprintf_s(output, "[OUTPUT]%S\n", resText.c_str());
	DebugLog(output);

	if (focusThreadId != currentThreadId) {
		AttachThreadInput2(focusThreadId, FALSE);
	}
}
bool IsPunctuationKey(DWORD vkCode) {
	bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

	wchar_t chEn = 0;
	wchar_t chCn = 0;

	switch (vkCode) {
	case '1':
		chEn = shiftDown ? L'!' : L'1';
		chCn = shiftDown ? L'！' : L'1';
		break;
	case '2':
		chEn = shiftDown ? L'@' : L'2';
		chCn = shiftDown ? L'@' : L'2';
		break;
	case VK_OEM_PERIOD:
		chEn = shiftDown ? L'>' : L'.';
		chCn = shiftDown ? L'>' : L'。';
		break;
	case VK_OEM_COMMA:
		chEn = shiftDown ? L'<' : L',';
		chCn = shiftDown ? L'<' : L'，';
		break;
	case VK_OEM_1:
		chEn = shiftDown ? L':' : L';';
		chCn = shiftDown ? L'：' : L'；';
		break;
	case VK_OEM_2:
		chEn = shiftDown ? L'?' : L'/';
		chCn = shiftDown ? L'？' : L'、';
		break;
	case VK_OEM_3:
		chEn = shiftDown ? L'~' : L'`';
		chCn = shiftDown ? L'~' : L'·';
		break;
	case VK_OEM_4:
		chEn = shiftDown ? L'{' : L'[';
		chCn = shiftDown ? L'【' : L'【';
		break;
	case VK_OEM_5:
		chEn = shiftDown ? L'|' : L'\\';
		chCn = shiftDown ? L'|' : L'、';
		break;
	case VK_OEM_6:
		chEn = shiftDown ? L'}' : L']';
		chCn = shiftDown ? L'】' : L'】';
		break;
	case VK_OEM_7:
		chEn = shiftDown ? L'"' : L'\'';
		chCn = shiftDown ? L'"' : L'\'';
		break;
	case VK_SPACE:
		return 0;
	default:
		return 0;
	}

	for (const wchar_t* p = punctlist; *p; p++) {
		if (chEn == *p) {
			DebugLog("[DEBUG]IsPunct: 命中英文标点\n");
			return 1;
		}
	}

	for (const wchar_t* p = punctlist; *p; p++) {
		if (chCn == *p) {
			DebugLog("[DEBUG]IsPunct: 命中中文标点\n");
			return 1;
		}
	}
	return 0;
}
bool InstallHook(){
	g_hHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
	if (g_hHook == NULL){
		DebugLog("[ERROR]Keyboard hook installation failed!\n");
		return 0;
	}
	DebugLog("[INFO]Keyboard hook installed successfully!\n");
	return 1;
}
void UninstallHook(){
	if (g_hHook != NULL){
		UnhookWindowsHookEx(g_hHook);
		g_hHook = NULL;
		DebugLog("[INFO]The keyboard hook has been uninstalled.\n");
	}
}
LRESULT CALLBACK KeyboardProc(int Code, WPARAM wParam, LPARAM lParam){
	if (Code < 0){
		return CallNextHookEx(g_hHook, Code, wParam, lParam);
	}
	if (g_Program == 0) {
		return CallNextHookEx(g_hHook, Code, wParam, lParam);
	}
	if (wParam == WM_KEYDOWN){
		KBDLLHOOKSTRUCT* pKeyb = (KBDLLHOOKSTRUCT*)lParam;
		char log[256];
		sprintf_s(log, "[DEBUG]KeyDown: vkCode=%d, g_Program=%d, g_trigger=%d\n", pKeyb->vkCode, g_Program, g_trigger);
		DebugLog(log);
		if (g_trigger == 2 && g_hotkeyVk != 0) {
			if (wParam == WM_KEYDOWN) {
				KBDLLHOOKSTRUCT* pKeyb = (KBDLLHOOKSTRUCT*)lParam;

				if (pKeyb->vkCode == g_hotkeyVk) {
					bool ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
					bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
					bool altDown = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
					bool winDown = (GetAsyncKeyState(VK_LWIN) & 0x8000) != 0 || (GetAsyncKeyState(VK_RWIN) & 0x8000) != 0;

					bool ctrlMatch = ((g_hotkeyMod & 0x02) != 0) == ctrlDown;
					bool shiftMatch = ((g_hotkeyMod & 0x04) != 0) == shiftDown;
					bool altMatch = ((g_hotkeyMod & 0x01) != 0) == altDown;
					bool winMatch = ((g_hotkeyMod & 0x08) != 0) == winDown;

					if (ctrlMatch && shiftMatch && altMatch && winMatch) {
						DebugLog("[INFO]快捷键被触发（键盘回调检测到）\n");
						AddSuffix_EnterKey();
						Sleep(500);
					}
				}
			}
		}
		if (g_trigger == 1) {
			if (IsPunctuationKey(pKeyb->vkCode)) {
				DebugLog("[INFO]检测到标点按键，启动定时器\n");
				KillTimer(g_hWnd, TIMER_REALTIME_CHECK);
				SetTimer(g_hWnd, TIMER_REALTIME_CHECK, 30, NULL);
			}
		}
		if (pKeyb->vkCode == VK_RETURN){
			if (g_trigger == 1){
				return CallNextHookEx(g_hHook, Code, wParam, lParam);
			}
			AddSuffix_EnterKey();
		}
	}
	if (wParam == WM_KEYUP){
		KBDLLHOOKSTRUCT* pKeyb = (KBDLLHOOKSTRUCT*)lParam;
	}
	return CallNextHookEx(g_hHook, Code, wParam, lParam);
}