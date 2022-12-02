/**************************************************************
 *
 *                     commandParse.h
 *
 *      Assignment: comp40 Assignment 6: um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Interface of command parsing
 *
**************************************************************/

#ifndef COMMANDPARSE_H
#define COMMANDPARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
// #include <bitpack.h>


enum Word_val {
        OPCODE = 0, REG_A, REG_B, REG_C, VAL
};

void parse_command(uint32_t word, uint32_t *arr);


#endif