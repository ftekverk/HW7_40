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



// void call_operation(Segments_T segs, uint32_t *registers,
//                     uint32_t *command_info, bool *keep_going, 
//                     uint32_t *pc, uint32_t **commands_arr_p);



void parse_command(uint32_t word, uint32_t *arr);



/* The various 4-bit opcode values at the start of 32-bit instructions */
enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
};

enum Word_val {
        OPCODE = 0, REG_A, REG_B, REG_C, VAL
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
        /* -------------------- PARSE COMMAND ------------------------------- */

                command_info[OPCODE] = (command << (32 - (OPCODE_LSB + OPCODE_WIDTH))) >> (32 - OPCODE_WIDTH);
                assert(command_info[OPCODE] <= 13);

                /* Load value case */
                if (command_info[OPCODE] == 13){
                        command_info[REG_A] = (command << (32 - (REG_A_LV_LSB + REG_WIDTH))) >> (32 - REG_WIDTH); 
                        command_info[VAL]   = (command << (32 - (VALUE_LSB + VALUE_WIDTH)))  >> (32 - VALUE_WIDTH);   
                }
                /* Three register instructions */
                else {
                        command_info[REG_A] = (command << (32 - (REG_A_LSB + REG_WIDTH))) >> (32 - REG_WIDTH);
                        command_info[REG_B] = (command << (32 - (REG_B_LSB + REG_WIDTH))) >> (32 - REG_WIDTH);
                        command_info[REG_C] = (command << (32 - (REG_C_LSB + REG_WIDTH))) >> (32 - REG_WIDTH);
                }


        /* -------------------- PARSE COMMAND ------------------------------- */

        /* 4.) Call the function needed based on the opcode */
        /* -------------------- CALL OPERATION ------------------------------- */

                uint32_t op_code = command_info[OPCODE];
                assert(op_code <= 13);

                uint32_t id, offset, reg_a_val, reg_b_val, reg_c_val, val_A,
                        val_B, val_C, seg_num, zero, val, full_word;

                int char_val;

                switch (op_code)
                {
                case CMOV :
                        if (registers[command_info[REG_C]] != 0)
                        {
                                reg_b_val = registers[command_info[REG_B]];
                                registers[command_info[REG_A]] = reg_b_val;
                        }
                        break;
                case SLOAD :
                        id = registers[command_info[REG_B]];
                        offset = registers[command_info[REG_C]];
                        reg_a_val = get_word(segs, id, offset);
                        registers[command_info[REG_A]] = reg_a_val;
                        break;
                case SSTORE :
                        id = registers[command_info[REG_A]];
                        offset = registers[command_info[REG_B]];
                        reg_c_val = registers[command_info[REG_C]];
                        store_word(segs, id, offset, reg_c_val);
                        break;
                case ADD :
                        val_B = registers[command_info[REG_B]];
                        val_C = registers[command_info[REG_C]];
                        registers[command_info[REG_A]] = val_B + val_C;
                        break;
                case MUL :
                        val_B = registers[command_info[REG_B]];
                        val_C = registers[command_info[REG_C]];
                        registers[command_info[REG_A]] = val_B * val_C;
                        break;
                case DIV :
                        val_B = registers[command_info[REG_B]];
                        val_C = registers[command_info[REG_C]];
                        assert(val_C != 0);
                        registers[command_info[REG_A]] = val_B / val_C;
                        break;
                case NAND :
                        val_B = registers[command_info[REG_B]];
                        val_C = registers[command_info[REG_C]];
                        val_A = ~(val_B & val_C);
                        registers[command_info[REG_A]] = val_A;
                        break;
                case HALT :
                        keep_going = false;
                        break;
                case ACTIVATE :
                        val_C = registers[command_info[REG_C]];
                        seg_num = map_new(segs, val_C);
                        registers[command_info[REG_B]] = seg_num;
                        break;
                case INACTIVATE :
                        val_C = registers[command_info[REG_C]];
                        unmap_segment(segs, val_C);
                        break;
                case OUT :
                        val = registers[command_info[REG_C]];
                        assert(val <= 255);
                        char character = val;
                        fprintf(stdout, "%c", character);
                        break;
                case IN :
                        char_val = fgetc(stdin);
                        if (char_val == EOF)
                        {
                                full_word = 0;
                                full_word = ~full_word;
                                registers[command_info[REG_C]] = full_word;
                        }
                        else
                        {
                                assert(char_val >= 0 && char_val <= 255);
                                registers[command_info[REG_C]] = char_val;
                        }
                                break;
                case LOADP :
                        reg_b_val = registers[command_info[REG_B]];
                        reg_c_val = registers[command_info[REG_C]];
                        zero = 0;
                        if (reg_b_val != zero)
                        {
                                duplicate_seg(segs, reg_b_val);
                                commands_arr = get_seg_zero(segs);
                        }
                        pc = reg_c_val;
                        break;
                case LV :
                        registers[command_info[REG_A]] = command_info[VAL];
                        break;
                }


/* -------------------- CALL OPERATION  ------------------------------- */
        }

        /* Free data */
        segments_free(&segs);
}
