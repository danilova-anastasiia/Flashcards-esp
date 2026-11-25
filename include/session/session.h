#ifndef SESSION_H
#define SESSION_H

#define B_COUNT 5
typedef enum {
    STATE_DECKS,
    STATE_STUDY,
    STATE_DONE
} statuses;

enum {B_LEFT, B_MID, B_RIGHT, B_WEB };

uint8_t scanButtons(void);
void refreshDecks(bool force);
void refreshDecksIfDue(void);
int decksCount(void);
void showDecks(void);
statuses startStudy(void);
void studyButtonsInit(void);
int studyStart(const char *deck_path);
void studyStop(void);
void flip(void);
void studyAnswer(bool known);
void studyButtonsPoll(void);
bool studyIsFinished(void);
int studyKnown(void);
int studyTotal(void);

#endif
