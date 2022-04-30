#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>
#include <ctype.h>
#include "fileops.tscott5.h"

int main() {
  
    // Opens/creates file for r/w
    FILE *fp;
    char filename[32] = "words.dat";
    int fileExists = 0;
    fp = (FILE *) fopen(filename, "r+");
    if (fp != NULL) {
        fileExists = 1;
    }


    // Checks if something happend in file search/creation
    if (!fileExists) {
        fp = (FILE *) fopen(filename, "w+");
        if (fp == NULL) {
            printf("cannot open file '%s'\n", filename);
            return 8;
        }

        // Fill first 208 bytes with longs
        long longSize = 0;
        for (int i = 0; i < 201; i++) {
            fseek(fp, i, SEEK_SET);
            fwrite(&longSize, sizeof(long), 1, fp);
        }
    }

    char userInput[MAXWORDLEN+1];
    printf("\nEnter a word to save to the file: ");
    scanf("%s", userInput);
    printf("\n");
        
    // Insertion
    Record record;
    strcpy(record.word, userInput);
    insertWord(fp, record.word);

    // Count Words
    int count;
    char letter = 'n';
    countWords(fp, letter, &count);
    printf("Number of words with letter '%c': %d\n", letter, count);

    fclose(fp);
    return 0;
}

int checkWord(char *word) {
  int i, len;

  len = strlen(word);
  for (i=0; i<len; ++i) {
    if ( ! isalpha(word[i]) )
      return 1;
  }

  return 0;
} 

int convertToLower(char *word, char *convertedWord) {
  int i, len;

  strcpy(convertedWord, word);

  len = strlen(word);
  for (i=0; i<len; ++i)
    convertedWord[i] = tolower(word[i]);

  return 0;
}

int countWords(FILE *fp, char letter, int *count) {
    
    // Make sure it's acutually a letter
    if (checkWord(&letter) != 0) {
        printf("Invalid Word");
        return 1;
    }

    // Start at first word
    fseek(fp, 8*(letter - 'a'), SEEK_SET);  
    long longNum;
    fread(&longNum, sizeof(long), 1, fp);

    if (longNum == 0) {
        printf("No Words Starting With: %c", letter);
    }

    else { 

        fseek(fp, longNum, SEEK_SET); // Go to value stored in long
        Record record;
        fread(&record, sizeof(Record), 1, fp);

        while (record.nextPos != 0) {
            fseek(fp, record.nextPos, SEEK_SET);
            fread(&record, sizeof(Record), 1, fp); 
            *count += 1;
        }

        // If at at end of file
        if (record.nextPos == 0) {
            *count += 1;
            printf("No More Words Starting With: %c\n", letter);
        }
    }
    return 0;
}

char **getWords(FILE *fp, char letter) {
    char **rtnVal;
    rtnVal = (char**) malloc(sizeof(char *));
    rtnVal[0] = NULL;
}

int insertWord(FILE *fp, char *word) {
    
    char convertedWord[MAXWORDLEN+1]; 
    convertToLower(word, convertedWord); 
    long seekPos = 8 * (convertedWord[0] - 'a');
    printf("Seek Pos: %ld\n", seekPos);
    
    // Make sure it's acutually a word
    if (checkWord(convertedWord) != 0) {
        printf("Invalid Word");
        return 1;
    }

    // Get filesize
    char rc;
    long filesize;
    rc = fseek(fp, 0, SEEK_END);
    if (rc != 0) {
        printf("fseek() failed\n");
        return rc;
    }
    filesize = ftell(fp);
    printf("File size: %d\n", filesize);

    // Find position in file 
    fseek(fp, seekPos, SEEK_SET); // @Param: filepath ptr, # of bytes to offset, where to start pointer  
    printf("Letter Index Place - 1: %ld\n", (convertedWord[0]-'a'));
    
    long longNum;
    fread(&longNum, sizeof(long), 1, fp);
    printf("Long Pos: %ld\n", longNum);

    int posOne;
    posOne = ftell(fp) - sizeof(long);
    printf("Current Position: %ld\n", posOne);
    if (longNum == 0) {

        printf("No word that starts with '%c'\n", convertedWord[0]);
        fseek(fp, seekPos, SEEK_SET);
        fwrite(&filesize, sizeof(long), 1, fp);

        fseek(fp, 0, SEEK_END);
        Record record;
        strcpy(record.word, convertedWord);
        record.nextPos = 0; 
        fwrite(&record, sizeof(Record), 1, fp);
        printf("Writing First Word With '%c' at Position %d: %s | Next Pos: %d\n\n", convertedWord[0], filesize, record.word, record.nextPos);
        
    }

    // If there is already a word with that letter
    else {

        printf("At least one word starting with: %c\n", convertedWord[0]);
        int pos;
        pos = ftell(fp) - sizeof(long);
        // long curPos;
        // fread(&curPos, sizeof(long), 1, fp);
        printf("Current Position: %ld\n", pos);
        Record record;
        fseek(fp, longNum, SEEK_SET); // Go to value stored in long
        fread(&record, sizeof(Record), 1, fp);
        printf("Value Stored In Long Now: %ld\n", longNum);

        printf("First word with letter '%c': %s | %ld\n", record.word[0], record.word, record.nextPos);

        // This only gets last word since only the first word has a nextPos value
        while (record.nextPos != 0) {
            fseek(fp, record.nextPos, SEEK_SET);
            fread(&record, sizeof(Record), 1, fp); 
            printf("Last Word Found Starting With: '%c': %s | Next Pos: %d\n", record.word[0], record.word, record.nextPos); // Only gets last word
        }
        
        int curPos;
        curPos = ftell(fp);
        // long curPos;
        // fread(&curPos, sizeof(long), 1, fp);
        printf("Current Position: %ld\n", curPos);
        if (record.nextPos == 0) {
            printf("Word At Current Pos: %s | %ld\n", record.word, record.nextPos);
            fseek(fp, curPos, SEEK_SET);
            //fread(&record, sizeof(Record), 1, fp);
            Record recordNew;
            strcpy(recordNew.word, record.word); 
            recordNew.nextPos = filesize;
            fwrite(&recordNew, sizeof(Record), 1, fp);
            printf("Word Being Overwritten and New Position: %s | %d\n", recordNew.word, recordNew.nextPos);

            // New Record
            Record newRecord;
            fseek(fp, 0, SEEK_END);
            strcpy(newRecord.word, convertedWord);
            newRecord.nextPos = 0;
            printf("New Word Being Added and New Position: %s | %d\n\n", newRecord.word, newRecord.nextPos);
            fwrite(&newRecord, sizeof(Record), 1, fp);
            fread(&newRecord, sizeof(Record), 1, fp); 
            printf("After Writing Word: %s | %d\n\n", newRecord.word, newRecord.nextPos);

        }

    }
    return 0;
}