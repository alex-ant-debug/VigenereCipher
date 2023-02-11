/*
 * language.c
 *
 *  Created on: 8 Feb 2023
 *      Author: lemtugin
 */

#include "alphabet.h"


const alphabet English =
{
    .size = 26,
    .Big = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    .Small = "abcdefghijklmnopqrstuvwxyz",
    .frequentlyUsedLetters = "etaoinshrdlcumwfgypbvkxjqz",
    .dictionary = "dictionaryEnglish.txt"
};
