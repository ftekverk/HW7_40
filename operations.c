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

void call_operation(Segments_T segs, uint32_t *registers,
                    uint32_t *command_info, bool *keep_going, 
                    uint32_t *pc, uint32_t **commands_arr_p);


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
        uint32_t registers[8] = {0,0,0,0,0,0,0,0};
        uint32_t command_info[5] = {0,0,0,0,0};


        uint32_t pc = 0;
        uint32_t command = 0;
        bool keep_going = true;
        
        uint32_t *commands_arr = get_seg_zero(segs);

        /* Execute um program */
        while (keep_going){
                /* 1.) Get a command from segment 0 with offset of pc */
                command = commands_arr[pc];
                /* 2.) Update pc = pc + 1 */
                pc += 1;
                /* 3.) Call command parse to update command_info */
                parse_command(command, command_info);
                /* 4.) Call the function needed based on the opcode */
                call_operation(segs, registers, command_info, &keep_going, &pc, &commands_arr);
        }

        /* Free data */
        segments_free(&segs);
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
void call_operation(Segments_T segs, uint32_t *registers, 
        uint32_t *command_info, bool *keep_going, uint32_t *pc, uint32_t **commands_arr_p)
{

        uint32_t op_code = command_info[OPCODE];

        assert(op_code <= 13);
        
        if (op_code == CMOV){
                if (registers[command_info[REG_C]] != 0)
                {
                        uint32_t reg_b_val = registers[command_info[REG_B]];

                        registers[command_info[REG_A]] = reg_b_val;
                }
        }
        else if (op_code == SLOAD){
                uint32_t id = registers[command_info[REG_B]];
                uint32_t offset = registers[command_info[REG_C]];
                uint32_t reg_a_val = get_word(segs, id, offset);

                registers[command_info[REG_A]] = reg_a_val;
        }
        else if (op_code == SSTORE){
                uint32_t id = registers[command_info[REG_A]];
                uint32_t offset = registers[command_info[REG_B]];
                uint32_t reg_c_val = registers[command_info[REG_C]];
                store_word(segs, id, offset, reg_c_val);
        }
        else if (op_code == ADD){
                uint32_t val_B = registers[command_info[REG_B]];
                uint32_t val_C = registers[command_info[REG_C]];

                registers[command_info[REG_A]] = val_B + val_C;
        }
        else if (op_code == MUL){
                uint32_t val_B = registers[command_info[REG_B]];
                uint32_t val_C = registers[command_info[REG_C]];

                registers[command_info[REG_A]] = val_B * val_C;
        }
        else if (op_code == DIV){
                uint32_t val_B = registers[command_info[REG_B]];
                uint32_t val_C = registers[command_info[REG_C]];
                assert(val_C != 0);

                registers[command_info[REG_A]] = val_B / val_C;
        }
        else if (op_code == NAND){
                uint32_t val_B = registers[command_info[REG_B]];
                uint32_t val_C = registers[command_info[REG_C]];
                uint32_t val_A = ~(val_B & val_C);

                registers[command_info[REG_A]] = val_A;
        }
        else if (op_code == HALT){
                *keep_going = false;
        }
        else if (op_code == ACTIVATE){
                uint32_t val_C = registers[command_info[REG_C]];
                uint32_t seg_num = map_new(segs, val_C);
                registers[command_info[REG_B]] = seg_num;
        }
        else if (op_code == INACTIVATE){
                uint32_t val_C = registers[command_info[REG_C]];
                unmap_segment(segs, val_C);
        }
        else if (op_code == OUT){
                uint32_t val = registers[command_info[REG_C]];
                assert(val <= 255);
                char character = val;
                fprintf(stdout, "%c", character);
        }
        else if (op_code == IN){
                int char_val = fgetc(stdin);

                if (char_val == EOF){
                        uint32_t full_word = 0;
                        full_word = ~full_word;
                        registers[command_info[REG_C]] = full_word;
                }
                else{
                        assert(char_val >= 0 && char_val <= 255);
                        registers[command_info[REG_C]] = char_val;
                }
        }
        else if (op_code == LOADP){
                uint32_t reg_b_val = registers[command_info[REG_B]];
                uint32_t reg_c_val = registers[command_info[REG_C]];
        
                uint32_t zero = 0;

                if (reg_b_val != zero){
                        duplicate_seg(segs, reg_b_val);
                        *commands_arr_p = get_seg_zero(segs);
                }

                *pc = reg_c_val;
        }
        else if (op_code == LV){
                registers[command_info[REG_A]] = command_info[VAL];
        }
}