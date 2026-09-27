#pragma once

#include<Windows.h>
#include<string>
#include"Log.h"
#include"Hook.h"
#include"resource.h"

extern wchar_t dfutPunct[4096];
extern wchar_t dfutMoodw[4096];
extern wchar_t dfutBanw[4096];

INT_PTR CALLBACK SettingsProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam);
void ShowRulesWindow(HWND hWnd);