/*
 ============================================================================
 Name        : VigenereCipher.c
 Author      : Lemtugin
 Version     : v.0001
 Copyright   : Your copyright notice
 Description : Deciphering the text encrypted with the Vigenère cipher
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <stdbool.h>

#include "../inc/alphabet.h"
#include "../inc/errors.h"


typedef struct keyRating_t
{
    char key[15];
    unsigned int rating;
}keyRating_t;

// an array that will contain the number of letters in the text
static unsigned int frequentlyLetters[26];


static mtx_t mutex_read_file;
static mtx_t mutex_write_file;
static mtx_t mutex_array_access;


void error(int n)
{
    char *message[] = {"OK\n",
                       "Error occured while opening file\n",
                       "Can't allocate memory\n" };

    printf("%s\n", message[n]);
}


unsigned int findLargestLineInFile(FILE *file)
{
    unsigned int len = 0;
    unsigned int maxlen = 0;

    if(!file)
    {
        error(err_file);
        return 0;
    }

    while(1)
    {
        char symbol = 0;
        // On the first reading of the file, we find the display of the line
        fread(&symbol, 1, 1, file);
        if(symbol == '\r')
        {
            continue; // Ignore carriage return
        }

        int endOfFile = feof(file); //Is this the end of the file?

        if((symbol == '\n') || (endOfFile))
        {
            // The last line can also be the longest!
            if(len > maxlen)
            {
                maxlen = len;
            }
            len = 0;
        }
        else
        {
            len++;
        }

        if(endOfFile)
        {
            break;
        }
    }
    fseek(file, 0L, SEEK_SET);

    return (maxlen + 3);
}

char letterEncryption(char A, char B)
{
    unsigned int sumLetters;
    unsigned int firstLetterSmall = (unsigned int)English.Small[0];
    unsigned int firstLetterBig = (unsigned int)English.Big[0];

    unsigned int firstIndex = (unsigned int)A;
    unsigned int secondIndex = (unsigned int)B;


    if((firstIndex >= firstLetterSmall) && (firstIndex <= firstLetterSmall + English.size))
    {
        firstIndex -= firstLetterSmall;
    }

    if((firstIndex >= firstLetterBig) && (firstIndex <= firstLetterBig + English.size))
    {
        firstIndex -= firstLetterBig;
    }


    secondIndex -= firstLetterSmall;

    sumLetters = firstIndex + secondIndex;

    if(sumLetters >= English.size)
    {
        sumLetters -= English.size;
    }

    return English.Small[sumLetters];
}

char letterDecoding(char A, char B)
{
    int differenceLetters = 0;
    unsigned int firstLetterSmall = (unsigned int)English.Small[0];
    unsigned int firstLetterBig = (unsigned int)English.Big[0];

    unsigned int firstIndex = (unsigned int)A;
    unsigned int secondIndex = (unsigned int)B;

    if((firstIndex >= firstLetterSmall) && (firstIndex <= firstLetterSmall + English.size))
    {
        firstIndex -= firstLetterSmall;
    }

    if((firstIndex >= firstLetterBig) && (firstIndex <= firstLetterBig + English.size))
    {
        firstIndex -= firstLetterBig;
    }

    secondIndex -= firstLetterSmall;

    differenceLetters = firstIndex - secondIndex;

    if(differenceLetters < 0)
    {
        differenceLetters += English.size;
    }

    return English.Small[differenceLetters];
}

void textEncryption(char *text,
                    size_t sizeText,
                    char *encryptedText,
                    char *key)
{

    for(size_t index = 0, indexKey = 0; index < sizeText; index++)
    {
        unsigned int firstLetterSmall = (unsigned int)English.Small[0];
        unsigned int firstLetterBig = (unsigned int)English.Big[0];

        unsigned int letterNumber = (unsigned int)text[index];

        if(((letterNumber >= firstLetterSmall) && (letterNumber <= firstLetterSmall + English.size)) ||
           ((letterNumber >= firstLetterBig) && (letterNumber <= firstLetterBig + English.size)))
        {
            encryptedText[index] = letterEncryption(text[index], key[indexKey]);
            indexKey = (indexKey == 11)? 0: indexKey + 1;
        }
    }
}

void textTranscription(char *encryptedText,
                       size_t sizeText,
                       char *text,
                       char *key)
{
    for(size_t index = 0, indexKey = 0; index < sizeText; index++)
    {
        unsigned int firstLetterSmall = (unsigned int)English.Small[0];
        unsigned int firstLetterBig = (unsigned int)English.Big[0];

        unsigned int letterNumber = (unsigned int)encryptedText[index];

        if(((letterNumber >= firstLetterSmall) && (letterNumber <= firstLetterSmall + English.size)) ||
           ((letterNumber >= firstLetterBig) && (letterNumber <= firstLetterBig + English.size)))
        {
            text[index] = letterDecoding(encryptedText[index], key[indexKey]);
            indexKey = (indexKey == 11)? 0: indexKey + 1;
        }
        else
        {
            // if another character, then copy it
            text[index] = encryptedText[index];
        }
    }
}

err_t EncodingTextFromFileToFile(char *readFile,
                                char *fileNameEncrypted,
                                char *key)
{
    if(mtx_lock(&mutex_write_file) != 0)
    {
        return 0;
    }

    // Write to the file
    FILE *fileW = fopen(fileNameEncrypted, "w");
    if(!fileW)
    {
        error(err_file);
        mtx_unlock(&mutex_write_file);
        return ERR_OPEN_WRITE_FILE;
    }

    if(mtx_lock(&mutex_read_file) != 0)
    {
        return 0;
    }

    // Reading from the file
    FILE *fileR = fopen(readFile, "r");
    if(!fileR)
    {
        error(err_file);
        mtx_unlock(&mutex_read_file);
        return ERR_OPEN_READ_FILE;
    }

    unsigned int num = findLargestLineInFile(fileR);

    if(mtx_lock(&mutex_array_access) != 0)
    {
        return 0;
    }
    // We allocate memory for one line
    char *buffer = (char *)calloc((num + 3), sizeof(char));

    if (!buffer)
    {
        error(err_mem);
        fclose(fileR);
        fclose(fileW);
        mtx_unlock(&mutex_array_access);
        mtx_unlock(&mutex_read_file);
        mtx_unlock(&mutex_write_file);

        return ERR_MEMORY;
    }

    while(!feof(fileR))
    {
        if (fgets(buffer, num, fileR))
        {
            textEncryption(buffer, num, buffer, key);

            // Record the line in the file
            if(fputs(buffer, fileW) == EOF)
            {
                free(buffer);
                fclose(fileR);
                fclose(fileW);
                mtx_unlock(&mutex_array_access);
                mtx_unlock(&mutex_read_file);
                mtx_unlock(&mutex_write_file);

                return ERR_WRITE_FILE;
            }
        }
    }

    free(buffer);
    fclose(fileR);
    fclose(fileW);
    mtx_unlock(&mutex_array_access);
    mtx_unlock(&mutex_read_file);
    mtx_unlock(&mutex_write_file);

    return ERR_OK;
}

err_t DecodeTextFromFileToFile(char *fileNameEncrypted,
                              char *fileNameDeciphered,
                              char *key)
{
    if(mtx_lock(&mutex_write_file) != 0)
    {
        return 0;
    }

    // writing to file
    FILE *fileW = fopen(fileNameDeciphered, "w");
    if(!fileW)
    {
        error(err_file);
        mtx_unlock(&mutex_write_file);
        return ERR_OPEN_WRITE_FILE;
    }

    if(mtx_lock(&mutex_read_file) != 0)
    {
        return 0;
    }

    // reading from a file
    FILE *fileR = fopen(fileNameEncrypted, "r");
    if(!fileR)
    {
        error(err_file);
        mtx_unlock(&mutex_read_file);
        return ERR_OPEN_READ_FILE;
    }

    unsigned int num = findLargestLineInFile(fileR);

    if(mtx_lock(&mutex_array_access) != 0)
    {
        return 0;
    }

    // We allocate memory for one line
    char *bufferEncrypted = (char *)calloc((num + 3), sizeof(char));

    if (!bufferEncrypted)
    {
        error(err_mem);
        fclose(fileR);
        fclose(fileW);
        mtx_unlock(&mutex_array_access);
        mtx_unlock(&mutex_read_file);
        mtx_unlock(&mutex_write_file);

        return ERR_MEMORY;
    }

    while(!feof(fileR))
    {
        if (fgets(bufferEncrypted, num, fileR))
        {
            textTranscription(bufferEncrypted, num, bufferEncrypted, key);

            // write a string
            if(fputs(bufferEncrypted, fileW) == EOF)
            {
                free(bufferEncrypted);
                fclose(fileR);
                fclose(fileW);
                mtx_unlock(&mutex_array_access);
                mtx_unlock(&mutex_read_file);
                mtx_unlock(&mutex_write_file);

                return ERR_WRITE_FILE;
            }
        }
    }

    free(bufferEncrypted);
    fclose(fileR);
    fclose(fileW);
    mtx_unlock(&mutex_array_access);
    mtx_unlock(&mutex_read_file);
    mtx_unlock(&mutex_write_file);

    return ERR_OK;
}

void countingLettersString(char *stringToCount,
                           unsigned int *frequently,
                           unsigned int start,
                           unsigned int step)
{
    unsigned int sizeString = strlen(stringToCount);

    for(size_t j = start; j < sizeString; j += step)
    {
        for(size_t i = 0; i < English.size; i++)
        {
            if(English.Small[i] == stringToCount[j])
            {
                frequently[i] += 1;
                break;
            }
        }
    }
}

unsigned int findingKeySize(float *hitIndex)
{
    unsigned int numberLettersInText = 0;

    for(unsigned int i = 0; i < English.size; i++)
    {
        //Calculation of the number of letters in the text
        numberLettersInText += frequentlyLetters[i];
    }

    unsigned int maxIndex = 0;
    float max = 0;
    float matchIndice = 0;

    for(unsigned int i = 0; i < English.size; i++)
    {
        float temp = frequentlyLetters[i];
        hitIndex[i] = temp*((temp - 1)/(numberLettersInText*(numberLettersInText - 1)));
        printf("hitIndex[%d] = %f\n", i, hitIndex[i]);
        matchIndice += hitIndex[i];

        // key size lookup
        if((max < hitIndex[i]) && (i > 3) && (i < 21))
        {
            max = hitIndex[i];
            maxIndex = i;
        }
    }
    printf("matchIndice = %f\n", matchIndice);

    return maxIndex;
}

void frequencyAnalysis(FILE *readFile,
                       unsigned int *frequentlyLetters,
                       unsigned int size)
{

    unsigned int num = findLargestLineInFile(readFile);

    // We allocate memory for one line
    char *buffer = (char *)calloc((num + 3), sizeof(char));

    if (!buffer)
    {
        error(err_mem);
    }

    while(!feof(readFile))
    {
        if (fgets(buffer, num, readFile))
        {
            countingLettersString(buffer, frequentlyLetters, 0, 1);
        }
    }

    fseek(readFile, 0L, SEEK_SET);

    float hitIndex[English.size];
    unsigned int sizeKey = findingKeySize(hitIndex) + 1;

    printf("sizeKey = %d\n", sizeKey);

    static unsigned int frequently[26] = {0};

    while(!feof(readFile))
    {
        if (fgets(buffer, num, readFile))
        {
            countingLettersString(buffer, frequently, sizeKey - 12, 12);
        }
    }

    for(unsigned int i = 0; i < English.size; i++)
    {
        printf("frequently[%d] = %d\n", i, frequently[i]);
    }

    free(buffer);

}

void sortLetterFrequency(char *sortAlphabet,
                         unsigned int *frequentlyLetters,
                         unsigned int size)
{
    unsigned int max = 0;
    unsigned int maxindex = 0;

    for(size_t i = 0; i < size; i++)
    {
        for(size_t j = 0; j < size; j++)
        {
            if(max < frequentlyLetters[j])
            {
                max = frequentlyLetters[j];
                maxindex = j;
            }
        }

        sortAlphabet[i] = English.Small[maxindex];
        frequentlyLetters[maxindex] = 0;
        max = 0;
        maxindex = 0;
    }

    for(unsigned int i = 0; i < size; i++)
    {
        printf("sortAlphabet[%d] = %c\n", i, sortAlphabet[i]);
    }
}

unsigned int countExistingWords(FILE *dictionary,
                                unsigned int maxSizeWord,
                                char *checkedFile)
{
    unsigned int numberDetectedWords = 0;

    // open the file to be checked
    FILE *fileRead = fopen(checkedFile, "r");
    if(!fileRead)
    {
        error(err_file);
        return ERR_OPEN_READ_FILE;
    }

    unsigned int num = findLargestLineInFile(fileRead);

    // We allocate memory for one line
    char *encryptedString = (char *)calloc((num + 3), sizeof(char));

    if (!encryptedString)
    {
        error(err_mem);
        fclose(fileRead);
        return ERR_MEMORY;
    }

    // move the dictionary pointer to the beginning of the dictionary
    fseek(dictionary, 0L, SEEK_SET);

    while(!feof(fileRead))
    {
        if (fgets(encryptedString, num, fileRead))
        {
            unsigned int lengthWord = findLargestLineInFile(dictionary);
            char *wordFromDictionary = (char*)calloc(lengthWord, sizeof(char*));

            // read file being checked
            while(!feof(dictionary))
            {
                if (fgets(wordFromDictionary, lengthWord, dictionary))
                {
                    char *copyEncryptedString = encryptedString;
                    unsigned int sizeWord = strlen(wordFromDictionary);
                    char copyWordFromDictionary[sizeWord];

                    strcpy(copyWordFromDictionary, wordFromDictionary);
                    copyWordFromDictionary[sizeWord-1] = 0;

                    while(copyEncryptedString)
                    {
                        copyEncryptedString = strstr(copyEncryptedString, copyWordFromDictionary);

                        if(copyEncryptedString)
                        {
                            numberDetectedWords++;
                            copyEncryptedString +=1;
                        }
                    }
                }
            }
            fseek(dictionary, 0L, SEEK_SET);
            free(wordFromDictionary);
        }
    }
    fclose(fileRead);
    free(encryptedString);

    return numberDetectedWords;
}

err_t enumerationKeys(char *readFile, char *fileWrite, unsigned int sizeKey)
{
    unsigned int maxNumberKeys = 10;
    unsigned int numKey = 0;
    static unsigned int currentPosition = 0;

    keyRating_t *selectedKey = (keyRating_t *)calloc(maxNumberKeys, sizeof(keyRating_t));

    // reading a dictionary
    FILE *dictionary = fopen(English.dictionary, "r");
    if(!dictionary)
    {
        error(err_file);
        printf("Failed to open dictionary\n");
        return ERR_OPEN_DICTIONARY;
    }
    unsigned int num = findLargestLineInFile(dictionary);

    char *keyFromDictionary = (char *)calloc(num, sizeof(char *));

    if (!keyFromDictionary)
    {
        error(err_mem);
        fclose(dictionary);
        free(selectedKey);
        return ERR_MEMORY;
    }

    while(!feof(dictionary))
    {
        if (fgets(keyFromDictionary, num, dictionary))
        {
            size_t sizeWord = strlen(keyFromDictionary);
            // save the current position in the dictionary
            currentPosition +=  sizeWord;

            if((sizeKey + 1) == sizeWord)
            {
                // select the key
                DecodeTextFromFileToFile(readFile, fileWrite, keyFromDictionary);
                // count the number of words that matched the dictionary and write it to selectedKey
                selectedKey[numKey].rating = countExistingWords(dictionary, num, fileWrite);
                strcpy(selectedKey[numKey].key, keyFromDictionary);
                numKey++;
                //return a dictionary pointer to the previous value to use the new key
                fseek(dictionary, (currentPosition + 1) - sizeWord, SEEK_SET);
            }
        }
    }

    unsigned int maxRating = 0;
    unsigned int indexMaxRating = 0;

    for(unsigned int i = 0; i < maxNumberKeys; i++)
    {
        if(selectedKey[i].rating)
        {
            printf("key = %srating = %d\n", selectedKey[i].key, selectedKey[i].rating);
        }

        if(maxRating < selectedKey[i].rating)
        {
            maxRating = selectedKey[i].rating;
            indexMaxRating = i;
        }
    }
    fclose(dictionary);
    free(keyFromDictionary);


    if(selectedKey[0].rating)
    {
        printf("key = %s\n", selectedKey[indexMaxRating].key);
        free(selectedKey);
        return ERR_OK;
    }
    else
    {
        printf("The key could not be found from the dictionary\n");
        free(selectedKey);
        return ERR_NO_KEY_DICTIONARY;
    }
}


void crackCipher(char *readFile, char *fileNameDeciphered)
{
    err_t err;

    // reading from a file
    FILE *fpEncrypted = fopen(readFile, "r");

    if(!fpEncrypted)
    {
        error(err_file);
        return;
    }

    unsigned int num = findLargestLineInFile(fpEncrypted);

    // We allocate memory for one line
    char *buffer = (char *)calloc((num + 3), sizeof(char));

    if (!buffer)
    {
        error(err_mem);
    }

    while(!feof(fpEncrypted))
    {
        if (fgets(buffer, num, fpEncrypted))
        {
            countingLettersString(buffer, frequentlyLetters, 0, 1);
        }
    }

    fseek(fpEncrypted, 0L, SEEK_SET);


    float hitIndex[English.size];
    unsigned int sizeKey = findingKeySize(hitIndex) + 1;

    printf("sizeKey = %d\n", sizeKey);

    fclose(fpEncrypted);

    err = enumerationKeys(readFile, fileNameDeciphered, 12);

    if(err != ERR_OK)
    {
        //frequencyAnalysis(fpEncrypted, frequentlyLetters, English.size);
    }
    else
    {
        //DecodeTextFromFileToFile(readFile, fileNameDeciphered, );
    }

}



int main(int argc, char *argv[]) {

    unsigned int i;
    err_t err;

    char *menu[] = {"encrypt",
                    "decipher",
                    "hack"};

    if(argc < 3)
    {
        printf("You must enter the command with parameters:\n");
        printf("  <command> <read file> <write file> <key>\n");
        printf("Or, to crack an encrypted file, type:\n");
        printf("  <command> <read file> <write file>\n");
        printf("where:\n");
        printf("  <command>    -  action selection command, can be:\n");
        printf("                  encrypt, decipher, hack\n");
        printf("  <read file>  -  file for reading data\n");
        printf("  <write file> -  file for writing decrypted or ciphertext\n");
        printf("  <key>        -  fkeyword to encrypt or decrypt text\n");

        return 1;
    }

    for(i = 0; i < argc; i++)
    {
        if(strcmp(argv[1], menu[i]) == 0)
        {
            break;
        }
    }

    switch(i)
    {
        case 0:{

            err = EncodingTextFromFileToFile(argv[2], argv[3], argv[4]);

            if(err != ERR_OK)
            {
                printf("Encryption error %d\n", err);
            }
            else
            {
                printf("Text encryption was successful\n");
            }
            break;}

        case 1:{

            err = DecodeTextFromFileToFile(argv[2], argv[3], argv[4]);

            if(err != ERR_OK)
            {
                printf("Text decryption error %d\n", err);
            }
            else
            {
                printf("Text decryption was successful\n");
            }
            break;}

        case 2:{

            crackCipher(argv[2], argv[3]);

            break;}

        default: break;
    }


    return EXIT_SUCCESS;
}
