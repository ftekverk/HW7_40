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
static inline Segments_T segments_new(FILE *fp, char *name);
static inline void segments_free(Segments_T *segs);

/* Map and Unmap */
static inline uint32_t map_new(Segments_T segs, int numWords);
static inline void unmap_segment(Segments_T segs, int id);
static inline void store_word(Segments_T segs, uint32_t segment_ID, uint32_t offset, 
uint32_t word);
static inline uint32_t get_word(Segments_T segs, uint32_t segment_ID, uint32_t offset);
static inline void duplicate_seg(Segments_T segs, uint32_t segment_ID);

static inline uint32_t get_length(Segments_T segs, uint32_t segment_ID);
static inline uint32_t *get_seg_zero(Segments_T segs);

#endif