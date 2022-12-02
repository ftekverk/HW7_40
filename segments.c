/**************************************************************
 *
 *                     segments.c
 *
 *      Assignment: comp40 Assignment 6: um
 *      Authors:  ftekve01, mvivia01 
 *      Date:     11/22/22
 *
 *      Implementation of um segment operations such as mapping
 *      and unmapping segments, and converting ID to address.
 *
**************************************************************/

#include "segments.h"

#define SEQ_HINT 10000

typedef struct Seg_info {
        uint32_t *arr;
        uint32_t length;
} *Seg_info;


/* Structure that contains information about segments, such as which are 
   in use, and which IDs have been recycled */
struct Segments_T {
    Seq_T unmapped_IDs; /* Stores pointers to ids */
    Seq_T mapped_IDs;   /* Stores structs {*32-bit arr, length} */
};

/*Internal helper functions */
uint32_t get_next_id(Segments_T segs);

void initialize_to_zeros(Seg_info seg_info);

/* Memory Management */
Seg_info make_new_segment(int numWords);
void free_mapped_segs(Segments_T segs);
void free_unmapped_segs(Segments_T segs);
void free_seg_info(Seg_info segment);

/* Populate zero segment */
void read_file_data(FILE *fp, char *name, Segments_T segs);
uint32_t new_word(FILE *input);



/* Allocation and Deallocation */

/* segmetns_new
 * Description : 
 *      allocate and initialize an instance of the segments struct
 *    
 * Input Expectations: 
 *      An open file pointer and the name of that file
 *
 * Output: 
 *      returns a pointer to the new struct
 */
Segments_T segments_new(FILE *fp, char *name)
{
        Segments_T segments = malloc(sizeof(struct Segments_T));
        assert(segments != NULL);

        segments->unmapped_IDs = Seq_new(SEQ_HINT);
        segments->mapped_IDs   = Seq_new(SEQ_HINT);

        read_file_data(fp, name, segments);

        return segments;
}


/* This must be the first thing to map */

/* read_file_data
 * Description : 
 *      Map the 0 segment of the UM and populate it with the words
 *      from the file. This is a helper function-- it should only be called
 *      in segments_new
 *    
 * Input Expectations: 
 *      The segment struct segs should not be NULL
 *      fp is the open file pointer to the .um file being emulated
 *      name is the name of the .um file being emulated
 *
 * Output: 
 *      N/A
 */
void read_file_data(FILE *fp, char *name, Segments_T segs)
{
        assert(segs != NULL);
        struct stat sb;
        stat(name, &sb);
        int size = sb.st_size;
        /* CHECK IF WE NEED THIS!!! */
        assert(size % 4 == 0);
        int numWords = size / 4;
        // Seg_info zeroSeg = make_new_segment(numWords);
        map_new(segs, numWords);

        for (int i = 0; i < numWords; i++){
                uint32_t word = new_word(fp);
                store_word(segs, 0, i, word);
        }

        return;
}


/* Read a word in big endian */

/* new_word
 * Description : 
 *      Read in the next word (4 bytes) from the input file
 *      This is a helper function to be called only in read_file_data
 *    
 * Input Expectations: 
 *      The input file pointer must not be NULL
 *
 * Output: 
 *      Returns an unsigned 32 bit integer that stores the next 4 bytes from
 *      the file in big endian order
 */
uint32_t new_word(FILE *input)
{
        assert(input != NULL);
        uint32_t word = 0;
        int c;
        const int byte_width = 8;

        /* read in big-endian order */
        for (int i = 3; i >= 0; i--) {
                c = fgetc(input);
                assert(c != EOF);
                word = Bitpack_newu(word, byte_width, i * byte_width, c);    
        }

        return word;
}

/* segments_free
 * Description : 
 *      free an instance of the segments struct
 *    
 * Input Expectations: 
 *      the pointer to the struct must not be NULL
 *
 * Output: 
 *     N/A
 */
void segments_free(Segments_T *segs)
{
        assert(segs != NULL);
        
        /* 1.) Free Mapped Segments */
        free_mapped_segs(*segs);

        /* 2.) Free sequence of recycled IDs */
        free_unmapped_segs(*segs);


        /* 3.) Free outer structure */
        free(*segs);
}

/* free_mapped_segs
 * Description : 
 *      When freeing the segments struct, this helper function frees all of
 *      the segments that have not been unmapped
 *    
 * Input Expectations: 
 *      the segments struct must not be NULL
 *
 * Output: 
 *      N/A
 */
