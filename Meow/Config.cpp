#include<Windows.h>
#include "Config.h"
#include "Hook.h"

void Save(){
	if (isdiscard){
		return;
	}
	std::ofstream file("Config.save");
	if (!file.is_open()){
		return;
	}
	file << ISFIRST << std::endl;

	file << g_suffix << std::endl;

	file << g_hotkeyVk << std::endl;
	file << g_hotkeyMod << std::endl;

	char buf[4096];
	WideCharToMultiByte(CP_ACP, 0, punctlist, -1, buf, 4096, NULL, NULL);
	file << buf << std::endl;
	WideCharToMultiByte(CP_ACP, 0, moodwlist, -1, buf, 4096, NULL, NULL);
	file << buf << std::endl;
	WideCharToMultiByte(CP_ACP, 0, banwlist, -1, buf, 4096, NULL, NULL);
	file << buf << std::endl;

	file << g_ReplaceRules.size() << std::endl;
	for (size_t i = 0; i < g_ReplaceRules.size(); i++) {
		WideCharToMultiByte(CP_ACP, 0, g_ReplaceRules[i].source, -1, buf, 4096, NULL, NULL);
		file << buf << std::endl;

		WideCharToMultiByte(CP_ACP, 0, g_ReplaceRules[i].target, -1, buf, 4096, NULL, NULL);
		file << buf << std::endl;
	}
	file.close();
}
void Load(){
	if (isdiscard){
		return;
	}
	std::ifstream file("Config.save");
	if (!file.is_open()) {
		return;
	}

	file >> ISFIRST;

	std::string line;

	std::getline(file, line);
	strcpy_s(g_suffix, line.c_str());

	std::getline(file, line);
	g_hotkeyVk = (WORD)atoi(line.c_str());
	std::getline(file, line);
	g_hotkeyMod = (WORD)atoi(line.c_str());

	std::getline(file, line);
	MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, punctlist, 4096);
	std::getline(file, line);
	MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, moodwlist, 4096);
	std::getline(file, line);
	MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, banwlist, 4096);

	std::getline(file, line);
	int count = atoi(line.c_str());
	g_ReplaceRules.clear();
	for (int i = 0; i < count; i++) {
		ReplaceRule rule;
		std::getline(file, line);
		MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, rule.source, 4096);
		std::getline(file, line);
		MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, rule.target, 4096);

		g_ReplaceRules.push_back(rule);
	}
	file.close();
}