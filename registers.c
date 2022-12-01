/**************************************************************
 *
 *                     registers.c
 *
 *      Assignment: comp40 Assignment 6: um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Implementation of um register operations (updating and 
 *      getting values from registers).
 *
**************************************************************/


#include "registers.h"



struct Registers_T {
        uint32_t registers[8]; 
};


/* registers_new
 * Description : 
 *      allocate and initialize an instance of the registers struct
 *    
 * Input Expectations: 
 *      N/A
 *
 * Output: 
 *      returns a pointer to the new struct
 */
Registers_T registers_new()
{
        Registers_T registers = malloc(sizeof(struct Registers_T));
        assert(registers != NULL);

        uint32_t zero = 0;
        for(int i = 0; i < 8; i++){
                (registers->registers)[i] = zero;
        }
        
        return registers;
}

/* registers_free
 * Description : 
 *      free the allocated memory for an instance of the registers struct
 *    
 * Input Expectations: 
 *      A pointer to a registers struct that should not be NULL
 *
 * Output: 
 *      none
 */
void registers_free(Registers_T *regs)
{
        assert(regs != NULL);
        free(*regs);
}

/* get_register_value
 * Description : 
 *      get the value from a register
 *    
 * Input Expectations: 
 *      A pointer to a registers struct that should not be NULL
 *      An index in the range 0 to 7 (inclusive)
 *
 * Output: 
 *      returns the value in the given register
 */
uint32_t get_register_value(Registers_T regs, int index)
{
        assert(regs != NULL);
        assert(index >= 0 && index <= 7);
        

        return (regs->registers)[index];       
}

/* set_registe_value
 * Description : 
 *      set the value in a register
 *    
 * Input Expectations: 
 *      A pointer to a registers struct that should not be NULL
 *      An index in the range 0 to 7 (inclusive)
 *      An unsigned 32-bit value
 *
 * Output: 
 *      N/A
 */
void set_register_value(Registers_T regs, int index, uint32_t value)
{
        assert(regs != NULL);
        assert(index >= 0 && index <= 7);
        

        (regs->registers)[index] = value;       
}