/**************************************************************
 *
 *                     registers.h
 *
 *      Assignment: comp40 Assignment 6: um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Interface of um register operations (updating and 
 *      getting values from registers).
 *
**************************************************************/

#ifndef REGISTERS_H
#define REGISTERS_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


typedef struct Registers_T *Registers_T;

/* Allocation and Deallocation */
Registers_T registers_new();
void registers_free(Registers_T *regs);

/* Getters */
uint32_t get_register_value(Registers_T regs, int index);

/* Setters */
void set_register_value(Registers_T regs, int index, uint32_t value);

#endif