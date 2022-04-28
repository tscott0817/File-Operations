#define MAXWORDLEN 31

typedef struct RecordStruct {
    char word[MAXWORDLEN + 1];
    long nextPos;
} Record;

int checkWord(char *word);
int convertToLower(char *word, char *convertedWord);
int countWords(FILE *fp, char letter, int *count);
char **getWords(FILE *fp, char letter);
int insertWord(FILE *fp, char *word);