/*
 * umlab.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>

#define REGISTER_WIDTH 3
#define OPCODE_WIDTH 4

typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc)
{
        Um_instruction word = 0;
        word =  Bitpack_newu(word, REGISTER_WIDTH, 6, ra);
        word =  Bitpack_newu(word, REGISTER_WIDTH, 3, rb);
        word =  Bitpack_newu(word, REGISTER_WIDTH, 0, rc);
        word =  Bitpack_newu(word, OPCODE_WIDTH, 28, op);
        

        return word;
}



Um_instruction loadval(unsigned ra, unsigned val)
{
        Um_instruction word = 0;
        word = Bitpack_newu(word, OPCODE_WIDTH, 28, LV);
        word = Bitpack_newu(word, REGISTER_WIDTH, 25, ra);
        word = Bitpack_newu(word, 25, 0, val);

        return word;
}


/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

Um_instruction conditional_move(Um_register a, Um_register b, Um_register c)
{
        return three_register(CMOV, a, b, c);

}

Um_instruction seg_load(Um_register a, Um_register b, Um_register c)
{
        return three_register(SLOAD, a, b, c);

}
Um_instruction seg_store(Um_register a, Um_register b, Um_register c)
{
        return three_register(SSTORE, a, b, c);

}

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}


static inline Um_instruction mult(Um_register a, Um_register b, Um_register c)
{
        return three_register(MUL, a, b, c);

}
static inline Um_instruction div(Um_register a, Um_register b, Um_register c)
{
        return three_register(DIV, a, b, c);

}

static inline Um_instruction nand(Um_register a, Um_register b, Um_register c)
{
        return three_register(NAND, a, b, c);

}

Um_instruction map_seg(Um_register b, Um_register c)
{
        return three_register(ACTIVATE, 0, b, c);

}

Um_instruction unmap_seg(Um_register c)
{
        return three_register(INACTIVATE, 0, 0, c);

}

Um_instruction output(Um_register c)
{
        return three_register(OUT, 0, 0, c);
}

Um_instruction input(Um_register c)
{
        return three_register(IN, 0, 0, c);
}

Um_instruction load_p(Um_register b, Um_register c)
{
        return three_register(LOADP, 0, b, c);
}






/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }
      
}


/* Unit tests for the UM */

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, halt());
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}

void build_add_test(Seq_T stream)
{
        append(stream, add(r1, r2, r3));
        append(stream, halt());
}

void build_print_digit_test(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));

        append(stream, halt());

}




/* --- Tests we added ---- */
/* 1.) */
void build_cond_move_test(Seq_T stream)
{
        /* ra = r1, rb = r2, rc = r3 */
        /* test when r[c] = 0 */
        append(stream, loadval(r1, 64));
        append(stream, loadval(r2, 70));
        append(stream, loadval(r3, 0));
        append(stream, conditional_move(r1, r2, r3));
        append(stream, output(r1));
        /* Expect r1 = 64 = '@' */

        /* test when r[c] != 0 */
        append(stream, loadval(r3, 1));
        append(stream, conditional_move(r1, r2, r3));
        append(stream, output(r1));
        /* Expect r1 = 70 = 'F' */


        append(stream, halt());

        /* Expect "348" ? Maybe ascii vals */ 
}

/* 2.) */
void build_mult_div_test(Seq_T stream)
{

        append(stream, loadval(r1,33));
        append(stream, loadval(r2, 3));
        append(stream, mult(r3, r1, r2));
        append(stream, output(r3));
        /* expect 99 -> c */
        append(stream, div(r4, r3, r2));
        append(stream, output(r4));
        /* expected 33 -> ! */
        append(stream, loadval(r7, '\n'));
        append(stream, output(r7));

        append(stream, halt());
}

/* 3.) */
void build_nand_test(Seq_T stream)
{
        uint32_t val1 = 0xf2;   // 0000 0000 0000 0000 0000 0000 1111 0010 
        uint32_t val2 = 0x7d;   // 0000 0000 0000 0000 0000 0000 0111 1101
                                // 0000 0000 0000 0000 0000 0000 0111 0000 
        append(stream, loadval(r1, val1));
        append(stream, loadval(r2, val2));
        append(stream, nand(r3, r1, r2));
        append(stream, nand(r4, r3, r3));


        append(stream, output(r4));

        /* expect 126 -> ~ */

        
        append(stream, halt());
}


