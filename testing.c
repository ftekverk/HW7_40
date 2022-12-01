/**************************************************************
 *
 *                     testing.c
 *
 *      Assignment: comp40 Assignment 6: um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Runs various testing files for HW 6 : um
 *
**************************************************************/

#include "registers.h"
#include "segments.h"
#include "commandParse.h"
#include <stdio.h>

/* Register Testing */
void test_register();
void print_all_registers();

/* Segment Testing */
void test_segment(FILE *fp, char *name);
void test_map_unmap(Segments_T seg);
void unmap_segs(Segments_T seg);
void map_segs(Segments_T seg);


void test_get_store(Segments_T seg);
void test_duplicate(Segments_T seg);
void test_file_read(Segments_T seg);

void test_parsing();
uint32_t packWord(uint32_t op_code, uint32_t reg_a, uint32_t reg_b, 
        uint32_t reg_c, uint32_t value);




int main(int argv, char *argc[])
{
        // test_register();
        assert(argv == 2);
        

        FILE *fp = fopen(argc[1], "r");
        assert(fp);
        
        test_segment(fp, argc[1]);
        // test_parsing();
        // test_operations(fp, argc[1]);
        fclose(fp);
        return 0;
}


/* ---- Operation Testing ---- */











/* ---- Parsing Testing ---- */

void test_parsing()
{

        uint32_t op_code = 13;
        uint32_t reg_a = 7;
        uint32_t reg_b = -1;
        uint32_t reg_c = -1;
        uint32_t value = 30;

        uint32_t vals[5] = {op_code, reg_a, reg_b, reg_c, value};

        uint32_t arr[5];
        uint32_t word = packWord(op_code, reg_a, reg_b, reg_c, value);


        parse_command(word, arr);
        for (int i = 0; i < 5; i++){
                assert(vals[i] == arr[i]);
        }
        fprintf(stderr, "\n");
}

uint32_t packWord(uint32_t op_code, uint32_t reg_a, uint32_t reg_b, 
        uint32_t reg_c, uint32_t value)
{
        uint32_t word = 0;
        word = Bitpack_newu(word, 4, 28, op_code);

        if (op_code < 13){
                word = Bitpack_newu(word, 3, 6,  reg_a);
                word = Bitpack_newu(word, 3, 3,  reg_b);
                word = Bitpack_newu(word, 3, 0,  reg_c);
        }
        else if (op_code == 13){
                word = Bitpack_newu(word, 3, 25,  reg_a);
                word = Bitpack_newu(word, 25, 0,  value);
        }


        return word;
}

// 4 1111 0001 0010 0011 0100 0101 0110














/* ---- Segment Testing ---- */

void test_segment(FILE *fp, char *name)
{
        Segments_T seg = segments_new(fp, name);
        

        // test_file_read(seg);

        // test_map_unmap(seg);
        // test_get_store(seg);
        test_duplicate(seg);

        
        // fprintf(stderr, "here\n");
        // store_word(seg, 0, 200, 0);
        // fprintf(stderr, "hello\n");

        
        segments_free(&seg);

}

void test_file_read(Segments_T seg)
{
        for (int i = 0; i < 12; i++){
                fprintf(stderr, "%x ", get_word(seg, 0, i));
        }
        fprintf(stderr, "\n");
}


void test_map_unmap(Segments_T seg)
{
        map_segs(seg);
        /* at end [ zero, 10, 1000, 100, 20 ]*/

        unmap_segs(seg);

}

void map_segs(Segments_T seg)
{
        map_new(seg, 10); 
        map_new(seg, 100);
        map_new(seg, 1000);
        map_new(seg, 10000);
}

void unmap_segs(Segments_T seg)
{
        unmap_segment(seg, 1); 
        unmap_segment(seg, 2); 
        unmap_segment(seg, 3); 
        unmap_segment(seg, 4); 
}


/* Out of bounds Offset creates valgrind errors*/
void test_get_store(Segments_T seg)
{
        map_segs(seg);

        for (int i = 0; i < 20; i++){
                store_word(seg, 2, i, i);
        }

        // for (int i = 0; i < 25; i++){
        //         store_word(seg, 0, 4*i, i);
        // }

        for (int i = 0; i < 100; i++){
                // fprintf(stderr, "%x ", get_word(seg, 0, i));
        }

        for (int i = 0; i < 10; i++){
                store_word(seg, 1, i, i);
        }

        for (int i = 0; i < 100; i++){
                // fprintf(stderr, "%x ", get_word(seg, 1, i));
        }  


        unmap_segs(seg);
}


void test_duplicate(Segments_T seg)
{
        map_segs(seg);
        duplicate_seg(seg, 2);

        for (int i = 0; i < 100; i++){
                assert(get_word(seg, 0, i) == get_word(seg, 2, i));
        }
}







/* ---- Register Testing ---- */

/* call register functions for testing */
void test_register()
{
        Registers_T registers = registers_new();

        print_all_registers(registers);
        for(int i = 0; i < 8; i++)
        {
                set_register_value(registers, i+12, i + 0xffffffff);
        }

        print_all_registers(registers);

        registers_free(&registers);

}

void print_all_registers(Registers_T regs)
{
        for(int i = 0; i < 8; i ++){
                fprintf(stdout, "%x\n", get_register_value(regs, i) );
        }
}
