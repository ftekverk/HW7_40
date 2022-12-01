#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <seq.h>
#include <sys/stat.h>
#include <bitpack.h>




typedef struct Segments_T *Segments_T;


/* Allocation and Deallocation */
Segments_T segments_new(FILE *fp, char *name);
void segments_free(Segments_T *segs);

/* Map and Unmap */
uint32_t map_new(Segments_T segs, int numWords);
void unmap_segment(Segments_T segs, int id);
void store_word(Segments_T segs, uint32_t segment_ID, uint32_t offset, 
uint32_t word);
uint32_t get_word(Segments_T segs, uint32_t segment_ID, uint32_t offset);
void duplicate_seg(Segments_T segs, uint32_t segment_ID);

uint32_t get_length(Segments_T segs, uint32_t segment_ID);

#endif