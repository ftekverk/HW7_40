/**************************************************************
 *
 *                     operations.h
 *
 *      Assignment: comp40 Assignment 6 : um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Interface of the um. Implementation handles running
 *      commands.
 *
**************************************************************/

#ifndef OPERATIONS_H
#define OPERATIONS_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "segments.h"
// #include "registers.h"
#include "commandParse.h"


void emulate_um(FILE *fp, char *name);


#endif