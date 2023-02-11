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
    ERR_OK              = 0,    /**< Success - no error */
    ERR_OPEN_READ_FILE  = 1,
	ERR_OPEN_WRITE_FILE = 2,
	ERR_WRITE_FILE      = 3,
    ERR_MEMORY          = 4,
    ERR_WOULDBLOCK      = 5,

}err_t;

#endif /* ERRORS_H_ */