void free_mapped_segs(Segments_T segs)
{
        assert(segs != NULL);
        
        /* Frees structs {array pointer, length} */
        Seq_T mapped = segs->mapped_IDs;
        int mapped_length = Seq_length(mapped);
        for(int i = 0; i < mapped_length; i++){
                Seg_info segment = (Seg_info) Seq_get(mapped, i);
                if (segment != NULL){
                        free_seg_info(segment);
                }
        }
        
        Seq_free(&mapped);
}

/* free_unmapped_segs
 * Description : 
 *      When freeing the segments struct, this helper function frees all of
 *      the unmapped segment IDs that are still stored in the sequence
 *    
 * Input Expectations: 
 *      the segments struct must not be NULL
 *
 * Output: 
 *      N/A
 */
void free_unmapped_segs(Segments_T segs)
{
        assert(segs != NULL);

        /* Frees uint32_t pointers */
        Seq_T unmapped = segs->unmapped_IDs;
        int unmapped_length = Seq_length(unmapped);
        for(int i = 0; i < unmapped_length; i++){
                free(Seq_get(unmapped, i));
        }
        Seq_free(&unmapped);

        return;
}

/* free_seg_info
 * Description : 
 *      When freeing the mapped segments in the segments struct, this helper
 *      function frees all of the segment info structs
 *    
 * Input Expectations: 
 *      the segmetns struct must not be NULL
 *
 * Output: 
 *      N/A
 */
void free_seg_info(Seg_info segment)
{
        assert(segment != NULL);
        free(segment->arr);
        free(segment);

        return;
}


/* map_new
 * Description : 
 *      Create a new segment initialized to be all zeros, reusing unmapped IDs
 *      if possible
 *    
 * Input Expectations: 
 *      The segments struct must not be NULL
 *      The number of words in the new segment must not be negative
 *
 * Output: 
 *      Returns the ID of the newly mapped segment
 */
uint32_t map_new(Segments_T segs, int numWords)
{
        assert(segs != NULL);
        assert(numWords > 0);

        /* 1.) Make a new segment */
        Seg_info newSeg = make_new_segment(numWords);

        /* 2.) Initialize segment values to 0 */
        initialize_to_zeros(newSeg);

        /* 3.) Add segment to mapped ID, checking if there are available
                unmapped IDs */
        /* Reuse available unmapped IDs*/
        if (Seq_length(segs->unmapped_IDs) != 0){
                uint32_t id = get_next_id(segs);
                Seq_put(segs->mapped_IDs, id, newSeg);
                return id;
        }
        /* Otherwise, use the next ID sequentially */
        else{
                Seq_addhi(segs->mapped_IDs, newSeg);
                return Seq_length(segs->mapped_IDs) - 1;
        }
        
}

/* make_new_segment
 * Description : 
 *      Allocate the memory needed to make a new segment of specified size
 *      the segments that have not been unmapped
 *    
 * Input Expectations: 
 *      The number of words in the new segment must not be negative
 *
 * Output: 
 *      Returns a Seg_info struct with an uninitialized array of the specified
 *      size and the size of the segment
 */
Seg_info make_new_segment(int numWords)
{
        assert(numWords > 0);

        Seg_info new_seg = malloc(sizeof(struct Seg_info));
        assert(new_seg != NULL);


        uint32_t *arr = malloc(numWords * sizeof(uint32_t));
        assert(arr != NULL);

        new_seg->length = numWords;
        new_seg->arr = arr;


        return new_seg;
}

/* initialize_to_zeros
 * Description : 
 *     Initialize the array of words in a segment to be all 0
 *    
 * Input Expectations: 
 *      The segments struct must not be NULL
 *
 * Output: 
 *     N/A
 */
void initialize_to_zeros(Seg_info seg_info)
{
        assert(seg_info != NULL);

        uint32_t zero = 0;
        uint32_t length = seg_info->length;


        for (uint32_t i = 0; i< length; i++)
        {
                seg_info->arr[i] = zero;
        }

        return;
}

/* unmap_segment
 * Description : 
 *     free a segment and allow its ID to be reused by adding it to the unmapped
 *      IDs sequence
 *    
 * Input Expectations: 
 *      The segments struct must not be NULL
 *      The ID given must be positve
 *
 * Output: 
 *     N/A
 */
