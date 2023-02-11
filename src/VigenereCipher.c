/*
 ============================================================================
 Name        : VigenereCipher.c
 Author      : Lemtugin
 Version     : v.0001
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alphabet.h>

// an array that will contain the number of letters in the text
static unsigned int frequentlyLetters[26];


enum err_t
{
	err_OK = 0,
	err_file = 1,
	err_mem = 2,
};

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
    char c;

    if (!file)
    {
        error(err_file);
    }

    while (1)
    {
        // On the first reading of the file, we find the display of the line
        fread(&c, 1, 1, file);
        if (c == '\r')
        {
            continue; // Ignore carriage return
        }

        int e = feof(file); //Is this the end of the file?

        if (c == '\n' || e)
        {
            // The last line can also be the longest!
            if (len > maxlen)
            {
                maxlen = len;
            }
            len = 0;
        }
        else
        {
            len++;
        }

        if(e)
        {
            break;
        }
    }

    printf("Max. length of string = %d\n", maxlen);

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

void EncodingTextFromFileToFile(char *readFile,
                                char *fileNameEncrypted,
                                char *key)
{
    // Write to the file
    FILE *fileW = fopen(fileNameEncrypted, "w");
    if(!fileW)
    {
    	error(err_file);
        return;
    }

    // Reading from the file
    FILE *fileR = fopen(readFile, "r");
    if(!fileR)
    {
    	error(err_file);
        return;
    }

    unsigned int num = findLargestLineInFile(fileR);

	// We allocate memory for one line
	char *buffer = (char *)calloc((num + 3), sizeof(char));

	if (!buffer)
	{
		error(err_mem);
	    fclose(fileR);
	    fclose(fileW);
		return;
	}

    while(!feof(fileR))
    {
        if (fgets(buffer, num, fileR))
        {
            textEncryption(buffer, num, buffer, key);

            // Record the line in the file
            fputs(buffer, fileW);
        }
    }
	free(buffer);

    fclose(fileR);
    fclose(fileW);
}

void DecodeTextFromFileToFile(char *fileNameEncrypted,
                              char *fileNameDeciphered,
                              char *key)
{
        // writing to file
        FILE *fileW = fopen(fileNameDeciphered, "w");
        if(!fileW)
        {
        	error(err_file);
            return;
        }

        // reading from a file
        FILE *fileR = fopen(fileNameEncrypted, "r");
        if(!fileR)
        {
        	error(err_file);
            return;
        }

        unsigned int num = findLargestLineInFile(fileR);

    	// We allocate memory for one line
    	char *bufferEncrypted = (char *)calloc((num + 3), sizeof(char));

    	if (!bufferEncrypted)
    	{
    		error(err_mem);
            fclose(fileR);
            fclose(fileW);
    		return;
    	}

        while(!feof(fileR))
        {
            if (fgets(bufferEncrypted, num, fileR))
            {
                textTranscription(bufferEncrypted, num, bufferEncrypted, key);

                // write a string
                fputs(bufferEncrypted, fileW);
            }
        }
    	free(bufferEncrypted);

        fclose(fileR);
        fclose(fileW);
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

unsigned int hitIndexCalculation()
{
    float hitIndex[English.size];
    unsigned int numberLettersInText = 0;

    for(unsigned int i = 0; i < English.size; i++)
    {
        //Calculation of the number of letters in the text
        numberLettersInText += frequentlyLetters[i];
    }
    printf("numberLettersInText = %d\n", numberLettersInText);

    unsigned int maxIndex = 5;
    float max = 0;

    for(unsigned int i = 0; i < English.size; i++)
    {
        float temp = frequentlyLetters[i];
        //hitIndex[i] = (temp*100)/numberLettersInText;
        hitIndex[i] = temp*((temp - 1)/(numberLettersInText*(numberLettersInText - 1)));
        printf("hitIndex[%d] = %f\n", i, hitIndex[i]);

        if((max < hitIndex[i]) && (i > 3) && (i < 21))
        {
            max = hitIndex[i];
            maxIndex = i;
        }
    }

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


    unsigned int sizeKey = hitIndexCalculation() + 1;

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

    unsigned int maxIndex = 0;
    float max = 0;
    float hitIndex[English.size];
    float coincidenceIndex = 0;

    for(unsigned int i = 0; i < English.size; i++)
    {
        float temp = frequently[i];
        //hitIndex[i] = (temp*100)/numberLettersInText;
        hitIndex[i] = temp*((temp - 1)/(941*(941 - 1)));
        printf("hitIndex[%d] = %f\n", i, hitIndex[i]);

        coincidenceIndex += hitIndex[i];

        if(max < hitIndex[i])
        {
            max = hitIndex[i];
            maxIndex = i;
        }
    }

    printf("maxIndex = %d\n", maxIndex);
    printf("Coincidence Index = %f\n", coincidenceIndex);

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




int main(int argc, char *argv[]) {

    char key[12] = "distribution";

//-----------------------We encrypt the text---------------------------

    // Reading file
    char * fileNameData = "data.txt";
    // File for recording
    char * fileNameEncrypted = "EncryptedText.txt";

    EncodingTextFromFileToFile(fileNameData,
                               fileNameEncrypted,
                               key);


//-----------------------We decrypt on the key-----------------------

    // File for recording
    char *fileNameDeciphered = "DecipheredText.txt";

    DecodeTextFromFileToFile(fileNameEncrypted,
                             fileNameDeciphered,
                             key);


    return EXIT_SUCCESS;
}
