

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

#define SEQ_HINT 10000
#define EXPAND_CONST 2

typedef struct Seg_info {
        uint32_t *arr;
        uint32_t length;
} *Seg_info;


/* Structure that contains information about segments, such as which are 
   in use, and which IDs have been recycled */
struct Segments_T {
//     Seq_T unmapped_IDs; /* Stores pointers to ids */
//     Seq_T mapped_IDs;   /* Stores structs {*32-bit arr, length} */

        uint32_t *unmapped_IDs;
        unsigned unmapped_length;
        unsigned num_unmapped_IDs;

        Seg_info *mapped_IDs;
        unsigned mapped_length;
        unsigned num_segments;
};

/*Internal helper functions */
static inline uint32_t get_next_id(Segments_T segs);

static inline void initialize_to_zeros(Seg_info seg_info);

/* Memory Management */
static inline Seg_info make_new_segment(int numWords);
static inline void free_mapped_segs(Segments_T segs);
static inline void free_unmapped_segs(Segments_T segs);
static inline void free_seg_info(Seg_info segment);

/* Populate zero segment */
static inline void read_file_data(FILE *fp, char *name, Segments_T segs);
static inline uint32_t new_word(FILE *input);

static inline void expand(Segments_T segs);
static inline void expand_unmapped(Segments_T segs);



// void call_operation(Segments_T segs, uint32_t *registers,
//                     uint32_t *command_info, bool *keep_going, 
//                     uint32_t *pc, uint32_t **commands_arr_p);



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


