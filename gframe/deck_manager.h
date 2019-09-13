#ifndef DECKMANAGER_H
#define DECKMANAGER_H

#include "config.h"
#include "client_card.h"
#include <unordered_map>
#include <vector>

namespace ygo {

struct LFList {
	unsigned int hash;
	std::wstring listName;
	std::unordered_map<int, int> content;
	bool whitelist;
};
struct Deck {
	std::vector<CardDataC*> main;
	std::vector<CardDataC*> extra;
	std::vector<CardDataC*> side;
	Deck() {}
	Deck(const Deck& ndeck) {
		main = ndeck.main;
		extra = ndeck.extra;
		side = ndeck.side;
	}
	void clear() {
		main.clear();
		extra.clear();
		side.clear();
	}
};

class DeckManager {
public:
	std::vector<LFList> _lfList;

	void LoadLFListSingle(const std::wstring& path);
	void LoadLFList();
	int CheckDeck(Deck& deck, int lfhash, bool allow_ocg, bool allow_tcg, bool doubled, int forbiddentypes = 0, bool is_speed = false);
	int TypeCount(std::vector<CardDataC*> cards, int type);
	int LoadDeck(Deck& deck, int* dbuf, int mainc, int sidec, int mainc2 = 0, int sidec2 = 0);
	int LoadDeck(Deck& deck, std::vector<int> mainlist, std::vector<int> sidelist);
	bool LoadSide(Deck& deck, int* dbuf, int mainc, int sidec);
};

extern DeckManager deckManager;

}

#endif //DECKMANAGER_H
