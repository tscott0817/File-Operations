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
    int fileExists, rc;
    fileExists = 0;
    fp = (FILE *) fopen(filename, "r+");
    if (fp != NULL) {
        printf("File Foud: %s\n", FILENAME);
        fileExists = 1;
    }

    // Checks if something happend in file search/creation
    if (!fileExists) {
        fp = (FILE *) fopen(filename, "w+");
        if (fp == NULL) {
            printf("Cannot open file '%s'\n", filename);
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

    // Get Words
    char **words;
    words = getWords(fp, LETTER_TO_SEARCH);
    int i = 0;
    while (words[i] != NULL) {
        printf("Word [%d] With Letter '%c': %s\n", i, LETTER_TO_SEARCH, words[i]);
        //printf("word[%d] is |%s|\n", i, words[i]);
        i = i + 1;
    }
    printf("\n");

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
    int rc;

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
    int wordLen, count, i, index, start;
    countWords(fp, letter, &count);
    Record record;

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

    // Get length of word and allocate space
    int k = 0;
    for (k = 0; record.word[k] != '\0'; ++k);
    wordLen = k;
    word = (char *) malloc((wordLen+1) * sizeof(char));
    
    // Copy word and set end of string char
    strncpy(word, record.word, wordLen);
    word[wordLen] = '\0';

    // Put word in array
    rtnVal[i] = word;
    i += 1;

    while (record.nextPos != 0) {

        fseek(fp, record.nextPos, SEEK_SET);
        fread(&record, sizeof(Record), 1, fp); 
       
        // Get length of word and allocate space
        int j = 0;
        for (j = 0; record.word[j] != '\0'; ++j);       
        wordLen = j;
        word = (char *) malloc((wordLen+1) * sizeof(char));

        strncpy(word, record.word, wordLen);
        word[wordLen] = '\0';

        rtnVal[i] = word;
        i += 1;
        long longPos = record.nextPos; // No idea why, but removing this causes error?
    }
    rtnVal[i] = NULL;
    return rtnVal;
}

int insertWord(FILE *fp, char *word) {
    
    char convertedWord[MAXWORDLEN+1]; 
    convertToLower(word, convertedWord); 
    long pos, longNum, filesize;
    int rc, rn;
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

    // Find position in file 
    rc = fseek(fp, pos, SEEK_SET); // @Param: filepath ptr, # of bytes to offset, where to start pointer  
    if (rc != 0) {
      printf("fseek() failed\n");
      fclose(fp);
      return rc;
    }
    // Read the value held by the long (Position of first word with letter)
    rn = fread(&longNum, sizeof(long), 1, fp);
    if (rn != 1) {
        printf("ERROR: fread() failed to read a value\n");
        fclose(fp);
        return 9;
    }

    // If no words with letter
    if (longNum == 0) {

        // Write the long into location of letter value
        rc = fseek(fp, pos, SEEK_SET);
        if (rc != 0) {
            printf("fseek() failed\n");
            fclose(fp);
        return rc;
        }
        rn = fwrite(&filesize, sizeof(long), 1, fp);
        if (rn != 1) {
            printf("ERROR: fwrite() failed to write a value\n");
            fclose(fp);
            return 8;
        }

        // Write first word of letter to file
        rc = fseek(fp, 0, SEEK_END);
        if (rc != 0) {
            printf("fseek() failed\n");
            fclose(fp);
            return rc;
        }
        strcpy(record.word, convertedWord);
        record.nextPos = 0; 
        rn = fwrite(&record, sizeof(Record), 1, fp);
        if (rn != 1) {
            printf("ERROR: fwrite() failed to write a value\n");
            fclose(fp);
            return 8;
        }
     
    }

    else {

        // Go to value stored in long (The first word with letter)
        rc = fseek(fp, longNum, SEEK_SET); 
        if (rc != 0) {
            printf("fseek() failed\n");
            fclose(fp);
            return rc;
        }
        rn = fread(&record, sizeof(Record), 1, fp);
        if (rn != 1) {
            printf("ERROR: fread() failed to read a value\n");
            fclose(fp);
            return 8;
        }

        // Check every next position until one is 0
        while (record.nextPos != 0) {
            rc = fseek(fp, record.nextPos, SEEK_SET);
            if (rc != 0) {
                printf("fseek() failed\n");
                fclose(fp);
                return rc;
            }
            rn = fread(&record, sizeof(Record), 1, fp); 
            if (rn != 1) {
                printf("ERROR: fread() failed to read a value\n");
                fclose(fp);
                return 9;
            }
        }
        
        // Go back to the beginning of the currently read Record
        pos = ftell(fp) - sizeof(Record);
        if (record.nextPos == 0) {

            // Overwrite current word and nextPos
            rc = fseek(fp, pos, SEEK_SET);
            if (rc != 0) {
                printf("fseek() failed\n");
                fclose(fp);
                return rc;
            }
            strcpy(record.word, record.word); 
            record.nextPos = filesize;
            rn = fwrite(&record, sizeof(Record), 1, fp);
            if (rn != 1) {
                printf("ERROR: fwrite() failed to write a value\n");
                fclose(fp);
                return 8;
            }

            // Create new record at end of file
            rc = fseek(fp, 0, SEEK_END);
            if (rc != 0) {
                printf("fseek() failed\n");
                fclose(fp);
                return rc;
            }
            strcpy(record.word, convertedWord);
            record.nextPos = 0;
            rn = fwrite(&record, sizeof(Record), 1, fp);
            if (rn != 1) {
                printf("ERROR: fwrite() failed to write a value\n");
                fclose(fp);
                return 8;
            }

        }
    }
    return 0;
}