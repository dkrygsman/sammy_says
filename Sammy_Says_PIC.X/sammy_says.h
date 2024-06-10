/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef sammy_says_H    /* Guard against multiple inclusion */
#define sammy_says_H


/* ************************************************************************** */
/* ************************************************************************** */
#include "BOARD.h"
#include <xc.h>
#include <sys/attribs.h>                                                  
/* ************************************************************************** */
/* ************************************************************************** */


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
 
 
/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/ 



 /**
 * @function QEI_Init(void)
 * @param none
 * @brief  Enables the Change Notify peripheral and sets up the interrupt, anything
 *         else that needs to be done to initialize the module. 
 * @return SUCCESS or ERROR (as defined in BOARD.h)
*/
void sammy_says_init(void);

#endif	/* sammy_says.h */