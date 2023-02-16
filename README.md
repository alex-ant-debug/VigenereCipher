# Console program for decrypting text encrypted with Vigenere cipher

This is a command line application that decrypts text encrypted with the Vigenère cipher. The program is built using Mikefile. The program executes three 
commands: encrypting text with a key, decrypting text with a key, and decrypting text without a key. 

## Project build

To build the program, go to the folder with the program, open a terminal and enter the command "**cmake --build.**". To clean up a project, enter the 
"**make clean**" command.

## Application Launch Commands

After building the project and compiling, you can encrypt the file with text. To do this, enter the command: "**./VigenereCipher encrypt data.txt EncryptedText.txt distribution**"
where: 
- **./VigenereCipher** - application launch, 
- **encrypt**  - is a command that tells the application that the file needs to be encrypted, 
- **data.txt** - is the file to encrypt, 
- **EncryptedText.txt** - is the file where the encrypted text will be written,
- **distribution** - word key for encryption.

To decrypt text, you can use the "decipher" command, for example: **"./VigenereCipher decipher EncryptedText.txt DecipheredText.txt distribution"**
where 
- **./VigenereCipher** - application launch,
- **decipher** - a command that tells the application that the text needs to be decrypted,
- **EncryptedText.txt**  - file for decryption,
- **DecipheredText.txt** -  distribution - the file where the decrypted text will be written,
- **distribution** - word key for decryption.

To decrypt the ciphertext without a key, use the "a" command, for example: **"./VigenereCipher hack EncryptedText.txt DecipheredText.txt"** 
where:
- **./VigenereCipher** - application launch,
- **hack** - is a command telling the application to decrypt the text without a key,
- **EncryptedText.txt**  - file for decryption,
- **DecipheredText.txt** -  distribution - the file where the decrypted text will be written.

After entering the "hack" command, if the text is successfully decrypted, the approximate key size will be displayed in the terminal. Next, the key itself 
and the decrypted text. If the application could not decrypt the text, it will display a message about it. For example, if the key is not in the dictionary,
then the inscription will be displayed: "The key could not be found from the dictionary".

When you run the program with an insufficient number of arguments or with an excess of them, information about the commands and their parameters is 
displayed. For example:
```
You must enter the command with parameters:
  <command> <read file> <write file> <key>
Or, to crack an encrypted file, type:
  <command> <read file> <write file>
where:
  <command>    -  action selection command, can be:
                  encrypt, decipher, hack
  <read file>  -  file for reading data
  <write file> -  file for writing decrypted or ciphertext
  <key>        -  fkeyword to encrypt or decrypt text
```

## Description of the algorithm of the program and functions

In the main "main" function, a small menu is implemented, with the help of which the action on files is selected. 
One of the main functions is:
```C++
err_t EncodingTextFromFileToFile(char *readFile,
                                 char *fileNameEncrypted,
                                 char *key)
```
This function encrypts the text from the given "readFile" file into the "fileNameEncrypted" file. "key" is the key to encrypt. This function opens the 
file "readFile" for reading and the file "fileNameEncrypted" for writing. Writes line by line to a temporary buffer a line from file "readFile", encrypts 
it and writes it to file "fileNameEncrypted" for writing. If the operation is successful, it returns ERR_OK, if one of the resources is blocked by another 
thread, it returns ERR_BLOCK_TREAD. If the file cannot be opened for reading it returns ERR_OPEN_READ_FILE, if the file cannot be opened for writing it 
returns ERR_OPEN_WRITE_FILE. It also returns ERR_MEMORY on unsuccessful memory allocation.

The function that decrypts the text with the key:
```C++
err_t DecodeTextFromFileToFile(char *fileNameEncrypted, char *fileNameDeciphered, char *key)
```
This function, like the previous one, opens two files, one for reading "fileNameEncrypted" and the other for writing "fileNameDeciphered". If the operation 
is successful, it returns ERR_OK, if one of the resources is blocked by another thread, it returns ERR_BLOCK_TREAD. If the file cannot be opened for 
reading it returns ERR_OPEN_READ_FILE, if the file cannot be opened for writing it returns ERR_OPEN_WRITE_FILE. It also returns ERR_MEMORY on unsuccessful 
memory allocation.
Text decoding is performed by the function:
```C++
void textTranscription(char *encryptedText, size_t sizeText, char *text, char *key)
```