static inline Segments_T segments_new(FILE *fp, char *name)
{
        Segments_T segments = malloc(sizeof(struct Segments_T));
        assert(segments != NULL);

        segments->unmapped_IDs = malloc(sizeof(uint32_t) * SEQ_HINT);        
        segments->mapped_IDs = malloc(sizeof(struct Seg_info) * SEQ_HINT);
        assert(segments->mapped_IDs != NULL);
        
        segments->mapped_length = SEQ_HINT;
        segments->num_segments = 0;

        segments->num_unmapped_IDs = 0;
        segments->unmapped_length = SEQ_HINT;

        for (unsigned i = 0; i < SEQ_HINT; i++)
        {
                (segments->mapped_IDs)[i] = NULL;
        }

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
static inline void read_file_data(FILE *fp, char *name, Segments_T segs)
{
        assert(segs != NULL);
        struct stat sb;
        stat(name, &sb);
        int size = sb.st_size;
        /* CHECK IF WE NEED THIS!!! */
        assert(size % 4 == 0);
        int numWords = size / 4;
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
static inline uint32_t new_word(FILE *input)
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
static inline void segments_free(Segments_T *segs)
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
 /* Frees structs {array pointer, length} */
static inline void free_mapped_segs(Segments_T segs)
{
        assert(segs != NULL);
        
        Seg_info *mapped = segs->mapped_IDs;

        /* Iterate over the length of the array */
        unsigned length = segs->num_segments;
        for(unsigned i = 0; i < length; i++){
                Seg_info segment = mapped[i];
                if (segment != NULL){
                        free_seg_info(segment);
                }
        }

        free(mapped);
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
static inline void free_unmapped_segs(Segments_T segs)
{
        assert(segs != NULL);

        /* Frees uint32_t pointers */
        uint32_t* unmapped = segs->unmapped_IDs;
        free(unmapped);

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
static inline void free_seg_info(Seg_info segment)
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
static inline uint32_t map_new(Segments_T segs, int numWords)
{
        assert(segs != NULL);
        assert(numWords > 0);

        /* Implement as a dynamic array */
        segs->num_segments += 1;
        if (segs->num_segments >= segs->mapped_length){
                expand(segs);
        }

        /* 1.) Make a new segment */
        Seg_info newSeg = make_new_segment(numWords);

        /* 2.) Initialize segment values to 0 */
        initialize_to_zeros(newSeg);

        /* 3.) Add segment to mapped ID, checking if there are available
                unmapped IDs */
        /* Reuse available unmapped IDs*/
        if (segs->num_unmapped_IDs != 0){
                uint32_t id = get_next_id(segs);
                (segs->mapped_IDs)[id] = newSeg;
                return id;
        }
        /* Otherwise, use the next ID sequentially */
        else{
                segs->mapped_IDs[segs->num_segments - 1] = newSeg;
                return segs->num_segments - 1;
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
        Seg_info segment = segs->mapped_IDs[id];
        segs->mapped_IDs[id] = NULL;
        free_seg_info(segment);
        
        /* 2.) Add id to recycled ids */
        segs->num_unmapped_IDs += 1;
        unsigned num = segs->num_unmapped_IDs; 
        if (num >= segs->unmapped_length)
        {
                expand_unmapped(segs);
        }
        (segs->unmapped_IDs)[num] = id;

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

        // /* 1.) Remove ID stored at end of sequence */
        uint32_t id = (segs->unmapped_IDs)[segs->num_unmapped_IDs];
        segs->num_unmapped_IDs -= 1;

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
        Seg_info *mapped = segs->mapped_IDs;
        // Seg_info currSeg = Seq_get(mapped, segment_ID);
        Seg_info currSeg = mapped[segment_ID];
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
        Seg_info * mapped = segs->mapped_IDs;
        Seg_info currSeg = mapped[segment_ID];

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
        Seg_info * mapped = segs->mapped_IDs;
        Seg_info currSeg = mapped[segment_ID];

        assert(currSeg != NULL);
        
        /* 2.) Create a new segment */
        Seg_info copySeg = make_new_segment(currSeg->length);

        /* 3.) Copy Array Info */
        for (uint32_t i = 0; i < copySeg->length; i++){
                (copySeg->arr)[i] = (currSeg->arr)[i];
        }
        
        /* Not casting as a seg info */
        Seg_info zeroSeg = mapped[0];
        free_seg_info(zeroSeg);    
        mapped[0] = copySeg;

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

        Seg_info * mapped = segs->mapped_IDs;
        Seg_info currSeg = mapped[segment_ID];

        assert(currSeg != NULL);
        return currSeg->length;
}

uint32_t *get_seg_zero(Segments_T segs)
{
        return ((segs->mapped_IDs)[0])->arr;
}

void expand(Segments_T segs)
{
        uint64_t size = EXPAND_CONST * segs->mapped_length;

        /* Allocate new array */
        Seg_info *new_arr = malloc(sizeof(struct Seg_info) * size);
        assert(new_arr != NULL);
        
        /* Initialize  */
        for (unsigned i = 0; i < size; i++){
                new_arr[i] = NULL;
        }
        /* Copy Data Over */
        for (unsigned i = 0 ; i < segs->num_segments; i++){
                new_arr[i] = (segs->mapped_IDs)[i];
        }
        
        /* Free old array */
        free(segs->mapped_IDs);
        

        /* Update pointer and values */
        segs->mapped_length *= EXPAND_CONST;
        segs->mapped_IDs = new_arr;

}

void expand_unmapped(Segments_T segs)
{
        uint64_t size = EXPAND_CONST * segs->unmapped_length;

        /* Allocate new array */
        uint32_t *new_arr = malloc(sizeof(uint32_t) * size);
        assert(new_arr != NULL);

        /* Copy Data Over */
        for (unsigned i = 0; i < segs->unmapped_length; i++){
                new_arr[i] = (segs->unmapped_IDs)[i];
        }
        free(segs->unmapped_IDs);


        segs->unmapped_length *= EXPAND_CONST;
        segs->unmapped_IDs = new_arr;

}

#if 0

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
#include <sys/stat.h>

/* --- COMMAND PARSE CONSTANTS --- */
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


/* --- SEQUENCE CONSTANTS --- */
#define SEQ_HINT 10000
#define EXPAND_CONST 2

typedef struct Seg_info {
        uint32_t *arr;
        uint32_t length;
} *Seg_info;



// void parse_command(uint32_t word, uint32_t *arr);


/* Segment Functions */
static inline void initialize_to_zeros(Seg_info seg_info);
static inline uint32_t new_word(FILE *input);
static inline Seg_info make_new_segment(int numWords);
static inline void free_seg_info(Seg_info segment);


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

/* -------------------- INITIALIZATION ------------------------------- */
        /* Make the segments, registers, and command info array */
        /* 1.) Segment Fields */
        // Segments_T  segs = segments_new(fp, name);
        /*      Segments New */
        uint32_t *unmapped_IDs = malloc(sizeof(uint32_t) * SEQ_HINT); 
        Seg_info *mapped_IDs = malloc(sizeof(struct Seg_info) * SEQ_HINT);
        assert(unmapped_IDs != NULL);
        assert(mapped_IDs != NULL);
        unsigned mapped_length = SEQ_HINT;
        unsigned num_segments = 0;
        unsigned num_unmapped_IDs = 0;
        unsigned unmapped_length = SEQ_HINT;
        for (unsigned i = 0; i < SEQ_HINT; i++) mapped_IDs[i] = NULL;
        
/* ----  Read File data ---- */
        struct stat sb;
        stat(name, &sb);
        int size = sb.st_size;
        /* CHECK IF WE NEED THIS!!! */
        assert(size % 4 == 0);
        int numWords = size / 4;
    
/* ------- Map New (specifically 0 segment) ------- */
        /* Implement as a dynamic array */
        num_segments += 1;
        /* 1.) Make a new segment */
        Seg_info newSeg = make_new_segment(numWords);
        /* 2.) Initialize segment values to 0 */
        initialize_to_zeros(newSeg);
        mapped_IDs[num_segments - 1] = newSeg;

/* ------- Map New ------- */
        for (int i = 0; i < numWords; i++){
                uint32_t word = new_word(fp);
/* ------- Store Word ------- */
                // store_word(segs, 0, i, word);
                /* 1.) Get Mapped Segment Sequence and array it stores */
                Seg_info currSeg = mapped_IDs[0];

                assert(currSeg != NULL);
                uint32_t *currSegArr = currSeg->arr;
                uint32_t length = currSeg->length;

                /* 2.) Add word into offset */
                assert(i < (int)length);
                currSegArr[i] = word;
/* ------- Store Word ------- */

                /* ------- Map New ------- */
        }
/* ----  Read File data ---- */


        /* 2.) Register Fields */
        uint32_t registers[8] = {0,0,0,0,0,0,0,0};

        /* 3.) Command Info Fields */
        uint32_t command_info[5] = {0,0,0,0,0};

        /* 4.) Other Initialization */
        uint32_t pc = 0;
        uint32_t command = 0;
        bool keep_going = true;
        
        uint32_t *commands_arr = (mapped_IDs[0])->arr;

/* -------------------- INITIALIZATION ------------------------------- */
        /* -------------------- EXECUTION ------------------------------- */
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
                        val_B, val_C, seg_num, zero, val, full_word, length;

                Seg_info currSeg;
                uint32_t *currSegArr;
                

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
                        // reg_a_val = get_word(segs, id, offset);
                /* Get Word */
                        /* 1.) Get Mapped Segment Sequence and array it stores */
                        currSeg = mapped_IDs[id];
                        assert(currSeg != NULL);
                        currSegArr = currSeg->arr;
                        length = currSeg->length;
                        assert(offset < length);
                        reg_a_val = currSegArr[offset];
                /* Get Word */

                        registers[command_info[REG_A]] = reg_a_val;
                        break;
                case SSTORE :
                        id = registers[command_info[REG_A]];
                        offset = registers[command_info[REG_B]];
                        reg_c_val = registers[command_info[REG_C]];

                        // store_word(segs, id, offset, reg_c_val);
                        /* Store Word */
                        currSeg = mapped_IDs[id];
                        assert(currSeg != NULL);
                        currSegArr = currSeg->arr;
                        length     = currSeg->length;
                        
                        /* 2.) Add word into offset */
                        assert(offset < length);
                        currSegArr[offset] = reg_c_val;

                        /* Store Word */
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
                        // seg_num = map_new(segs, val_C);
                        
                /* Map New */
                       
                        /* Implement as a dynamic array */
                        num_segments += 1;
                        if (num_segments >= mapped_length)
                        {
                                /* ------Expand------ */
                                uint64_t size = EXPAND_CONST * mapped_length;

                                /* Allocate new array */
                                Seg_info *new_arr = malloc(sizeof(struct Seg_info) * size);
                                assert(new_arr != NULL);

                                /* Initialize  */
                                for (unsigned i = 0; i < size; i++) new_arr[i] = NULL;
                                
                                /* Copy Data Over */
                                for (unsigned i = 0; i < num_segments; i++) new_arr[i] = (mapped_IDs)[i];

                                /* Free old array */
                                free(mapped_IDs);

                                /* Update pointer and values */
                                mapped_length *= EXPAND_CONST;
                                mapped_IDs = new_arr;

                                /* ------Expand------ */

                        }

                        /* 1.) Make a new segment */
                        Seg_info newSeg = make_new_segment(numWords);

                        /* 2.) Initialize segment values to 0 */
                        initialize_to_zeros(newSeg);

                        /* 3.) Add segment to mapped ID, checking if there are available
                                unmapped IDs */
                        /* Reuse available unmapped IDs*/
                        if (num_unmapped_IDs != 0)
                        {
                                /* --- Get Next ID --- */
                                // uint32_t id = get_next_id(segs);
                                uint32_t id = unmapped_IDs[num_unmapped_IDs];
                                num_unmapped_IDs -= 1;

                                /* --- Get Next ID --- */

                                mapped_IDs[id] = newSeg;
                                seg_num = id;
                        }
                        /* Otherwise, use the next ID sequentially */
                        else
                        {
                                mapped_IDs[num_segments - 1] = newSeg;
                                seg_num = num_segments - 1;
                        }
                /* Map New */

                        registers[command_info[REG_B]] = seg_num;
                        break;
                case INACTIVATE :
                        val_C = registers[command_info[REG_C]];

                        id = val_C;
                        // unmap_segment(segs, val_C);
                        /* ---- Unmap Segment ---- */
                        /* 1.) Update array which stores segment addresses */
                        Seg_info segment = mapped_IDs[id];
                        mapped_IDs[id] = NULL;
                        free_seg_info(segment);

                        /* 2.) Add id to recycled ids */
                        num_unmapped_IDs += 1;
                        unsigned num = num_unmapped_IDs;
                        if (num >= unmapped_length)
                        {
                                // expand_unmapped(segs);
                                /* --- Expand Unmapped ---- */
                                uint64_t size = EXPAND_CONST * unmapped_length;

                                /* Allocate new array */
                                uint32_t *new_arr = malloc(sizeof(uint32_t) * size);
                                assert(new_arr != NULL);

                                /* Copy Data Over */
                                for (unsigned i = 0; i < unmapped_length; i++)
                                {
                                        new_arr[i] = (unmapped_IDs)[i];
                                }
                                free(unmapped_IDs);

                                unmapped_length *= EXPAND_CONST;
                                unmapped_IDs = new_arr;

                                /* --- Expand Unmapped ---- */
                        }
                        unmapped_IDs[num] = id;

                        /* ---- Unmap Segment ---- */
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
                                /* --- Duplicate Seg ---- */
                                // duplicate_seg(segs, reg_b_val);

                                /* 1.) Get Segment to Copy */
                                Seg_info currSeg = mapped_IDs[reg_b_val];
                                assert(currSeg != NULL);

                                /* 2.) Create a new segment */
                                Seg_info copySeg = make_new_segment(currSeg->length);

                                /* 3.) Copy Array Info */
                                for (uint32_t i = 0; i < copySeg->length; i++)
                                {
                                        (copySeg->arr)[i] = (currSeg->arr)[i];
                                }

                                /* Not casting as a seg info */
                                Seg_info zeroSeg = mapped_IDs[0];
                                free_seg_info(zeroSeg);
                                mapped_IDs[0] = copySeg;

                                /* --- Duplicate Seg ---- */

                                
                                commands_arr = (mapped_IDs[0])->arr;
                        }
                        pc = reg_c_val;
                        break;
                case LV :
                        registers[command_info[REG_A]] = command_info[VAL];
                        break;
                }
        }
        /* -------------------- EXECUTION ------------------------------- */


        /* Free data */
        /* Free segment data */
        for(unsigned i = 0; i < num_segments; i++){
                Seg_info segment = mapped_IDs[i];
                if (segment != NULL){
                        free_seg_info(segment);
                }
        }

        free(mapped_IDs);

        free(unmapped_IDs);
}



static inline Seg_info make_new_segment(int numWords)
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

static inline void initialize_to_zeros(Seg_info seg_info)
{
        assert(seg_info != NULL);

        uint32_t zero = 0;
        uint32_t length = seg_info->length;

        for (uint32_t i = 0; i < length; i++)
        {
                seg_info->arr[i] = zero;
        }

        return;
}


static inline uint32_t new_word(FILE *input)
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



static inline void free_seg_info(Seg_info segment)
{
        assert(segment != NULL);
        free(segment->arr);
        free(segment);

        return;
}
#endif
