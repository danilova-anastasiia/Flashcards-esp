#ifndef SESSION_H
#define SESSION_H

int studyStart(const char *deck_path);
void studyStop(void);
void flip(void);
void studyAnswer(bool known);
void studyButtonsPoll(void);
bool studyIsFinished(void);
int studyKnown(void);
int studyTotal(void);

#endif
