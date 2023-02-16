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

#define MAX_SIZE_KEY    12

// key rating, to select the most appropriate key
typedef struct keyRating_t
{
    char key[15];
    unsigned int rating;
}keyRating_t;

// structure for calculating the match index for different offsets
typedef struct matchIndexShift_t
{
	char shift;
    unsigned int rating[26];
}matchIndexShift_t;

// an array that will contain the number of letters in the text
static unsigned int frequentlyLetters[26];

mtx_t mutex_crack_cipher;
mtx_t mutex_enumeration;
mtx_t mutex_decode;
mtx_t mutex_encoding;
mtx_t mutex_hackMI;
mtx_t mutex_existW;
mtx_t mutex_printFile;


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
    if(mtx_lock(&mutex_encoding) != 0)
    {
        return ERR_BLOCK_TREAD;
    }

    // Write to the file
    FILE *fileW = fopen(fileNameEncrypted, "w");
    if(!fileW)
    {
        error(err_file);
        mtx_unlock(&mutex_encoding);
        return ERR_OPEN_WRITE_FILE;
    }

    // Reading from the file
    FILE *fileR = fopen(readFile, "r");
    if(!fileR)
    {
        error(err_file);
        mtx_unlock(&mutex_encoding);
        fclose(fileW);
        return ERR_OPEN_READ_FILE;
    }

    unsigned int num = findLargestLineInFile(fileR);

    // We allocate memory for one line
    char *buffer = (char *)calloc((num + 3), sizeof(char));

    if (!buffer)
    {
        error(err_mem);
        fclose(fileR);
        fclose(fileW);
        mtx_unlock(&mutex_encoding);

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
                mtx_unlock(&mutex_encoding);

                return ERR_WRITE_FILE;
            }
        }
    }

    free(buffer);
    fclose(fileR);
    fclose(fileW);
    mtx_unlock(&mutex_encoding);

    return ERR_OK;
}

err_t DecodeTextFromFileToFile(char *fileNameEncrypted,
                               char *fileNameDeciphered,
                               char *key)
{
    if(mtx_lock(&mutex_decode) != 0)
    {
        return ERR_BLOCK_TREAD;
    }

    // writing to file
    FILE *fileW = fopen(fileNameDeciphered, "w");
    if(!fileW)
    {
        error(err_file);
        mtx_unlock(&mutex_decode);
        return ERR_OPEN_WRITE_FILE;
    }

    // reading from a file
    FILE *fileR = fopen(fileNameEncrypted, "r");
    if(!fileR)
    {
        error(err_file);
        fclose(fileW);
        mtx_unlock(&mutex_decode);
        return ERR_OPEN_READ_FILE;
    }

    unsigned int num = findLargestLineInFile(fileR);

    // We allocate memory for one line
    char *bufferEncrypted = (char *)calloc((num + 3), sizeof(char));

    if (!bufferEncrypted)
    {
        error(err_mem);
        fclose(fileR);
        fclose(fileW);
        mtx_unlock(&mutex_decode);

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
                mtx_unlock(&mutex_decode);

                return ERR_WRITE_FILE;
            }
        }
    }

    free(bufferEncrypted);
    fclose(fileR);
    fclose(fileW);
    mtx_unlock(&mutex_decode);

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

unsigned int calculationNumberLettersText(unsigned int *freqLetters)
{
	unsigned int numberLettersInText = 0;

	for(unsigned int i = 0; i < English.size; i++)
	{
		numberLettersInText += freqLetters[i];
	}

	return numberLettersInText;
}


unsigned int findingKeySize(float *hitIndex)
{
    unsigned int numberLettersInText = calculationNumberLettersText(frequentlyLetters);

    unsigned int maxIndex = 0;
    float max = 0;
    float matchIndice = 0;

    for(unsigned int i = 0; i < English.size; i++)
    {
        float temp = frequentlyLetters[i];
        hitIndex[i] = temp*((temp - 1)/(numberLettersInText*(numberLettersInText - 1)));
        matchIndice += hitIndex[i];

        // key size search, key not less than 3 and not more than 21
        if((max < hitIndex[i]) && (i > 3) && (i < 21))
        {
            max = hitIndex[i];
            maxIndex = i;
        }
    }

    return maxIndex;
}

