#pragma once

#include<Windows.h>
#include<string>
#include"Log.h"
#include"Hook.h"
#include"resource.h"
#include"variable.h"

extern bool g_replace;
extern std::vector<ReplaceRule> g_ReplaceRules;

INT_PTR CALLBACK ReplaceProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam);
void ShowReplacesWindow(HWND hWnd);
std::wstring ApplyReplaceRules(const std::wstring& text);