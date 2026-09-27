#pragma once

#include <Windows.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <commctrl.h>

#define TIMER_REALTIME_CHECK 2002

const int Nchar = 4096;

extern HHOOK g_hHook;
extern bool g_Program;
extern int g_trigger;
extern bool g_deweigiht;
extern bool g_jumpmood;
extern char g_suffix[Nchar];
extern HANDLE g_hOutput;
extern HWND g_hWnd;
extern WORD g_hotkeyVk;
extern WORD g_hotkeyMod;

extern wchar_t punctlist[4096];
extern wchar_t moodwlist[4096];
extern wchar_t banwlist[4096];

struct ReplaceRule {
	wchar_t source[4096];
	wchar_t target[4096];
	bool ENorDI;
};

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

bool InstallHook();
void UninstallHook();

void AddSuffix_EnterKey();

HANDLE BackupClipBoard();
void RestoreClipBoard(HANDLE hBp);
std::wstring GetClipBoardText();
void SetClipBoardText(const std::wstring& text);

void SimulateKey(BYTE vk);
bool IsPunctuationKey(DWORD vkCode);

bool RemovePunctuaionAndDuplicateCheck_Suffix(const std::wstring& text);
std::wstring SetSuffix(const std::wstring& text);
std::wstring GetSuffix();
std::vector<std::wstring> SplitText(std::wstring text);
bool IsPunctuationKey(DWORD vkCode);
bool HasBanword(const std::wstring& text);
std::wstring ProcessText(const std::wstring& text);