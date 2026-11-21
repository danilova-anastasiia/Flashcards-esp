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

    cJSON *root = loadJson(deck_path);
    if (root == NULL) {
        return 0;
    }

    cJSON *arrayCards = cJSON_GetObjectItemCaseSensitive(root, "cards");
    int cardCount = 0;
    if (cJSON_IsArray(arrayCards)) {
        cardCount = cJSON_GetArraySize(arrayCards);
    }
    cJSON_Delete(root);

    return cardCount;

}

int createDeck(const char *deck_path, const char *deck_title) {
    
    FILE *ifExistsCheck = fopen(deck_path, "rb");
    if (ifExistsCheck != NULL) {
        fclose(ifExistsCheck);
        return 0;
    }
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return 0;
    }
    cJSON_AddStringToObject(root, "title", deck_title);
    cJSON_AddArrayToObject(root, "cards");

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

int deleteDeckFile(const char *deck_path) {
    if (deck_path == NULL) {
        return 0;
    }
    return remove(deck_path) == 0;
}


void deckClose(void) {

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

int deckSize(void) {
    return deck_count;
}

void deckShuffle(void) {

    int count = deckSize();
    if (order == NULL || count < 2) return;

    for (int i = count - 1; i > 0; i--) {
        int randomIndex = rand() % (i+1);
        if (randomIndex != i) {
            int temp = *(order + i);
            *(order + i) = *(order + randomIndex);
            *(order + randomIndex) = temp;
        }
    }

}

const char* deckTerm(int pos) {

    if (deck_root == NULL || deck_cards == NULL || pos < 0 || pos >= deck_count) {
        return "";
    }

    cJSON *cardIndex = cJSON_GetArrayItem(deck_cards, order[pos]);
    cJSON *term = cJSON_GetObjectItemCaseSensitive(cardIndex, "term");

    if (!cJSON_IsString(term)) return "";
    return term -> valuestring;

}

const char* deckDefinition(int pos) {

    if (deck_root == NULL || deck_cards == NULL || pos < 0 || pos >= deck_count) {
        return "";
    }

    cJSON *cardIndex = cJSON_GetArrayItem(deck_cards, order[pos]);
    cJSON *definition = cJSON_GetObjectItemCaseSensitive(cardIndex, "definition");

    if (!cJSON_IsString(definition)) return "";
    return definition -> valuestring;

}

int addCard(const char* deck_path, const char *term, const char *definition) {
    
    cJSON *root = loadJson(deck_path);
    if (root == NULL) {
        return 0;
    }

    cJSON *arrayCards = cJSON_GetObjectItemCaseSensitive(root, "cards");
    if(!cJSON_IsArray(arrayCards)) {
        cJSON_Delete(root);
        return 0;
    }
    int size = cJSON_GetArraySize(arrayCards);

    cJSON *newCard = cJSON_CreateObject();
    cJSON_AddStringToObject(newCard, "term", term);
    cJSON_AddStringToObject(newCard, "definition", definition);

    if (cJSON_AddItemToArray(arrayCards, newCard) == 0) {
        cJSON_Delete(newCard);
        cJSON_Delete(root);
        return 0;
    }
        
    char *jsonString = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(jsonString == NULL) {
        return 0;
    }
    
    FILE *file = fopen(deck_path, "wb");
    if (file == NULL) {
        free(jsonString);
        return 0;
    }

    fputs(jsonString, file);
    fclose(file);
    free(jsonString);

    return 1;

}

int deckList(const char* dir_path, char deckName[][67], int max) {

    if (dir_path == NULL || deckName == NULL || max <= 0) return 0;

    DIR *directory = opendir(dir_path);
    if (directory == NULL) return 0;

    int countDir = 0; 
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL && countDir < max) {
        size_t nameLength = strlen(entry -> d_name);
        if (nameLength > 5 && strcmp(entry -> d_name + nameLength - 5, ".json") == 0) {
            snprintf(deckName[countDir], 67, "%s", entry -> d_name);
            countDir++;
        }
    }

    closedir(directory);
    return countDir;

}
