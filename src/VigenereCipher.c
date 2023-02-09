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


void error(int n)
{
	char *msg[] = { "OK",
					"Can't open file",
					"Can't allocate memory" };
	printf("%s\n", msg[n]);
}


unsigned int findingLengthLineInFile(FILE *file)
{
	if (!file)
	{
		error(1);
	}

	int len = 0, maxlen = 0;
	char c;

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

	// We allocate memory for one line and stand at the beginning of the file:
//	char *buf = (char *)malloc((maxlen + 3)*sizeof(char));
//
//	if (!buf)
//	{
//		error(2);
//	}

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

	//printf("A = %c; B = %c;", A, B);

	if((firstIndex >= firstLetterSmall) && (firstIndex <= firstLetterSmall + English.size))
	{
		firstIndex -= firstLetterSmall;
	}

	if((firstIndex >= firstLetterBig) && (firstIndex <= firstLetterBig + English.size))
	{
		firstIndex -= firstLetterBig;
	}


	if(((firstIndex >= firstLetterSmall) && (firstIndex <= firstLetterSmall + English.size))	||
	   ((secondIndex >= firstLetterSmall) && (secondIndex <= firstLetterSmall + English.size))	||
	   ((firstIndex >= firstLetterBig) && (firstIndex <= firstLetterBig + English.size))		||
	   ((secondIndex >= firstLetterBig) && (secondIndex <= firstLetterBig + English.size)))
	{
		secondIndex -= firstLetterSmall;

		sumLetters = firstIndex + secondIndex;

		if(sumLetters >= English.size)
		{
			sumLetters -= English.size;
		}

		//printf(" English.Small[differenceLetters] = %c\n", English.Small[sumLetters]);

		return English.Small[sumLetters];
	}

	return '^';
}

char letterDecoding(char A, char B)
{
	int differenceLetters = 0;
	unsigned int firstLetterSmall = (unsigned int)English.Small[0];
	unsigned int firstLetterBig = (unsigned int)English.Big[0];

	unsigned int firstIndex = (unsigned int)A;
	unsigned int secondIndex = (unsigned int)B;

	//printf("A = %c; B = %c;", A, B);

	if((firstIndex >= firstLetterSmall) && (firstIndex <= firstLetterSmall + English.size))
	{
		firstIndex -= firstLetterSmall;
	}

	if((firstIndex >= firstLetterBig) && (firstIndex <= firstLetterBig + English.size))
	{
		firstIndex -= firstLetterBig;
	}


	if(((firstIndex >= firstLetterSmall) && (firstIndex <= firstLetterSmall + English.size))	||
	   ((secondIndex >= firstLetterSmall) && (secondIndex <= firstLetterSmall + English.size))	||
	   ((firstIndex >= firstLetterBig) && (firstIndex <= firstLetterBig + English.size))		||
	   ((secondIndex >= firstLetterBig) && (secondIndex <= firstLetterBig + English.size)))
	{

		secondIndex -= firstLetterSmall;

		differenceLetters = firstIndex - secondIndex;

		if(differenceLetters < 0)
		{
			differenceLetters += English.size;
		}
		//printf(" English.Small[differenceLetters] = %c\n", English.Small[differenceLetters]);

		return English.Small[differenceLetters];
	}

	return '*';
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
			encryptedText[index] = letterDecoding(text[index], key[indexKey]);
			//printf("%c", encryptedText[index]);
		}

		indexKey = (indexKey == 11)? 0: indexKey + 1;
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

		unsigned int letterNumber = (unsigned int)encryptedText[index];
		//printf("index = %d\tindexKey = %d\tencryptedText[index] = %c\t", index, indexKey, symbol);

		if((letterNumber >= firstLetterSmall) && (letterNumber <= firstLetterSmall + English.size))
		{
			text[index] = letterEncryption(encryptedText[index], key[indexKey]);
			//printf("text[index] = %c\n", text[index]);
		}
		else
		{
			// if another character, then copy it
			text[index] = encryptedText[index];
			//printf("\n");
		}

		indexKey = (indexKey == 11)? 0: indexKey + 1;
	}
}

