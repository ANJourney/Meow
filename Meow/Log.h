#pragma once
#include <Windows.h>
#include <string>

extern HANDLE g_hOutput;
extern bool g_DEBUG;

inline void DebugLog(const char* msg) {
	if (g_DEBUG){
		WriteConsole(g_hOutput, msg, strlen(msg), NULL, NULL);
	}
}
inline void DebugLog(const std::string& msg) {
	if (g_DEBUG){
		DebugLog(msg.c_str());
	}
}
inline void DebugLogW(const wchar_t* msg) {
	if (g_DEBUG){
		WriteConsoleW(g_hOutput, msg, (DWORD)wcslen(msg), NULL, NULL);
	}
}
inline void ShowFocusInfoEx() {
	char log[512];

	// 获取前台窗口
	HWND hForeground = GetForegroundWindow();
	sprintf_s(log, "[DEBUG]前台窗口句柄: 0x%p\n", hForeground);
	DebugLog(log);

	// 获取焦点控件
	HWND hFocus = GetFocus();
	sprintf_s(log, "[DEBUG]焦点控件句柄: 0x%p\n", hFocus);
	DebugLog(log);

	// 获取焦点窗口的类名
	if (hFocus) {
		wchar_t className[256] = { 0 };
		GetClassNameW(hFocus, className, 256);
		char classLog[512];
		sprintf_s(classLog, "[DEBUG]焦点控件类名: %S\n", className);
		DebugLog(classLog);
	}

	// 获取前台窗口的类名
	if (hForeground) {
		wchar_t className[256] = { 0 };
		GetClassNameW(hForeground, className, 256);
		char classLog[512];
		sprintf_s(classLog, "[DEBUG]前台窗口类名: %S\n", className);
		DebugLog(classLog);
	}
}