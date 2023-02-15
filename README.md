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
This function loops through the "text" file line by line and passes those lines to the "textEncryption()" function.
```C++
void textEncryption(char *text, size_t sizeText, char *encryptedText, char *key)
```
