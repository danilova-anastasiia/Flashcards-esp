#include <stdio.h>
#include "cJSON.h"
#include <stdlib.h>

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