/* 4.) */
/* might have to come back to this */
void build_map_unmap_test(Seq_T stream)
{
        uint32_t length = 255;

        for (int i = 0; i < 128; i++){
                append(stream, loadval(r1, i+10));
                append(stream, map_seg(r2, r1));
                append(stream, output(r2));
        }
        append(stream, loadval(r7, length));
        append(stream, loadval(r6, '\n'));
        append(stream, output(r6));
        append(stream, output(r6));

        /* Test IDs are being recycled */
        for (int i = 126; i>33; i -= 2){
                append(stream, loadval(r4, i));
                append(stream, unmap_seg(r4));
                append(stream, loadval(r4, i - 1));
                append(stream, unmap_seg(r4));
                append(stream, map_seg(r2, r7));
                append(stream, output(r2));
        }
      
        append(stream, halt());
}


/* 5.) */
/* Come back and ask about halt */
void build_seg_load_store_test(Seq_T stream)
{
        append(stream, loadval(r1, 200));
        for (int i = 0; i < 200; i++){
                append(stream, map_seg(r2, r1));
                append(stream, output(r2));
        }

        for (int i = 0; i < 200; i++){
                append(stream, loadval(r3, i));
                append(stream, seg_store(r3, r3, r3));
        }

        for (int i = 0; i < 200; i++){
                append(stream, loadval(r3, i));
                append(stream, seg_load(r4, r3, r3));
                append(stream, output(r4));
        }

        append(stream, halt());
}

/* 6.) */
void build_load_prog_test(Seq_T stream)
{
        append(stream, loadval(r1, 5));        
        append(stream, loadval(r2, 0));

        /* Skip to the 5th instruction */
        append(stream, load_p(r2,r1));
        /* these instructions should be skipped so '!' shouldn't be printed */
        append(stream, loadval(r1, '!'));  
        append(stream, output(r1));

        /* should jump to here, output '@' */
        append(stream, loadval(r4, '@'));        
        append(stream, output(r4));

        append(stream, loadval(r1, 10));
        append(stream, map_seg(r4, r1));

       

        append(stream, loadval(r5, 0xa00000));
        append(stream, loadval(r6, 0x100));
        append(stream, mult(r5, r5, r6));
        append(stream, loadval(r6, 0x6));
        append(stream, add(r7, r5, r6));
        /* (0xa00000 x 0x100) + 0x6 = 0xa0000006 -> output register 6 */
        append(stream, loadval(r2, 0));
        append(stream, seg_store(r4,r2,r7));

        /* r5 still contains 0xa0000000 -> output register 0*/
        append(stream, loadval(r2, 1));
        append(stream, seg_store(r4,r2,r5));



        
        /* 0x700000 x 0x100 = 0x70000000 -> halt */
        append(stream, loadval(r5, 0x700000));
        append(stream, loadval(r6, 0x100));
        append(stream, mult(r7, r5, r6));
        append(stream, loadval(r2, 2));
        append(stream, seg_store(r4,r2,r7));
        /* seg 1 now holds output reg 6, output reg 0, and halt */
        /* load 'Q' into 6 and newline into 0 to print */
        append(stream, loadval(r6, 'Q'));
        append(stream, loadval(r0, '\n'));

        /* also ran a test when r2 was not reset, so program counter was 
         * 1 not 0. It did not print 'Q\n' and just halted as expected, as halt
         * was the instruction with ofset 2*/
        append(stream, loadval(r2, 0));
        append(stream, load_p(r4,r2));

        /* should not print these */
        /* should execute program that was in seg 1 -> output 'Q\n' and halt */
        /* if adding load program insturction is ommitted it does print these */
        append(stream, loadval(r4, 'B'));        
        append(stream, output(r4));
        append(stream, loadval(r4, 'a'));        
        append(stream, output(r4));
        append(stream, loadval(r4, 'd'));        
        append(stream, output(r4));
        append(stream, loadval(r4, '\n'));        
        append(stream, output(r4));
        append(stream, halt());

  
}

/* 7.) */
void build_fifty_mil(Seq_T stream)
{
        
        for(int i = 0; i < 49999999; i++){
                append(stream, loadval(i % 7, i % 255));
        }

        append(stream, halt());
}

void build_input_test(Seq_T stream)
{
        for (int i = 0; i < 10; i++){
                append(stream, input(r0));
                append(stream, output(r0));
        }

        append(stream, halt());
}