/**************************************************************
 *
 *                     um.c
 *
 *      Assignment: comp40 Assignment 6: um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Main for HW 6 : um. Executes the universal machine
 *
**************************************************************/

#include "operations.h"
#include <stdio.h>



int main(int argv, char *argc[])
{
        /* Open File */
        if (argv != 2){
                fprintf(stderr, "Usage : ./um filename\n");
                exit(EXIT_FAILURE);
        }
        FILE *fp = fopen(argc[1], "r");
        if(fp == NULL){
                fprintf(stderr, "%s: No such file or directory\n", argc[1]);
                exit(EXIT_FAILURE);
        }

        /* Run um*/
        emulate_um(fp, argc[1]);

        fclose(fp);
        return 0;       
}