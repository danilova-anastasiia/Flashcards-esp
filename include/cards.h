#ifndef CARDS_H
#define CARDS_H

static cJSON *loadJson(const char *deck_path);
int deleteCardByIndex(const char *deck_path, int index);
int getCardCount(const char *deck_path);
int createDeck(const char *deck_path, const char *deck_title);
int deleteDeckFile(const char *deck_path);
void deckClose(void);
int deckOpen(const char *deck_path);
int deckSize(void);
void deckShuffle(void);
const char* deckTerm(int pos);
const char* deckDefinition(int pos);
int addCard(const char* deck_path, const char *term, const char *definition);
int deckList(const char* dir_path, char deckName[][67], int max);

#endif
