#include "config.h"
#include "game.h"
#include "netserver.h"
#include "data_manager.h"
#include "deck_manager.h"
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

unsigned short aServerPort;
HostInfo game_info;

void Game::MainServerLoop() {
	deckManager.LoadLFList();
	LoadExpansionDB();
	dataManager.LoadDB("cards.cdb");
	if(dataManager._datas.empty())
		return;
	aServerPort = NetServer::StartServer(aServerPort);
	NetServer::InitDuel(game_info);
	printf("%u\n", aServerPort);
	fflush(stdout);
	while(NetServer::net_evbase) {
#ifdef WIN32
		Sleep(200);
#else
		usleep(200000);
#endif
	}
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
std::vector<unsigned char> Game::LoadScript(const std::string& _name, int& slen) {
	std::vector<unsigned char> buffer;
	buffer.clear();
	slen = 0;
	std::ifstream script;
#ifdef _WIN32
	std::wstring name = BufferIO::DecodeUTF8s(_name);
#else
	std::string name = name;
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
	slen = buffer.size();
	return buffer;
}
std::vector<unsigned char> Game::PreLoadScript(void* pduel, const std::string& script_name) {
	int len = 0;
	auto buf = LoadScript(script_name, len);
	preload_script(pduel, (char*)script_name.c_str(), 0, len, (char*)buf.data());
	return buf;
}
void* Game::SetupDuel(uint32 seed) {
	set_script_reader((script_reader)ScriptReader);
	set_card_reader((card_reader)DataManager::CardReader);
	set_message_handler((message_handler)MessageHandler);
	void* pduel = create_duel(seed);
	PreLoadScript(pduel, "constant.lua");
	PreLoadScript(pduel, "utility.lua");
	return pduel;
}
byte* Game::ScriptReader(const char* script_name, int* slen) {
	static std::vector<unsigned char> buffer;
	buffer = mainGame->LoadScript(script_name, *slen);
	return buffer.data();
}
int Game::MessageHandler(void* fduel, int type) {
	char msgbuf[1024];
	get_log_message(fduel, (byte*)msgbuf);
	mainGame->AddDebugMsg(msgbuf);
	return 0;
}
void Game::PopulateResourcesDirectories() {
	script_dirs.push_back(TEXT("./expansions/script/"));
	script_dirs.push_back(TEXT("./script/"));
}


}