void keySizedSubstrings(char *bufferEncrypted,
		                unsigned int sizeSrting,
						char **arrayString,
						unsigned int sizeKey)
{
	unsigned int lineCounter = 0;

	for(unsigned int i = 0, j = 0; i < sizeSrting; i++)
	{
		if(bufferEncrypted[i] == '\0')
		{
			break;
		}

		if((bufferEncrypted[i] >= 'a') && (bufferEncrypted[i] <= 'z'))
		{
			arrayString[lineCounter][j] = bufferEncrypted[i];

			if(j == 11)
			{
				arrayString[lineCounter][sizeKey] = '\0';
				lineCounter++;
				j = 0;
			}
			else
			{
				j++;
			}
		}
	}
}

void stringAlphabetShift(char *array,
		                 unsigned int sizeKey,
						 unsigned int shiftKey)
{
	for(unsigned int i = 0; i < sizeKey; i++)
	{
		if((array[i] == '\0'))
		{
			break;
		}

		char shift = array[i] - shiftKey;

		array[i] = ((shift) < 'a')? 'z' - ('a' - shift): shift;
	}
}

void countingFreqLettersInPairLines(char *firstLine,
		                            char *secondLine,
									unsigned int *rating,
									unsigned int sizeKey)
{
	for(unsigned int i = 0; i < English.size; i++)
	{
		for(unsigned int j = 0; j < sizeKey + 1; j++)
		{
			if(firstLine[j] == English.Small[i])
			{
				rating[i] += 1;
			}

			if(secondLine[j] == English.Small[i])
			{
				rating[i] += 1;
			}
		}
	}
}

void countMutualIndexRows(matchIndexShift_t *offsetEachLetter,
		                  float *mutualMatchIndex)
{
	for(unsigned int shift = 0; shift < English.size; shift++)
	{
		unsigned int numberLetters = calculationNumberLettersText(offsetEachLetter[shift].rating);

		numberLetters *= (numberLetters - 1);
		// index calculation at one shift
		for(unsigned int i = 0; i < English.size; i++)
		{
			float temp = offsetEachLetter[shift].rating[i];

			if(temp)
			{
				temp *= (temp - 1);
				mutualMatchIndex[shift] += temp/numberLetters;
			}
		}
	}
}

err_t hackingWithMutualIndex(char *readFile,
                             char *fileWrite,
                             unsigned int sizeKey)
{
    if(mtx_lock(&mutex_hackMI) != 0)
    {
        return ERR_BLOCK_TREAD;
    }

	matchIndexShift_t *offsetEachLetter = (matchIndexShift_t*)calloc(English.size, sizeof(matchIndexShift_t));
	if(!offsetEachLetter)
	{
		error(err_file);
		mtx_unlock(&mutex_hackMI);
		return ERR_OPEN_READ_FILE;
	}

    FILE *fileR = fopen(readFile, "r");
    if(!fileR)
    {
        error(err_file);
        free(offsetEachLetter);
        mtx_unlock(&mutex_hackMI);
        return ERR_OPEN_READ_FILE;
    }

    unsigned int num = findLargestLineInFile(fileR);

    // We allocate memory for one line
    char *bufferEncrypted = (char *)calloc((num + 3), sizeof(char));
    if (!bufferEncrypted)
    {
        error(err_mem);
        fclose(fileR);
        free(offsetEachLetter);
        mtx_unlock(&mutex_hackMI);
        return ERR_MEMORY;
    }

    while(!feof(fileR))
    {
        if (fgets(bufferEncrypted, num, fileR))
        {
        	size_t sizeSrting = strlen(bufferEncrypted);
        	unsigned int numberSubstrings = sizeSrting/sizeKey;
        	// allocated memory for an array of strings, each string is the size of a key
        	char **arrayString = (char**)calloc(numberSubstrings, sizeof(char*));

        	for(unsigned int i = 0; i < numberSubstrings; i++)
        	{
        		arrayString[i]=(char *)calloc(sizeKey + 1, sizeof(char));
        	}
        	// splitting a string into substrings the size of a key
        	keySizedSubstrings(bufferEncrypted, sizeSrting, arrayString, sizeKey);

        	for(unsigned int shiftKey = 1; shiftKey < English.size; shiftKey++)
        	{
        		offsetEachLetter[shiftKey-1].shift = shiftKey;
        		// loop through the arrayString array line by line, take two lines and
        		// on the second line we shift the alphabet relative to the alphabet of the previous line
				for(unsigned int i = 0; i < (numberSubstrings) - 1; i++)
				{
					if(arrayString[i + 1][0] == '\0')
					{
						break;
					}
					char *array = (char*)calloc(sizeKey+1, sizeof(char*));
					strcpy(array, arrayString[i + 1]);

					// string alphabet shift
					stringAlphabetShift(array, sizeKey, shiftKey);
					// counting the frequency of letters in a pair of lines
					countingFreqLettersInPairLines(arrayString[i],
							                       array,
												   offsetEachLetter[shiftKey-1].rating,
												   sizeKey);
					free(array);
				}
        	}
        	// memory free
        	for(unsigned int i = 0; i < numberSubstrings; i++)
        	{
        		free(arrayString[i]);
        	}
        	free(arrayString);
        }
    }
    free(bufferEncrypted);
    fclose(fileR);

	//count the mutual index of rows
    float *mutualMatchIndex = (float*)calloc(English.size, sizeof(float));
	countMutualIndexRows(offsetEachLetter, mutualMatchIndex);

	for(unsigned int shift = 0; shift < English.size; shift++)
	{
		printf("mutualMatchIndex[%u] = %e\n", shift, mutualMatchIndex[shift]);
	}

	//Here we must determine the maximums by mutualMatchIndex and select the desired key

	free(mutualMatchIndex);
    free(offsetEachLetter);
    mtx_unlock(&mutex_hackMI);

    return ERR_OK;
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
}

