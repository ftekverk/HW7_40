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

        arr[OPCODE] = Bitpack_getu(word, OPCODE_WIDTH, OPCODE_LSB);
        assert(arr[OPCODE] <= 13);

        /* Load value case */
        if (arr[OPCODE] == 13){
                arr[REG_A] = Bitpack_getu(word, REG_WIDTH,   REG_A_LV_LSB);
                arr[VAL]   = Bitpack_getu(word, VALUE_WIDTH, VALUE_LSB);
                arr[REG_B] = -1;
                arr[REG_C] = -1;
        }
        /* Three register instructions */
        else {
                arr[REG_A] = Bitpack_getu(word, REG_WIDTH, REG_A_LSB);
                arr[REG_B] = Bitpack_getu(word, REG_WIDTH, REG_B_LSB);
                arr[REG_C] = Bitpack_getu(word, REG_WIDTH, REG_C_LSB);
                arr[VAL]   = -1;
        }


        return;
}