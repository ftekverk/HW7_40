/**************************************************************
 *
 *                     commandParse.c
 *
 *      Assignment: comp40 Assignment 6 : um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Implementation of a function to read a 32-bit word and
 *      update the values of a size 5 array t
 *
**************************************************************/

#include "commandParse.h"

#define OPCODE_LSB 28
#define OPCODE_WIDTH 4

#define REG_WIDTH 3
#define REG_A_LSB 6
#define REG_B_LSB 3
#define REG_C_LSB 0

/* IF OP CODE == 13 */
#define REG_A_LV_LSB 25
#define VALUE_LSB 0
#define VALUE_WIDTH 25



/* parse_command
 * Description : 
 *      Uses the bitpack interface to update opcode, registers a-b, and value
 *      packed into 32-bit codewords.
 *    
 * Input Expectations: 
 *      It is a checked runtime error arr to be NULL. arr is expected to be a
 *      pointer to a 5 element array storing uint32_t : 
 *      [opcode, regA, regB, regC, value]
 *
 * Output: 
 *      void. arr is updated by reference.
 */
void parse_command(uint32_t word, uint32_t *arr)
{
        assert(arr != NULL);

        arr[OPCODE] = (word << (32 - (OPCODE_LSB + OPCODE_WIDTH))) >> (32 - OPCODE_WIDTH);
        assert(arr[OPCODE] <= 13);

        /* Load value case */
        if (arr[OPCODE] == 13){
                arr[REG_A] = (word << (32 - (REG_A_LV_LSB + REG_WIDTH))) >> (32 - REG_WIDTH); 
                arr[VAL]   = (word << (32 - (VALUE_LSB + VALUE_WIDTH)))  >> (32 - VALUE_WIDTH);   
        }
        /* Three register instructions */
        else {
                arr[REG_A] = (word << (32 - (REG_A_LSB + REG_WIDTH))) >> (32 - REG_WIDTH);
                arr[REG_B] = (word << (32 - (REG_B_LSB + REG_WIDTH))) >> (32 - REG_WIDTH);
                arr[REG_C] = (word << (32 - (REG_C_LSB + REG_WIDTH))) >> (32 - REG_WIDTH);
        }
}