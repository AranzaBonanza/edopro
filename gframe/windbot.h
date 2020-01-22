#ifndef WINDBOT_H
#define WINDBOT_H

#include <set>
#include <string>
#include "text_types.h"

namespace ygo {

struct WindBot {
	std::wstring name;
	std::wstring deck;
	int difficulty;
	std::set<int> masterRules;

	bool Launch(int port, bool chat = true, int hand = 0) const;

	// Can be made static
	unsigned short version;
	path_string executablePath;
};

}

#endif