This function iterates over strings, letter by letter. By passing letters to the function "letterDecoding()". In which the letters are shifted relative to 
the letters of the key. This function also searches for the key for decryption. The decoding of letters performs the function:
```C++
char letterDecoding(char A, char B)
```
The "letterDecoding" function performs an alphabetic shift of the letter of the decrypted text relative to the letters of the key. Returns the shifted 
letter. If a letter is outside the alphabet then the shift is applied to the opposite side of the alphabet.
One of the main functions is the encryption function:
```C++
err_t EncodingTextFromFileToFile(char *readFile,
                                char *fileNameEncrypted,
                                char *key)
```
This function opens two files, one for reading "readFile" and the other for writing ciphertext "fileNameEncrypted". If the operation is successful, it 
returns ERR_OK, if one of the resources is blocked by another thread, it returns ERR_BLOCK_TREAD. If the file cannot be opened for reading it returns 
ERR_OPEN_READ_FILE, if the file cannot be opened for writing it returns ERR_OPEN_WRITE_FILE. It also returns ERR_MEMORY on unsuccessful memory allocation.
String encryption is performed using the "textEncryption()" function.
```C++
void textEncryption(char *text,
                    size_t sizeText,
                    char *encryptedText,
                    char *key)
```
This function loops through the "text" file line by line and passes those lines to the "textEncryption()" function. If the character is not a letter then it is skipped. At the same time, the key is moved.
```C++
void textEncryption(char *text, size_t sizeText, char *encryptedText, char *key)
```
The textEncryption function shifts the alphabet of the letter "A" to the right by the value of the index of the letter "B" in the alphabet. If the letter index goes beyond the alphabet index, then the shift continues, but with the index of the beginning of the alphabet. This function returns the letter with the offset.
  This project has additional files errors.h, alphabet.h and language.c. The errors.h file contains the user-defined data type err_t, which lists the errors that may occur during program operation. There is also an "error" enum which is used to display errors.
  
  The file "alphabet.h" contains a user-defined data type, the "alphabet" structure, whose fields store the language property. The language contains strings listing the alphabet itself in small and large cases. The language also has a language size field, a string listing commonly used characters, and a string with the name of the dictionary file. This data type is made for convenience, for example, if you need to decrypt texts in several languages, then the properties of each language can be described in the "language.c" file. So far there is only one language, it is English.
  
Consider the "crackCipher()" function, it is called in the main function. It is intended for deciphering text without a key word. It opens two files, one for reading the ciphertext and one for writing attempts at the decrypted text. After that, the file requiring decryption is read line by line in the loop. Each line is written to the "buffer" variable, after which the "countingLettersString()" function counts how many times each letter of the alphabet was used in the text. This information is written to the "frequentlyLetters" variable.
```C++
  err_t crackCipher(char *readFile, char *fileNameDeciphered)
```
This function also finds the word length of the key. This is done using the "findingKeySize()" function. After the approximate length of the key has been found, the function "enumerationKeys()" is called, it selects the key word from the dictionary. If the word key is present in the dictionary it will return the message "ERR_OK", if not it will return an error message.If an error message is returned, the "hackingWithMutualIndex()" function will run, which looks up the word key using the index of matches. On success it will return "ERR_OK", on failure it will return an error code. All error codes in the "crackCipher()" function are written to the "err" variable. If at the end of the function its value is "ERR_OK", then the "DecodeTextFromFileToFile()" function will be called, which will decrypt the file using the received key. And the "printFile()" function will also be called, which will print the contents of the decrypted file to the terminal window.

```C++
unsigned int findingKeySize(float *hitIndex)
```
The "findingKeySize()" function searches for an approximate key size. An array with a length equal to the length of the alphabet is written to the argument of this function. This function calculates the match index for each letter of the alphabet in the decrypted text. Then it finds the letter with the highest match index value. The index of a given letter in the alphabet indicates the word length of the key. There are restrictions in this function, the key cannot be less than 3 letters or more than 21 letters.The function returns the approximate key size.

The function "enumerationKeys()" selects a key from the available words in the dictionary.The arguments to this function are also two files, the encrypted file and the file that will be written to during decryption. Also, the estimated word length of the key and the string in which the found key will be written are passed to the arguments.

```C++
err_t enumerationKeys(char *readFile,
                      char *fileWrite,
                      unsigned int sizeKey,
                      char *key)
```
If the operation is successful, it returns ERR_OK, if one of the resources is blocked by another thread, it returns ERR_BLOCK_TREAD. If the file cannot be opened for reading it returns ERR_OPEN_READ_FILE, if the file cannot be opened for writing it returns ERR_OPEN_WRITE_FILE. It also returns ERR_MEMORY on unsuccessful memory allocation. In this function, after opening the files, a cycle of line-by-line reading of the dictionary file is started. The string data is stored in the "keyFromDictionary" buffer, after which a string with a size equal to the size of the key word is searched for. When words of this length are found, the "DecodeTextFromFileToFile()" function is launched, which decrypts the encrypted file using the given key. After decryption, the "countExistingWords()" function is launched, which counts the words in the decrypted text that match the words in the dictionary. This value is written to the array of variables "selectedKey[numKey].rating" and the word key itself is also written to it. At the end of reading the dictionary, using this array, you can determine the key word by the maximum number of "existing" words found. If the key is found, then the key itself is printed into the terminal window and it is also written to the "key" variable.
```C++
err_t hackingWithMutualIndex(char *readFile,
                             char *fileWrite,
                             unsigned int sizeKey)
```
