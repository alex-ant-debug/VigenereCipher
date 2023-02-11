/*
 * alphabet.h
 *
 *  Created on: 8 Feb 2023
 *      Author: lemtugin
 */

#ifndef ALPHABET_H_
#define ALPHABET_H_

typedef struct alphabet
{
    unsigned int size;
    char *Big;
    char *Small;
    char *frequentlyUsedLetters;
    const char *dictionary;
}alphabet;

extern const alphabet English;


#endif /* ALPHABET_H_ */
