#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>
#include <ctype.h>
#include "fileops.tscott5.h"

#define FILENAME "words.dat"
#define LETTER_TO_SEARCH 'n'

int main() {
  
    // Opens/creates file for r/w
    FILE *fp;
    char filename[32] = FILENAME;
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
    countWords(fp, LETTER_TO_SEARCH, &count);
    printf("Number of words with letter '%c': %d\n\n", LETTER_TO_SEARCH, count);

    /*
     * getWords() not 100% working 
     */ 

    char **words;
    words = getWords(fp, LETTER_TO_SEARCH);
    printf("Word From Array: %s\n", *words);
    for (int i = 0; words[i] != NULL; i++) {
        // printf("Word From Array: %s\n", **words[i]);
        printf("Word From Array: %s\n", *words);
    }

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
    
    long seekPos = 8 * (letter - 'a');

    // Make sure it's acutually a letter
    if (checkWord(&letter) != 0) {
        printf("Invalid Word\n");
        return 1;
    }

    // Start at first word
    fseek(fp, seekPos, SEEK_SET);  
    long longNum;
    fread(&longNum, sizeof(long), 1, fp);

    if (longNum == 0) {
        printf("No Words Starting With: %c\n", letter);
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
        }
    }
    return 0;
}

char **getWords(FILE *fp, char letter) {

    char **rtnVal, *word;
    long pos = 8*(letter - 'a');
    int wordLen, count, i;
    countWords(fp, letter, &count);
    Record record;
    i = 0;

    printf("Get Word Letter: %c\n", letter);

    rtnVal = (char**) malloc((count+1) * sizeof(char *));
    if (count == 0) {
        rtnVal[0] = NULL;
        return rtnVal;
    }
    
    // Start at first word
    fseek(fp, pos, SEEK_SET);  
    long longNum;
    fread(&longNum, sizeof(long), 1, fp);

    fseek(fp, longNum, SEEK_SET);
    fread(&record, sizeof(record), 1, fp);

    int k = 0;
    for (k = 0; record.word[k] != '\0'; ++k);
    wordLen = k;
    word = (char *) malloc((wordLen+1) * sizeof(char));
    
    //strcpy(word, record.word);
    strncpy(word, record.word, wordLen);
    word[wordLen] = '\0';

    rtnVal[i] = word;
    i += 1;
    pos = ftell(fp);
    printf("Current Pos In Get Word: %ld\n", pos);
    printf("First Word: %s | %ld\n", record.word, record.nextPos);

    while (record.nextPos != 0) {
        fseek(fp, record.nextPos, SEEK_SET);
        fread(&record, sizeof(Record), 1, fp); 
       
        pos = ftell(fp);
        printf("Current Pos In Get Word Loop: %ld\n", pos);
       
        int j = 0;
        for (j = 0; record.word[j] != '\0'; ++j);       
        wordLen = j;
        word = (char *) malloc((wordLen+1) * sizeof(char));

        //strcpy(word, record.word);
        strncpy(word, record.word, wordLen);
        word[wordLen] = '\0';

        rtnVal[i] = word;
        i += 1;
        long longPos = record.nextPos;
        printf("Next Word: %s | %ld\n", word, longPos);   
    }
    // Make last value in array NULL
    rtnVal[i] = NULL;
    return rtnVal;
}

int insertWord(FILE *fp, char *word) {
    
    char convertedWord[MAXWORDLEN+1]; 
    convertToLower(word, convertedWord); 
    long pos, longNum, filesize, rc;
    pos = 8*(convertedWord[0] - 'a'); // Starts at indexed value for letter
    Record record;
    
    // Make sure it's acutually a word
    if (checkWord(convertedWord) != 0) {
        printf("Invalid Word\n");
        return 1;
    }

    // Get filesize
    rc = fseek(fp, 0, SEEK_END);
    if (rc != 0) {
        printf("fseek() failed\n");
        return rc;
    }
    filesize = ftell(fp);
    printf("File size: %d\n", filesize);

    // Find position in file 
    fseek(fp, pos, SEEK_SET); // @Param: filepath ptr, # of bytes to offset, where to start pointer  
    printf("Letter Index Place - 1: %ld\n", (convertedWord[0]-'a'));

    // Read the value held by the long (Position of first word with letter)
    fread(&longNum, sizeof(long), 1, fp);
    printf("First Word Pos: %ld\n", longNum);

    // If no words with letter
    if (longNum == 0) {

        // Write the long into location of letter value
        printf("No word that starts with '%c'\n", convertedWord[0]);
        fseek(fp, pos, SEEK_SET);
        fwrite(&filesize, sizeof(long), 1, fp);

        // Write first word of letter to file
        fseek(fp, 0, SEEK_END);
        strcpy(record.word, convertedWord);
        record.nextPos = 0; 
        fwrite(&record, sizeof(Record), 1, fp);
        printf("Writing First Word With '%c' at Position %d: %s | Next Pos: %d\n\n", convertedWord[0], filesize, record.word, record.nextPos);
        
    }

    else {

        // Go to value stored in long (The first word with letter)
        fseek(fp, longNum, SEEK_SET); 
        fread(&record, sizeof(Record), 1, fp);
        printf("\nFirst word with letter '%c': %s | %ld\n", record.word[0], record.word, record.nextPos);

        // Check every next position until one is 0
        while (record.nextPos != 0) {
            fseek(fp, record.nextPos, SEEK_SET);
            fread(&record, sizeof(Record), 1, fp); 
            printf("Word Found Starting With: '%c': %s | %d\n", record.word[0], record.word, record.nextPos); 
        }
        
        // Go back to the beginning of the currently read Record
        pos = ftell(fp) - sizeof(Record);
        if (record.nextPos == 0) {

            // Overwrite current word and nextPos
            printf("Word At Current Pos: %s | %ld\n", record.word, record.nextPos);
            fseek(fp, pos, SEEK_SET);
            strcpy(record.word, record.word); 
            record.nextPos = filesize;
            fwrite(&record, sizeof(Record), 1, fp);
            printf("Word Being Overwritten and New Position: %s | %d\n", record.word, record.nextPos);

            // Create new record at end of file
            fseek(fp, 0, SEEK_END);
            strcpy(record.word, convertedWord);
            record.nextPos = 0;
            fwrite(&record, sizeof(Record), 1, fp);
            printf("New Word Being Added and New Position: %s | %d\n\n", record.word, record.nextPos);

        }
    }
    return 0;
}