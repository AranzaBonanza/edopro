#include "config.h"
#include "game.h"
#include "netserver.h"
#include "data_manager.h"
#include "deck_manager.h"
#include "dllinterface.h"
#include "replay.h"
#include <sstream>
#include <fstream>

#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#endif

unsigned short PRO_VERSION = 0x1348;

namespace ygo {

Game* mainGame;

int Game::MainServerLoop(const std::string& corepath) {
	deckManager.LoadLFList();
	LoadExpansionDB();
	dataManager.LoadDB("cards.cdb");
	if(dataManager._datas.empty())
		return EXIT_FAILURE;
	PopulateResourcesDirectories();
#ifdef YGOPRO_BUILD_DLL
	if(corepath.size())
		if(!(ocgcore = LoadOCGcore(corepath + "/")))
			return EXIT_FAILURE;
	else if(!(ocgcore = LoadOCGcore("./")) && !(ocgcore = LoadOCGcore("./expansions/")))
		return EXIT_FAILURE;
#endif
	NetServer::StartServer();
#ifdef YGOPRO_BUILD_DLL
	UnloadCore(ocgcore);
#endif
	return EXIT_SUCCESS;
}
void Game::LoadExpansionDB() {
	auto files = Utils::FindfolderFiles(L"./expansions/", { L"cdb" }, 2);
	for(auto& file : files)
		dataManager.LoadDB(BufferIO::EncodeUTF8s(L"./expansions/" + file));
}
void Game::AddDebugMsg(const std::string& msg) {
	fprintf(stderr, "%s\n", msg);
}
int Game::GetMasterRule(uint32 param, uint32 forbiddentypes, int* truerule) {
	switch(param) {
	case MASTER_RULE_1: {
		if (truerule)
			*truerule = 1;
		if (forbiddentypes == MASTER_RULE_1_FORB)
			return 1;
	}
	case MASTER_RULE_2: {
		if (truerule)
			*truerule = 2;
		if (forbiddentypes == MASTER_RULE_2_FORB)
			return 2;
	}
	case MASTER_RULE_3: {
		if (truerule)
			*truerule = 3;
		if (forbiddentypes == MASTER_RULE_3_FORB)
			return 3;
	}
	case MASTER_RULE_4: {
		if (truerule)
			*truerule = 4;
		if (forbiddentypes == MASTER_RULE_4_FORB)
			return 4;
	}
	default: {
		if (truerule)
			*truerule = 5;
		if ((param & DUEL_PZONE) && (param & DUEL_SEPARATE_PZONE) && (param & DUEL_EMZONE))
			return 5;
		else if(param & DUEL_EMZONE)
			return 4;
		else if (param & DUEL_PZONE)
			return 3;
		else
			return 2;
	}
	}
}
std::vector<char> Game::LoadScript(const std::string& _name) {
	std::vector<char> buffer;
	std::ifstream script;
#ifdef _WIN32
	std::wstring name = BufferIO::DecodeUTF8s(_name);
#else
	std::string name = _name;
#endif
	for(auto& path : script_dirs) {
		script.open(path + name, std::ifstream::binary);
		if(script.is_open())
			break;
	}
	if(!script.is_open()) {
		script.open(name, std::ifstream::binary);
		if(!script.is_open())
			return buffer;
	}
	buffer.insert(buffer.begin(), std::istreambuf_iterator<char>(script), std::istreambuf_iterator<char>());
	return buffer;
}
bool Game::LoadScript(OCG_Duel pduel, const std::string& script_name) {
	auto buf = LoadScript(script_name);
	return buf.size() && OCG_LoadScript(pduel, buf.data(), buf.size(), script_name.c_str());
}
OCG_Duel Game::SetupDuel(OCG_DuelOptions opts) {
	opts.cardReader = (OCG_DataReader)&DataManager::CardReader;
	opts.payload1 = &dataManager;
	opts.scriptReader = (OCG_ScriptReader)&ScriptReader;
	opts.payload2 = this;
	opts.logHandler = (OCG_LogHandler)&MessageHandler;
	opts.payload3 = this;
	OCG_Duel pduel = nullptr;
	OCG_CreateDuel(&pduel, opts);
	LoadScript(pduel, "constant.lua");
	LoadScript(pduel, "utility.lua");
	return pduel;
}
int Game::ScriptReader(void* payload, OCG_Duel duel, const char* name) {
	return static_cast<Game*>(payload)->LoadScript(duel, name);
}
void Game::MessageHandler(void* payload, const char* string, int type) {
	static_cast<Game*>(payload)->AddDebugMsg(string);
}
void Game::PopulateResourcesDirectories() {
	script_dirs.push_back(TEXT("./expansions/script/"));
	script_dirs.push_back(TEXT("./script/"));
}


}
