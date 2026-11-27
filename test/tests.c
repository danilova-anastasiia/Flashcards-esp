#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cJSON.h"

int deleteCardByIndex(const char *deck_path, int index);
int getCardCount(const char *deck_path);
int createDeck(const char *deck_path, const char *deck_title);
int deleteDeckFile(const char *deck_path);
void deckClose(void);
int deckOpen(const char *deck_path);
int deckSize(void);
void deckShuffle(void);
const char *deckTerm(int pos);
const char *deckDefinition(int pos);
int addCard(const char *deck_path, const char *term, const char *definition);
int deckList(const char *dir_path, char deckName[][67], int max);

static int tests_run = 0;

#define RUN_TEST(fn) do { \
    printf("Running %s...\n", #fn); \
    fn(); \
    tests_run++; \
} while (0)

static void remove_if_exists(const char *path) {
    remove(path);
}

static void test_create_count_and_delete_deck(void) {
    const char *path = "test_deck_create.json";
    remove_if_exists(path);

    assert(createDeck(path, "Spanish Basics") == 1);
    assert(createDeck(path, "Duplicate") == 0);
    assert(getCardCount(path) == 0);

    assert(deleteDeckFile(path) == 1);
    assert(deleteDeckFile(path) == 0);
    assert(deleteDeckFile(NULL) == 0);
    assert(getCardCount(path) == 0);
}

static void test_add_and_delete_cards(void) {
    const char *path = "test_deck_cards.json";
    remove_if_exists(path);

    assert(createDeck(path, "C Terms") == 1);
    assert(addCard(path, "Pointer", "Variable holding a memory address") == 1);
    assert(addCard(path, "Malloc", "Dynamic heap memory allocator") == 1);
    assert(addCard(path, "Struct", "Composite data type") == 1);
    assert(getCardCount(path) == 3);

    assert(deleteCardByIndex(path, -1) == 0);
    assert(deleteCardByIndex(path, 3) == 0);
    assert(getCardCount(path) == 3);

    assert(deleteCardByIndex(path, 1) == 1);
    assert(getCardCount(path) == 2);

    assert(deckOpen(path) == 2);
    assert(strcmp(deckTerm(0), "Pointer") == 0);
    assert(strcmp(deckDefinition(0), "Variable holding a memory address") == 0);
    assert(strcmp(deckTerm(1), "Struct") == 0);
    assert(strcmp(deckDefinition(1), "Composite data type") == 0);
    deckClose();

    remove_if_exists(path);
}

static void test_deck_open_accessors_and_close(void) {
    const char *empty_path = "test_deck_empty.json";
    const char *missing_path = "test_deck_does_not_exist.json";
    remove_if_exists(empty_path);

    assert(deckOpen(missing_path) == 0);
    assert(deckSize() == 0);
    assert(strcmp(deckTerm(0), "") == 0);
    assert(strcmp(deckDefinition(0), "") == 0);

    assert(createDeck(empty_path, "Empty Deck") == 1);
    assert(deckOpen(empty_path) == 0);
    assert(deckSize() == 0);
    assert(strcmp(deckTerm(0), "") == 0);

    deckClose();
    assert(deckSize() == 0);

    remove_if_exists(empty_path);
}

static void test_deck_shuffle_preserves_multiset(void) {
    const char *path = "test_deck_shuffle.json";
    remove_if_exists(path);

    assert(createDeck(path, "Shuffle Test") == 1);
    for (int i = 0; i < 6; i++) {
        char term[32];
        char def[32];
        snprintf(term, sizeof(term), "Term_%d", i);
        snprintf(def, sizeof(def), "Def_%d", i);
        assert(addCard(path, term, def) == 1);
    }

    assert(deckOpen(path) == 6);
    assert(deckSize() == 6);

    srand(42);
    deckShuffle();

    assert(deckSize() == 6);
    int seen[6] = {0};
    for (int pos = 0; pos < deckSize(); pos++) {
        const char *t = deckTerm(pos);
        const char *d = deckDefinition(pos);
        int idx = -1;
        assert(sscanf(t, "Term_%d", &idx) == 1);
        assert(idx >= 0 && idx < 6);
        assert(seen[idx] == 0);
        seen[idx] = 1;

        char expected_def[32];
        snprintf(expected_def, sizeof(expected_def), "Def_%d", idx);
        assert(strcmp(d, expected_def) == 0);
    }

    assert(strcmp(deckTerm(-1), "") == 0);
    assert(strcmp(deckTerm(6), "") == 0);
    assert(strcmp(deckDefinition(-1), "") == 0);
    assert(strcmp(deckDefinition(6), "") == 0);

    deckClose();
    remove_if_exists(path);
}

static void test_deck_list(void) {
    const char *f1 = "tmp_unit_alpha.json";
    const char *f2 = "tmp_unit_beta.json";
    const char *f3 = "tmp_unit_notes.txt";
    const char *f4 = ".json";

    remove_if_exists(f1);
    remove_if_exists(f2);
    remove_if_exists(f3);
    remove_if_exists(f4);

    assert(createDeck(f1, "Alpha") == 1);
    assert(createDeck(f2, "Beta") == 1);
    FILE *txt = fopen(f3, "wb");
    assert(txt != NULL);
    fputs("hello", txt);
    fclose(txt);
    FILE *short_json = fopen(f4, "wb");
    assert(short_json != NULL);
    fputs("{}", short_json);
    fclose(short_json);

    char names[64][67];
    int count = deckList(".", names, 64);
    assert(count >= 2);

    int found_alpha = 0;
    int found_beta = 0;
    int found_txt = 0;
    int found_short = 0;

    for (int i = 0; i < count; i++) {
        if (strcmp(names[i], f1) == 0) found_alpha = 1;
        if (strcmp(names[i], f2) == 0) found_beta = 1;
        if (strcmp(names[i], f3) == 0) found_txt = 1;
        if (strcmp(names[i], f4) == 0) found_short = 1;
    }

    assert(found_alpha == 1);
    assert(found_beta == 1);
    assert(found_txt == 0);
    assert(found_short == 0);

    assert(deckList(NULL, names, 64) == 0);
    assert(deckList(".", NULL, 64) == 0);
    assert(deckList(".", names, 0) == 0);
    assert(deckList("nonexistent_dir_xyz_676", names, 64) == 0);

    remove_if_exists(f1);
    remove_if_exists(f2);
    remove_if_exists(f3);
    remove_if_exists(f4);
}

int main(void) {
    RUN_TEST(test_create_count_and_delete_deck);
    RUN_TEST(test_add_and_delete_cards);
    RUN_TEST(test_deck_open_accessors_and_close);
    RUN_TEST(test_deck_shuffle_preserves_multiset);
    RUN_TEST(test_deck_list);

    printf("All %d test suites passed!\n", tests_run);
    return 0;
}
