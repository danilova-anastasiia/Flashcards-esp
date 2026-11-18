#include <stdio.h>
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static cJSON *deck_root = NULL; 
static cJSON *deck_cards = NULL;
static int *order = NULL;
static int deck_count = 0;

static cJSON *loadJson(const char *deck_path) {
    FILE *file = fopen(deck_path, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long sizeOfFile = ftell(file);
    if (sizeOfFile <= 0) {
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(sizeOfFile + 1);
        if (buffer == NULL) {
            fclose(file);
            return NULL;
        }

    size_t sizeInBytesRead = fread(buffer, 1, sizeOfFile, file);
    buffer[sizeInBytesRead] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (root == NULL) {
        return NULL;
    }

    return root;

}


void deckClose() {

    if (deck_root != NULL) {
        cJSON_Delete(deck_root);
        deck_root = NULL;
    }
    deck_cards = NULL;

    if (order != NULL) {
        free(order);
        order = NULL;
    }

    deck_count = 0;
    return;
}


int deckOpen(const char *deck_path) {
    deckClose();

    deck_root = loadJson(deck_path);
    if (deck_root == NULL) return 0;

    deck_cards = cJSON_GetObjectItemCaseSensitive(deck_root, "cards");
    if (cJSON_IsArray(deck_cards)) {
        deck_count = cJSON_GetArraySize(deck_cards);
    }
    else {
        deckClose();
        return 0;
    }

    if (deck_count <= 0) {
        deckClose();
        return deck_count;
    }

    order = malloc((sizeof(int)*deck_count));
    if (order == NULL) {
        deckClose();
        return 0;
    }

    for (int i = 0; i < deck_count; i++) {
        *(order + i) = i;
    }

    return deck_count;
}

int deckSize() {
    return deck_count;
}


int deleteCardByIndex(const char *deck_path, int index) {
    
    cJSON *root = loadJson(deck_path);
    if (root == NULL) {
        return 0;
    } 

    cJSON *arrayCards = cJSON_GetObjectItemCaseSensitive(root, "cards");
    int size = cJSON_GetArraySize(arrayCards);
    if (index < 0 || index >= size) {
        cJSON_Delete(root);
        return 0;
    }


    if (cJSON_IsArray(arrayCards)) {
        cJSON_DeleteItemFromArray(arrayCards, index);
    }
    else {
        cJSON_Delete(root);
        return 0;
    }


    char *newJson = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (newJson == NULL) {
        return 0;
    }

    FILE *file = fopen(deck_path, "wb");
    if (file == NULL) {
        free(newJson);
        return 0;
    }
    else {
        fputs(newJson, file);
        fclose(file);
    }


    free(newJson); 
    return 1;
}


int getCardCount(const char *deck_path) {
    FILE *file = fopen(deck_path, "r");
    if (file == NULL) {
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long sizeOfFile = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(sizeOfFile + 1);
        if (buffer == NULL) {
            fclose(file);
            return 0;
        }

    size_t sizeInBytesRead = fread(buffer, 1, sizeOfFile, file);
    buffer[sizeInBytesRead] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (root == NULL) {
        return 0;
    }

    int cardCount = 0;
    if (cJSON_IsArray(root)) {
        cardCount = cJSON_GetArraySize(root);
    }
    cJSON_Delete(root);

    return cardCount;

}

void shuffleCardsRandom(const char *deck_path, int count){
    FILE *file = fopen(deck_path, "r");
    if (file == NULL) {
        return;
    }

    fseek(file, 0, SEEK_END);
    long sizeOfFile = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(sizeOfFile + 1);
        if (buffer == NULL) {
            fclose(file);
            return;
        }

    size_t sizeInBytesRead = fread(buffer, 1, sizeOfFile, file);
    buffer[sizeInBytesRead] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (root == NULL) {
        return;
    }

    cJSON *arrayOfCards = cJSON_GetObjectItemCaseSensitive(root, "cards");
    if (cJSON_IsArray(arrayOfCards)) {
        for (int i = count - 1; i > 0; i--) {
            int randomCardIndex = rand() % (i + 1);
            if (randomCardIndex != i) {
                cJSON *cardFromI = cJSON_GetArrayItem(arrayOfCards, i);
                cJSON *cardFromRand = cJSON_GetArrayItem(arrayOfCards, randomCardIndex);

                cJSON *copyFromI = cJSON_Duplicate(cardFromI, 1);
                cJSON *copyFromRand = cJSON_Duplicate(cardFromRand, 1);

                cJSON_ReplaceItemInArray(arrayOfCards, i, copyFromRand);
                cJSON_ReplaceItemInArray(arrayOfCards, randomCardIndex, copyFromI);
            }
        }
    }

    char *newJson = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (newJson == NULL) {
        return;
    }

    file = fopen(deck_path, "w");
    if (file == NULL) {
        free(newJson);
        return;
    }
    else {
        fputs(newJson, file);
        fclose(file);
    }


    free(newJson); 
}

void createDeck(const char *deck_path, const char *deck_title) {
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return;
    }
    cJSON_AddStringToObject(root, "title", deck_title);
    cJSON_AddArrayToObject(root, "cards");

    char *newJson = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (newJson == NULL) {
        return;
    }

    FILE *file = fopen(deck_path, "w");
    if (file == NULL) {
        free(newJson);
        return;
    }
    else {
        fputs(newJson, file);
        fclose(file);
    }
    free(newJson);
}

void deleteDeckFile(const char *deck_path) {
    remove(deck_path);
}

