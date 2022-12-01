/**************************************************************
 *
 *                     operations.c
 *
 *      Assignment: comp40 Assignment 6 : um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Implementation of the 14 required operations of the um.
 *      Exports one function, which handles the um emulation.
 *
**************************************************************/

#include "operations.h"

void call_operation(Segments_T segs, Registers_T registers, 
        uint32_t *command_info, bool *keep_going, uint32_t *pc);

void conditional_move(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void segmented_load(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void segmented_store(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void addition(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void multiplication(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void division(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void bitwise_nand(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void map_segment(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void unmap(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void output(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void input(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);
void load_program(Segments_T segs, Registers_T registers, 
                uint32_t *command_info, uint32_t *pc);
void load_value(Segments_T segs, Registers_T registers, 
                uint32_t *command_info);

/* The various 4-bit opcode values at the start of 32-bit instructions */
enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
};



/* emulate_um
 * Description : 
 *      Main operation for the um. Creates a segments instance, and register 
 *      instance. Parses commands from .um files and executes commands according
 *      to opcodes and other bitpacked values.
 *    
 * Input Expectations: 
 *      It is a checked runtime error for fp or name to be NULL.
 *
 * Side Effects:
 *      Information can be printed to standard out.
 *
 * Output: 
 *      void
 */
void emulate_um(FILE *fp, char *name)
{
        assert(fp != NULL);
        assert(name != NULL);

        /* Make the segments, registers, and command info array */
        Segments_T  segs = segments_new(fp, name);
        Registers_T registers = registers_new();
        uint32_t command_info[5] = {0,0,0,0,0};


        uint32_t pc = 0;
        uint32_t command = 0;
        bool keep_going = true;
        
        /* Execute um program */
        while (keep_going){
                /* 1.) Get a command from segment 0 with offset of pc */
                command = get_word(segs, 0, pc);
                /* 2.) Update pc = pc + 1 */
                pc += 1;
                /* 3.) Call command parse to update command_info */
                parse_command(command, command_info);
                /* 3.) Call the function needed based on the opcode */
                call_operation(segs, registers, command_info, &keep_going, &pc);
                
                /* 4.) Update if we reach the end of the segment */
                // keep_going = keep_going && (pc < get_length(segs, 0));
        }

        /* Free data */
        segments_free(&segs);
        registers_free(&registers);
}

/* call_operation
 * Description : 
 *      Calls the corresponding function based on provided command info opcode.
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Output: 
 *      void
 */
void call_operation(Segments_T segs, Registers_T registers, 
        uint32_t *command_info, bool *keep_going, uint32_t *pc)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);
        assert(keep_going != NULL);
        assert(pc != NULL);


        uint32_t op_code = command_info[OPCODE];

        assert(op_code <= 13);
        
        if (op_code == CMOV){
                conditional_move(segs, registers, command_info);
        }
        else if (op_code == SLOAD){
                segmented_load(segs, registers, command_info);
        }
        else if (op_code == SSTORE){
                segmented_store(segs, registers, command_info);
        }
        else if (op_code == ADD){
                addition(segs, registers, command_info); 
        }
        else if (op_code == MUL){
                multiplication(segs, registers, command_info);
        }
        else if (op_code == DIV){
                division(segs, registers, command_info);
        }
        else if (op_code == NAND){
                bitwise_nand(segs, registers, command_info);
        }
        else if (op_code == HALT){
                *keep_going = false;
        }
        else if (op_code == ACTIVATE){
                map_segment(segs, registers, command_info);
        }
        else if (op_code == INACTIVATE){
                unmap(segs, registers, command_info);
        }
        else if (op_code == OUT){
                output(segs, registers, command_info);
        }
        else if (op_code == IN){
                input(segs, registers, command_info);
        }
        else if (op_code == LOADP){
                load_program(segs, registers, command_info, pc);
        }
        else if (op_code == LV){
                load_value(segs, registers, command_info);
        }
}


/* conditional_move
 * Description : 
 *      Updates the value of a register conditional on the value of another reg.
 *              If $r[C] != 0, $r[A] = $r[B].
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      Registers can be updated.
 *
 * Output: 
 *      void
 */
void conditional_move(Segments_T segs, Registers_T registers, 
                uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);


        if (get_register_value(registers, command_info[REG_C]) != 0){
                uint32_t reg_b_val = get_register_value(registers, 
                                                 command_info[REG_B]);
                
                set_register_value(registers, command_info[REG_A], reg_b_val);
        }
        
        return;
}


/* segmented_load
 * Description : 
 *      Updates a register with the 32-bit word stored in a segment with an 
 *      offset:
 *              $r[A] = m[$r[B]][$r[C]]
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      Registers are updated.
 *
 * Output: 
 *      void
 */
void segmented_load(Segments_T segs, Registers_T registers, 
                uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        uint32_t id = get_register_value(registers, command_info[REG_B]);
        uint32_t offset = get_register_value(registers, command_info[REG_C]);
        uint32_t reg_a_val = get_word(segs, id, offset);
        
        set_register_value(registers, command_info[REG_A], reg_a_val);
}


/* segmented_store
 * Description : 
 *      Updates the 32-bit word stored in a segment with an offset to value
 *      stored in a register.
 *             m[$r[A]][$r[B]] =  $r[C] 
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      A segment is updated.
 *
 * Output: 
 *      void
 */
void segmented_store(Segments_T segs, Registers_T registers, 
                uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        uint32_t id = get_register_value(registers, command_info[REG_A]);
        uint32_t offset = get_register_value(registers, command_info[REG_B]);
        uint32_t reg_c_val = get_register_value(registers, command_info[REG_C]);
        store_word(segs, id, offset, reg_c_val);
}


/* addition
 * Description : 
 *      Adds two registers, storing the result in a third.
 *             $r[A] = ($r[B] + $r[C]) % 2^32
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      A register value is updated.
 *
 * Output: 
 *      void
 */
void addition(Segments_T segs, Registers_T registers, 
                uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        uint32_t val_B = get_register_value(registers, command_info[REG_B]);
        uint32_t val_C = get_register_value(registers, command_info[REG_C]);
        set_register_value(registers, command_info[REG_A], val_B + val_C);
}



/* multiplication
 * Description : 
 *      Multiplies two registers, storing the result in a third.
 *             $r[A] = ($r[B] * $r[C]) % 2^32
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      A register value is updated.
 *
 * Output: 
 *      void
 */
void multiplication(Segments_T segs, Registers_T registers, 
                uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        uint32_t val_B = get_register_value(registers, command_info[REG_B]);
        uint32_t val_C = get_register_value(registers, command_info[REG_C]);
        set_register_value(registers, command_info[REG_A], val_B * val_C);
}


/* division
 * Description : 
 *      Divides two registers, storing the result in a third.
 *             $r[A] = ($r[B] / $r[C]) % 2^32
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      A register value is updated.
 *
 * Output: 
 *      void
 */
void division(Segments_T segs, Registers_T registers, uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        /* Get register values for division */
        uint32_t val_B = get_register_value(registers, command_info[REG_B]);
        uint32_t val_C = get_register_value(registers, command_info[REG_C]);
        assert(val_C != 0);

        set_register_value(registers, command_info[REG_A], val_B / val_C);
}


/* bitwise_nand
 * Description : 
 *      Performs logical NAND of two registers, storing the result in a third.
 *             $r[A] = ~($r[B] & $r[C])
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      A register value is updated.
 *
 * Output: 
 *      void
 */
void bitwise_nand(Segments_T segs, Registers_T registers, 
                        uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        /* Perform nand and store*/
        uint32_t val_B = get_register_value(registers, command_info[REG_B]);
        uint32_t val_C = get_register_value(registers, command_info[REG_C]);
        uint32_t val_A = ~(val_B & val_C);

        set_register_value(registers, command_info[REG_A], val_A);
}




/* map_segment
 * Description : 
 *      Creates a new segment with number of words in $r[C]. Words are 
 *      initialized to 0. The unique 32-bit ID is placed in $r[B].
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      A segment is created. Register value is updated.
 *
 * Output: 
 *      void
 */
void map_segment(Segments_T segs, Registers_T registers, uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        uint32_t val_C = get_register_value(registers, command_info[REG_C]);
        uint32_t seg_num = map_new(segs, val_C);
        
        set_register_value(registers, command_info[REG_B], seg_num);
}


/* unmap
 * Description : 
 *      Frees the segment $m[$r[C]], recylcing the ID for future use.
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      A segment is freed in memory.
 *
 * Output: 
 *      void
 */
void unmap(Segments_T segs, Registers_T registers, uint32_t *command_info)
{
        
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);
        
        uint32_t val_C = get_register_value(registers, command_info[REG_C]);

        unmap_segment(segs, val_C);
}


/* output
 * Description: 
 *      Value in $r[C] is written to stdout
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      Information is written to stdout.
 *
 * Output: 
 *      void
 */
void output(Segments_T segs, Registers_T registers, uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        uint32_t val = get_register_value(registers, command_info[REG_C]);
        assert(val <= 255);

        char character = val;

        fprintf(stdout, "%c", character);
}



/* input
 * Description : 
 *      Stdin is read and put into $r[C].
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Output: 
 *      void
 */
void input(Segments_T segs, Registers_T registers, uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        int char_val = fgetc(stdin);

        if (char_val == EOF){
                uint32_t full_word = 0;
                full_word = ~full_word;
                set_register_value(registers, command_info[REG_C], full_word);
        }
        else{
                assert(char_val >= 0 && char_val <= 255);
                set_register_value(registers, command_info[REG_C], char_val);
        }

}


/* load_program
 * Description : 
 *      Duplicates segment $m[$r[B]] and replaces $m[0]. Program counter is
 *      set to $r[C].
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      Segment 0 and program counter are changed.
 *
 * Output: 
 *      void
 */
void load_program(Segments_T segs, Registers_T registers, 
                uint32_t *command_info, uint32_t *pc)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);
        assert(pc != NULL);

        uint32_t reg_b_val = get_register_value(registers, command_info[REG_B]);
        uint32_t reg_c_val = get_register_value(registers, command_info[REG_C]);
        
        uint32_t zero = 0;

        if (reg_b_val != zero){
                duplicate_seg(segs, reg_b_val);
        }

        *pc = reg_c_val;

        return;
}



/* load_value
 * Description : 
 *      Loads a value into a register, following a new format for 32-bit word.
 *    
 * Input Expectations: 
 *      It is a checked runtime error for any of the arguments to be NULL.
 *
 * Side Effects:
 *      Register value is updated.
 *
 * Output: 
 *      void
 */
void load_value(Segments_T segs, Registers_T registers, 
                uint32_t *command_info)
{
        assert(segs != NULL);
        assert(registers != NULL);
        assert(command_info != NULL);

        set_register_value(registers, command_info[REG_A], command_info[VAL]);
}