void unmap_segment(Segments_T segs, int id)
{
        assert(segs != NULL);
        assert(id >= 0);
        
        /* 1.) Update array which stores segment addresses */
        Seg_info segment = (Seg_info) Seq_put(segs->mapped_IDs, id, NULL);
        free_seg_info(segment);
        
        /* 2.) Add id to recycled ids */
        uint32_t *id_to_add = malloc(sizeof(id));
        assert(id_to_add != NULL);
        *id_to_add = id;
        Seq_addhi(segs->unmapped_IDs, id_to_add);
        
        return;
}

/* get_next_id
 * Description : 
 *     Get an unmapped ID to be reused
 *    
 * Input Expectations: 
 *      The segments struct must not be NULL
 *
 * Output: 
 *     An unsigned 32-bit integer that is the unmapped ID to be reused
 */
uint32_t get_next_id(Segments_T segs)
{
        assert(segs != NULL);

        /* 1.) Remove ID stored at end of sequence */
        void* low_val = Seq_remhi(segs->unmapped_IDs);
        assert(low_val != NULL);
        
        /* 2.) Ensure proper type uint32_t*/
        uint32_t id = *(uint32_t *) low_val;
        free(low_val);
        return id;
}

/* get_word
 * Description : 
 *     Get the word stored in the given segment at the given offset
 *    
 * Input Expectations: 
 *      The segments struct must not be NULL
 *
 * Output: 
 *     The word stored at the given location
 */
uint32_t get_word(Segments_T segs, uint32_t segment_ID, uint32_t offset)
{
        assert(segs != NULL);

        /* 1.) Get Mapped Segment Sequence and array it stores */
        Seq_T mapped = segs->mapped_IDs;
        Seg_info currSeg = Seq_get(mapped, segment_ID);
        assert(currSeg != NULL);
        uint32_t *currSegArr = currSeg->arr;
        uint32_t length      = currSeg->length;

        assert(offset < length);
        
        return currSegArr[offset];
}

/* store_word
 * Description : 
 *     Store the given word in the given segment at the given offset
 *    
 * Input Expectations: 
 *      The segments struct must not be NULL
 *
 * Output: 
 *     N/A
 */
void store_word(Segments_T segs, uint32_t segment_ID, uint32_t offset,
                uint32_t word)
{
        assert(segs != NULL);
        
        /* 1.) Get Mapped Segment Sequence and array it stores */
        Seq_T mapped = segs->mapped_IDs;
        Seg_info currSeg = Seq_get(mapped, segment_ID);
        assert(currSeg != NULL);
        uint32_t *currSegArr = currSeg->arr;
        uint32_t length      = currSeg->length;
        
        /* 2.) Add word into offset */
        assert(offset < length);
        currSegArr[offset] = word;

        return;
}

/* overwrite the zero seg with info at a new segment ID */

/* duplicate_seg
 * Description : 
 *     duplicate the given segment and store in segment 0
 *    
 * Input Expectations: 
 *      The segments struct must not be NULL
 *      segment_ID should refer to a mapped segment
 *
 * Output: 
 *     N/A
 */

void duplicate_seg(Segments_T segs, uint32_t segment_ID)
{
        assert(segs != NULL);

        /* 1.) Get Segment to Copy */
        Seq_T mapped = segs->mapped_IDs;
        Seg_info currSeg = (Seg_info) Seq_get(mapped, segment_ID);
        assert(currSeg != NULL);
        
        /* 2.) Create a new segment */
        Seg_info copySeg = make_new_segment(currSeg->length);

        /* 3.) Copy Array Info */
        for (uint32_t i = 0; i < copySeg->length; i++){
                (copySeg->arr)[i] = (currSeg->arr)[i];
        }
        
        /* Not casting as a seg info */
        Seg_info zeroSeg = (Seg_info) Seq_put(mapped, 0, copySeg);
        free_seg_info(zeroSeg);    

        return;
}


/* get_length
 * Description : 
 *     get the number of words in the given segment
 *    
 * Input Expectations: 
 *      The segments struct must not be NULL
 *      segment_ID should refer to a mapped segment
 *
 * Output: 
 *     The length of the given segment as a uint32_t
 */
uint32_t get_length(Segments_T segs, uint32_t segment_ID)
{
        assert(segs != NULL);

        Seq_T mapped = segs->mapped_IDs;
        Seg_info currSeg = Seq_get(mapped, segment_ID);
        assert(currSeg != NULL);
        return currSeg->length;
}

uint32_t *get_seg_zero(Segments_T segs)
{
       return ((Seg_info) Seq_get(segs->mapped_IDs, 0))->arr;
}

