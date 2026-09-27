#pragma once

#include<Windows.h>
#include<string>
#include<vector>
#include"Hook.h"
#include"RulesSettings.h"
#include"variable.h"

void log(){
	DebugLog("ReplaceLIST:\n");
	for (const auto& rule : g_ReplaceRules) {
		wchar_t log[512];
		wsprintfW(log, L"=>source: %s, target: %s, Enable: %d\n", rule.source, rule.target, (int)rule.ENorDI);
		DebugLogW(log);
	}
	DebugLog("END\n");
}

void ReplaceRule_Add(const wchar_t* source, const wchar_t* target, const bool EnOrDi) {
	ReplaceRule rule;
	wcscpy_s(rule.source, source);
	wcscpy_s(rule.target, target);
	rule.ENorDI = EnOrDi;
	g_ReplaceRules.push_back(rule);
	log();
}

void ReplaceRule_Delete(int index) {
	if (index >= 0 && index < (int)g_ReplaceRules.size()) {
		g_ReplaceRules.erase(g_ReplaceRules.begin() + index);
	}
	log();
}

void ReplaceRule_Up(int index) {
	if (index > 0 && index < (int)g_ReplaceRules.size()) {
		std::swap(g_ReplaceRules[index], g_ReplaceRules[index - 1]);
	}
	log();
}

void ReplaceRule_Down(int index) {
	if (index >= 0 && index < (int)g_ReplaceRules.size() - 1) {
		std::swap(g_ReplaceRules[index], g_ReplaceRules[index + 1]);
	}
	log();
}

void ReplaceRules_Clear() {
	g_ReplaceRules.clear();
	log();
}