void EncodingTextFromFileToFile(char *readFile,
	 	  	  	  	  	  	    char *fileNameEncrypted,
								char *key)
{
	// запись в файл
	FILE *fileW = fopen(fileNameEncrypted, "w");
	if(!fileW)
	{
		printf("Error occured while opening file\n");
		return;
	}

	// буфер для считавания данных из файла
	char buffer[256];
	// чтение из файла
	FILE *fp = fopen(readFile, "r");
	if(!fp)
	{
		printf("Error occured while opening file\n");
		return;
	}
	// пока не дойдем до конца, считываем по 256 байт

	while(!feof(fp))
	{
		if (fgets(buffer, 256, fp))
		{
			//printf("%s", buffer);
			textEncryption(buffer, 256, buffer, key);

		    // записываем строку
		    fputs(buffer, fileW);
		}
	}

	fclose(fp);
	fclose(fileW);
}

void DecodeTextFromFileToFile(char *fileNameEncrypted,
						 	  char *fileNameDeciphered,
							  char *key)
{
		// writing to file
		FILE *fileWriteD = fopen(fileNameDeciphered, "w");
		if(!fileWriteD)
		{
			printf("Error occured while opening file\n");
			return;
		}

		// buffer for reading data from a file
		char bufferEncrypted[256];
		// reading from a file
		FILE *fpEncrypted = fopen(fileNameEncrypted, "r");
		if(!fpEncrypted)
		{
			printf("Error occured while opening file\n");
			return;
		}

		//until we reach the end, we read 256 bytes

		while(!feof(fpEncrypted))
		{
			if (fgets(bufferEncrypted, 256, fpEncrypted))
			{
				//printf("%s", buffer);
				textTranscription(bufferEncrypted, 256, bufferEncrypted, key);

				// write a string
				fputs(bufferEncrypted, fileWriteD);
			}
		}

		fclose(fpEncrypted);
		fclose(fileWriteD);
}

void countingLettersString(char *stringToCount, unsigned int *frequently)
{
	size_t sizeString = strlen(stringToCount);

	printf("sizeString = %i\n", sizeString);
	for(size_t j = 0; j < English.size; j++)
	{
		for(size_t i = 0; i < sizeString; i++)
		{
			if(English.Small[j] == stringToCount[i])
			{
				frequently[j] += 1;
			}
		}
	}
}

void frequencyAnalysis(char *readFile)
{
	FILE *fp = fopen(readFile, "r");
	if(!fp)
	{
		printf("Error occured while opening file\n");
		return;
	}

	unsigned int num = findingLengthLineInFile(fp);
	// buffer for reading data from a file
	char buffer[num];

	while(!feof(fp))
	{
		if (fgets(buffer, num, fp))
		{

			countingLettersString(buffer, frequentlyLetters);
		}
	}

	fclose(fp);

	for(size_t i = 0; i < English.size; i++)
	{
		printf("%c = %d\n", English.Small[i], frequentlyLetters[i]);
	}
}


int main(int argc, char *argv[]) {

//	for(int i = 0; i < argc; i++)
//	{
//		printf("%s\n", argv[i]);
//	}

//	char key[12] = "amphitheater";
//
//-----------------------шифруем текст---------------------------
//
//	// файл чтения
//	char * fileNameData = "data.txt";
//	//файл записи
	char * fileNameEncrypted = "EncryptedText.txt";
//
//
//	EncodingTextFromFileToFile(fileNameData,
//							   fileNameEncrypted,
//							   key);


	//-----------------------расшифровваем по ключу-----------------------

	// файл чтения
	//char *fileNameEncrypted = "EncryptedText.txt";
	//файл записи
//	char *fileNameDeciphered = "DecipheredText.txt";
//
//	DecodeTextFromFileToFile(fileNameEncrypted,
//							 fileNameDeciphered,
//							 key);
	frequencyAnalysis(fileNameEncrypted);


	return EXIT_SUCCESS;
}
