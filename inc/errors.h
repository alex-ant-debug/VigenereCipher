/*
 * errors.h
 *
 *  Created on: 11 Feb 2023
 *      Author: lemtugin
 */

#ifndef ERRORS_H_
#define ERRORS_H_


enum error
{
    err_OK   = 0,
    err_file = 1,
    err_mem  = 2,
};

typedef enum err_t
{
    ERR_OK                = 0,    /** Success - no error */
    ERR_OPEN_READ_FILE    = 1,    /** Error opening file for reading */
    ERR_OPEN_WRITE_FILE   = 2,    /** Error opening file for writing */
    ERR_WRITE_FILE        = 3,    /** File write error */
    ERR_MEMORY            = 4,    /** Memory allocation error */
	ERR_OPEN_DICTIONARY   = 5,    /** Dictionary opening error */
    ERR_NO_KEY_DICTIONARY = 6,    /** Error no key in dictionary */
	ERR_BLOCK_TREAD       = 7,    /** Failed to block the current thread */

}err_t;

#endif /* ERRORS_H_ */