unsigned int countExistingWords(FILE *dictionary,
                                unsigned int maxSizeWord,
                                char *checkedFile)
{
    unsigned int numberDetectedWords = 0;

    if(mtx_lock(&mutex_existW) != 0)
    {
        return ERR_BLOCK_TREAD;
    }

    // open the file to be checked
    FILE *fileRead = fopen(checkedFile, "r");
    if(!fileRead)
    {
        error(err_file);
        mtx_unlock(&mutex_existW);
        return ERR_OPEN_READ_FILE;
    }
    unsigned int num = findLargestLineInFile(fileRead);

    // We allocate memory for one line
    char *encryptedString = (char *)calloc((num + 3), sizeof(char*));
    if (!encryptedString)
    {
        error(err_mem);
        fclose(fileRead);
        mtx_unlock(&mutex_existW);
        return ERR_MEMORY;
    }
    // move the dictionary pointer to the beginning of the dictionary
    fseek(dictionary, 0L, SEEK_SET);

    // read file being checked
    while(!feof(fileRead))
    {
        if (fgets(encryptedString, num, fileRead))
        {
            unsigned int lengthWord = findLargestLineInFile(dictionary);
            char *wordFromDictionary = (char*)calloc(lengthWord, sizeof(char*));

            // dictionary reading
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
    mtx_unlock(&mutex_existW);

    return numberDetectedWords;
}

err_t enumerationKeys(char *readFile,
                      char *fileWrite,
                      unsigned int sizeKey,
                      char *key)
{
    unsigned int maxNumberKeys = 10; // maximum number of matched keys
    unsigned int numKey = 0;
    static unsigned int currentPosition = 0;

    if(mtx_lock(&mutex_enumeration) != 0)
    {
        return ERR_BLOCK_TREAD;
    }

    keyRating_t *selectedKey = (keyRating_t *)calloc(maxNumberKeys, sizeof(keyRating_t));
    if (!selectedKey)
    {
        error(err_mem);
        mtx_unlock(&mutex_enumeration);
        return ERR_MEMORY;
    }

    // reading a dictionary
    FILE *dictionary = fopen(English.dictionary, "r");
    if(!dictionary)
    {
        error(err_file);
        free(selectedKey);
        mtx_unlock(&mutex_enumeration);
        return ERR_OPEN_DICTIONARY;
    }
    unsigned int num = findLargestLineInFile(dictionary);

    char *keyFromDictionary = (char *)calloc(num, sizeof(char *));
    if (!keyFromDictionary)
    {
        error(err_mem);
        fclose(dictionary);
        free(selectedKey);
        mtx_unlock(&mutex_enumeration);
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
    fclose(dictionary);
    free(keyFromDictionary);

    unsigned int maxRating = 0;
    unsigned int indexMaxRating = 0;

    //find the key by the maximum rating
    for(unsigned int i = 0; i < maxNumberKeys; i++)
    {
        if(maxRating < selectedKey[i].rating)
        {
            maxRating = selectedKey[i].rating;
            indexMaxRating = i;
        }
    }

    if(selectedKey[0].rating)
    {
        printf("key = %s\n", selectedKey[indexMaxRating].key);
        strcpy(key, selectedKey[indexMaxRating].key);
        free(selectedKey);
        mtx_unlock(&mutex_enumeration);
        return ERR_OK;
    }
    else
    {
        printf("The key could not be found from the dictionary\n");
        free(selectedKey);
        mtx_unlock(&mutex_enumeration);
        return ERR_NO_KEY_DICTIONARY;
    }
}

err_t printFile(char *fileRead)
{
    if(mtx_lock(&mutex_printFile) != 0)
    {
        return ERR_BLOCK_TREAD;
    }

    // Reading from the file
    FILE *fileR = fopen(fileRead, "r");
    if(!fileR)
    {
        error(err_file);
        mtx_unlock(&mutex_printFile);
        return ERR_OPEN_READ_FILE;
    }

    unsigned int num = findLargestLineInFile(fileR);

    // We allocate memory for one line
    char *buffer = (char *)calloc((num + 3), sizeof(char));
    if (!buffer)
    {
        error(err_mem);
        fclose(fileR);
        mtx_unlock(&mutex_printFile);

        return ERR_MEMORY;
    }

    while(!feof(fileR))
    {
        if (fgets(buffer, num, fileR))
        {
        	printf("%s\n", buffer);
        }
    }

    free(buffer);
    fclose(fileR);
    mtx_unlock(&mutex_printFile);

    return ERR_OK;
}

err_t crackCipher(char *readFile, char *fileNameDeciphered)
{
    err_t err;

    if(mtx_lock(&mutex_crack_cipher) != 0)
    {
        return ERR_BLOCK_TREAD;
    }

    // reading from a file
    FILE *fpEncrypted = fopen(readFile, "r");
    if(!fpEncrypted)
    {
        error(err_file);
        mtx_unlock(&mutex_crack_cipher);
        return ERR_OPEN_READ_FILE;
    }

    unsigned int num = findLargestLineInFile(fpEncrypted);

    // We allocate memory for one line
    char *buffer = (char *)calloc((num + 3), sizeof(char));
    if (!buffer)
    {
        error(err_mem);
        fclose(fpEncrypted);
        mtx_unlock(&mutex_crack_cipher);
        return ERR_MEMORY;
    }

    while(!feof(fpEncrypted))
    {
        if (fgets(buffer, num, fpEncrypted))
        {
            countingLettersString(buffer, frequentlyLetters, 0, 1);
        }
    }

    fclose(fpEncrypted);
    free(buffer);

    float *hitIndex = (float*)calloc(English.size, sizeof(float));
    if (!hitIndex)
    {
        error(err_mem);
        return ERR_MEMORY;
    }

    unsigned int sizeKey = findingKeySize(hitIndex) + 1;
    free(hitIndex);
    mtx_unlock(&mutex_crack_cipher);

    printf("approximate key size = %d\n", sizeKey);

    if(sizeKey > MAX_SIZE_KEY)
    {
    	sizeKey = MAX_SIZE_KEY;
    }

    char *key = (char*)calloc(sizeKey, sizeof(char));

    err = enumerationKeys(readFile, fileNameDeciphered, sizeKey, key);

    if(err != ERR_OK)
    {
    	err = hackingWithMutualIndex(readFile, fileNameDeciphered, sizeKey);
    }

    if(err == ERR_OK)
    {
    	err = DecodeTextFromFileToFile(readFile, fileNameDeciphered, key);
    	printFile(fileNameDeciphered);
    }
    free(key);

    return err;
}


int main(int argc, char *argv[]) {

    unsigned int menuNumber;
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

    for(menuNumber = 0; menuNumber < argc; menuNumber++)
    {
        if(strcmp(argv[1], menu[menuNumber]) == 0)
        {
            break;
        }
    }

    switch(menuNumber)
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
