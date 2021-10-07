// Xception: generates an Intel hex file representing the exception
// EPROM for a CTS256A text to allophone IC.
//
// The format of the input text file is as follows:
//
// BASE n     <-- The 4K page where the EPROM will be generated.
//                Allowed values: 1..9,A-E
// <[word1]<=[allophone list] ; comment   <-- See the CTS256A datasheet for 
// <[word2]<=[allophone list] ; comment   <-- word format and allophones
//    .                                   <-- words should be in alphabetical
//    .                                   <-- order
//    .
// <[wordN]<=[allophone list] ; comment

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// SP0256AL2 allophone names, in numerical order
// starting at 0
const char *allophones[] = {
   "PA1", "PA2", "PA3", "PA4", "PA5", "OY",  "AY",  "EH",
   "KK3", "PP",  "JH",  "NN1", "IH",  "TT2", "RR1", "AX",
   "MM",  "TT1", "DH1", "IY",  "EY",  "DD1", "UW1", "AO",
   "AA",  "YY2", "AE",  "HH1", "BB1", "TH",  "UH",  "UW2",
   "AW",  "DD2", "GG3", "VV",  "GG1", "SH",  "ZH",  "RR2",
   "FF",  "KK2", "KK1", "ZZ",  "NG",  "LL",  "WW",  "XR",
   "WH",  "YY1", "CH",  "ER1", "ER2", "OW",  "DH2", "SS",
   "NN2", "HH2", "OR",  "AR",  "YR",  "GG2", "EL",  "BB2"
};
#define NUM_ALLOPHONES (sizeof allophones/sizeof(const char *))

#define WORD_DELIMITER_CHAR      '<'
#define WORD_DELIMITER_ENCODED   0x13
#define START_DEFN_CHAR          '['
#define START_DEFN_ENCODED       0x40
#define END_DEFN_CHAR            ']'
#define END_DEFN_ENCODED         0x80
#define SEPARATOR_CHAR           '='

// Exception-word EPROM header
// contains the 5 magic bytes that identify the EPROM
// as well as the new parameters data
// and the routine to setup the new parameters
uint8_t hdr[] = {
   0x80,0x48,0x28,0x58,0x85,0xE0,0x35,0xE0,
   0x31,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
   0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
   0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
   0xFF,0xFF,0xFF,0x1E,0x1F,0x20,0x21,0x28,
   0x29,0x24,0x25,0x22,0x23,0x2A,0x2B,0x26,
   0x27,0x2C,0x2D,0x2E,0x2F,0x32,0x33,0x34,
   0x35,0x36,0xE0,0x65,0x78,0x02,0x31,0x8E,
   0xF1,0x43,0xC5,0xAA,0x00,0x09,0x2D,0xFF,
   0xE2,0x1E,0xB8,0xAA,0x00,0x23,0xD5,0x12,
   0xD0,0x13,0xB9,0x9B,0x13,0xC3,0xAA,0x00,
   0x09,0x2D,0xFF,0xE2,0x0B,0xB8,0xAA,0x00,
   0x23,0xD5,0x12,0xD0,0x13,0xB9,0x9B,0x13,
   0x5D,0x16,0xE6,0xE9,0xC3,0xAA,0x00,0x09,
   0x2D,0xFF,0xE2,0x14,0xA2,0x40,0x11,0x82,
   0x11,0xA2,0x15,0x11,0xC3,0xAA,0x00,0x09,
   0x82,0x15,0xC3,0xAA,0x00,0x09,0x82,0x14,
   0x98,0x29,0x03,0x98,0x2B,0x07,0x22,0x20,
   0x9B,0x03,0x8E,0xF7,0x2B,0x98,0x03,0x05,
   0x98,0x07,0x09,0x98,0x03,0x19,0x8C,0xF1,
   0x00,0xE0,0x36
};
#define HDR_LENGTH (sizeof hdr/sizeof(uint8_t))
#define HDR_OFFSET 0
// the following bytes within the header array need
// to have the base 4K page value added to them
uint16_t hdrRelocs[] = {
   0x0044,0x004C,0x0057,0x005F,0x006E,0x007E,0x0084
};
#define HDR_RELOCS_NUM (sizeof hdrRelocs/sizeof(uint16_t))

// following the hdr array is 27 words for the start
// address of each letter list and the symbol list
#define LETTER_INDEX_OFFSET (HDR_OFFSET+HDR_LENGTH)
#define LETTER_INDEX_LENGTH (('Z'+1-'A'+1)*sizeof(uint16_t))

// this is the routine that the CTS256 uses to search the
// exception-word EPROM
#define EXC_WORDS_RTN_OFFSET (HDR_OFFSET+LETTER_INDEX_OFFSET+LETTER_INDEX_LENGTH)
uint8_t excWordsRtn[] = {
         0xD8,0x02,0xD8,0x03,0x98,0x03,0x11,
    0x8E,0xF7,0x4B,0x8E,0xF7,0x0F,0x77,0x01,
    0x0A,0x05,0x74,0x80,0x0B,0xE0,0x03,0x73,
    0x7F,0x0B,0x8E,0xF3,0xAF,0x76,0x20,0x0A,
    0x0E,0x52,0x34,0xAA,0x00,0xA3,0xD0,0x14,
    0xAA,0x00,0xA4,0xD0,0x15,0xE0,0x0F,0xC5,
    0x2A,0x41,0x2C,0x02,0xAA,0x00,0xA3,0xD0,
    0x14,0xAA,0x00,0xA4,0xD0,0x15,0x52,0x01,
    0x8E,0xF4,0x88,0x8E,0xF4,0xC2,0x76,0x10,
    0x0A,0x4D,0x2D,0xFF,0xE2,0x60,0x98,0x11,
    0x1D,0x73,0xBF,0x0A,0x8E,0xF5,0x64,0x76,
    0x10,0x0A,0x3C,0x8E,0xF4,0x7E,0x74,0x40,
    0x0A,0x8E,0xF5,0x64,0x76,0x10,0x0A,0x42,
    0x48,0x37,0x34,0x79,0x00,0x33,0xD5,0x37,
    0x73,0xFD,0x0B,0x52,0x02,0x8E,0xF4,0x88,
    0x8E,0xF4,0x9E,0x98,0x0F,0x03,0x98,0x03,
    0x11,0x8E,0xF7,0x4B,0x77,0x80,0x0B,0x0A,
    0xDB,0x39,0x8E,0xF3,0x47,0xC9,0xC9,0x8C,
    0xF1,0x36,0xC9,0xC9,0x8C,0xF3,0xF4,0xD3,
    0x15,0xE7,0x02,0xD3,0x14,0x52,0x02,0x8E,
    0xF4,0x88,0x72,0x01,0x37,0x73,0xFD,0x0B,
    0xE0,0x99,0x52,0x03,0xE0,0xF1,0xD9,0x03,
    0xD9,0x02,0xD5,0x37,0x73,0xFD,0x0B,0x8C,
    0xF3,0xEE,0xFF
};
// these addresses within the excWordsRtn
// need to be offset by the base 4K page value
uint16_t excWordsRelocs[] = {
   0x00FC,0x0101,0x010D,0x0112
};
#define EXC_WORDS_RELOCS_NUM (sizeof excWordsRelocs/sizeof(uint16_t))
#define EXC_WORDS_RTN_LENGTH (sizeof excWordsRtn/sizeof(uint8_t))
#define WORDS_OFFSET (HDR_LENGTH+LETTER_INDEX_LENGTH+EXC_WORDS_RTN_LENGTH)

// assume a 4K EPROM size (2732 type)
#define EPROM_LENGTH  4096
uint8_t bin[EPROM_LENGTH];

uint16_t base;       // 4K page base address of EPROM (x000 where x is 1..E)

typedef enum {       // scan states
   WORD_START,       // looking for initial < or [
   START_DEFN,       // looking for initial [
   IGNORE_TO_EOL,    // found a ; (comment)
   IN_WORD,          // accumulate all letters in word
   END_DEFN,         // found trailing ]
   SEPARATOR,        // looking for = separator
   START_ALLOS_DEFN, // looking for leading [ for allophone list
   IN_ALLOPHONES     // processing allophones
} ParseStates;

ParseStates parseState = WORD_START;

uint16_t lineNumber = 1;

// convert byte array to Intel hex format file
// bin:  pointer to source byte array
// base: start address
// lng:  # bytes in source array
static void hexout(uint8_t *bin, uint16_t base, uint16_t lng) {

   uint8_t checksum = 0, val;

   for( uint16_t addr = base; addr != base + lng; ++addr ) {
      if( (addr & 0x0F) == 0x00 ) {
         printf(":10%04X00", addr);
         checksum = 0x10 + ((addr) >> 8) + ((addr) & 0xFF);
      }
      val = bin[addr-base];
      checksum += val;
      printf("%02X", val);
      if( (addr & 0x0F) == 0x0F ) {
         checksum = ~checksum;
         ++checksum;
         printf("%02X\n", checksum);
      }
   }
   printf(":00000001FF\n");
}

// Convert from ASCII-7 printable to encoded value
// (basically shift 20-7E to 00-5E)
static uint8_t encode(uint8_t c) {
   return c - 0x20;
}

static void error(const char *str) {
   fprintf(stderr, "Error in line %u:\n%s\n", lineNumber, str);
   exit(EXIT_FAILURE);
}

static void errors(const char *fmt, char* str) {
   char buf[80];
   snprintf(buf, sizeof buf, fmt, str);
   fprintf(stderr, "Error in line %u:\n%s\n", lineNumber, buf);
   exit(EXIT_FAILURE);
}

static void errorc(const char *fmt, char c) {
   char buf[80];
   snprintf(buf, sizeof buf, fmt, c);
   fprintf(stderr, "Error in line %u:\n%s\n", lineNumber, buf);
   exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

   uint16_t letterIndexOffset = LETTER_INDEX_OFFSET;
   uint16_t wordsOffset = WORDS_OFFSET;
   char baseStr[3];
   int ok = 0, startFlag = 0;
   char word[20], suffix[20],allophone[4];
   uint16_t wordLength, suffixLength, allophoneLength, numAllophones;
   int c;
   char currentLetter = '@';

   // this section needs to be revisited at the first
   // opportunity and replaced with something more robust
   ok = scanf("BASE %2s",baseStr) == 1 && strlen(baseStr) == 1;
   if( ok ) {
      // valid values for base are 1..9,A..E
      char baseChar = toupper(baseStr[0]);
      if( baseChar >= '1' && baseChar <= '9' ) {
         base = (baseChar - '0') << 12;
         ok = 1;
      } else if ( baseChar >= 'A' && baseChar <= 'E' ) {
         base = (baseChar - 'A' + 10) << 12;
         ok = 1;
      }
   }
   if( !ok ) {
      error("Error in line 1:\nInvalid BASE declaration.");
   }
   memset(bin, 0xFF, sizeof bin);

   // copy the header in and update the relocatable
   // bytes with values based on the user's base value
   memcpy(bin + HDR_OFFSET, hdr, HDR_LENGTH);
   for( int i = 0; i < HDR_RELOCS_NUM; ++i ) {
      bin[hdrRelocs[i]] += (base >> 8);
   }

   // copy the exception-word routine in and update the relocatable
   // bytes with values based on the user's base value
   memcpy(bin + EXC_WORDS_RTN_OFFSET, excWordsRtn, EXC_WORDS_RTN_LENGTH);
   for( int i = 0; i < EXC_WORDS_RELOCS_NUM; ++i ) {
      bin[excWordsRelocs[i]] += (base >> 8);
   }
   ++lineNumber;

   // discard the remainder of the BASE line
   do {
      c = getchar();
   } while( c != EOF && c != '\n');
   if( c != EOF ) {
      c = getchar();
   }

   while( c != EOF ) {
      switch( parseState ) {
         case WORD_START:
            if( c == WORD_DELIMITER_CHAR ) {    // <
               parseState = START_DEFN;         // look for leading [
               startFlag = 1;                   // 'word' will be a full word or prefix
            } else if( c == START_DEFN_CHAR ) { // [
               parseState = IN_WORD;            // look for A-Z or '
               startFlag = 0;                   // 'word' could be any part of the full word
               wordLength = 0;
               memset(word, '\x00', sizeof word);
               suffixLength = 0;
               memset(suffix, '\x00', sizeof suffix);
            } else if( c == ';' ) {             // comment
               parseState = IGNORE_TO_EOL;
            } else if( !isblank(c) ) {
               errorc(
                  "Found '%c' while looking for start of word ('<') or start of definition ('[')",
                  c);
            }
            break;
         case IGNORE_TO_EOL:                    // comment
            if( c == '\n' ) {
               ++lineNumber;
               parseState = WORD_START;
            }
            break;
         case START_DEFN:
            if( c == START_DEFN_CHAR ) {        // [
               parseState = IN_WORD;
               wordLength = 0;
               memset(word, '\x00', sizeof word);
               suffixLength = 0;
               memset(suffix, '\x00', sizeof suffix);
            } else {
               errorc(
                  "Found '%c' while looking for start of word definition ('[')",
                  c);
            }
            break;
         case IN_WORD:
            if( !wordLength || isalnum(c) || c == '\'' || c == '(' || c == ')' ) {
               if( wordLength >= sizeof word - 1) {
                  errors("Word '%s' is too long", word);
               }
               word[wordLength++] = c;
            } else if( c == END_DEFN_CHAR ) {   // ]
               if( wordLength == 0 ) {
                  error("Empty word not allowed");
               }
               parseState = END_DEFN;
            } else {
               errorc("Unexpected character ('%c') in word",c);
            }
            break;
         case END_DEFN:
            if( isalpha(c) ) {                  // suffix after 'word'
               if( suffixLength >= sizeof suffix - 1) {
                  errors("Suffix '%s' is too long", suffix);
               }
               suffix[suffixLength++] = c;
            } else if( c == WORD_DELIMITER_CHAR || c == SEPARATOR_CHAR ) {
               if( isalpha(word[0]) ) {         // A-Z class
                  while( word[0] > currentLetter ) {
                     // each letter class ends with a byte of FF
                     if( currentLetter != '@' ) {
                        bin[wordsOffset++] = 0xFF;
                     }
                     // update start of next letter class
                     bin[letterIndexOffset++] = (wordsOffset + base) >> 8;
                     bin[letterIndexOffset++] = (wordsOffset + base) & 0xFF;
                     ++currentLetter;
                  }
                  // was there a leading < character to indicate
                  // that this entry is at the start of the word?
                  if( startFlag ) {
                     bin[wordsOffset++] = WORD_DELIMITER_ENCODED;
                  }
                  // single letter word?
                  if( wordLength == 1 ) {
                     bin[wordsOffset++] = 0xFF;
                  } else if( wordLength == 2 ) {// only 1 letter to be encoded?
                     bin[wordsOffset++] = 
                        START_DEFN_ENCODED + 
                        END_DEFN_ENCODED + 
                        encode(word[1]);
                  } else {                      // at least 3 characters
                     bin[wordsOffset++] =       // encode 2nd char in word
                        START_DEFN_ENCODED +    // with [
                        encode(word[1]);
                     for( int i = 2; i < wordLength - 1; ++i ) {
                        bin[wordsOffset++] = encode(word[i]);
                     }
                     bin[wordsOffset++] =       // encode last char in word
                        END_DEFN_ENCODED +      // with ]
                        encode(word[wordLength - 1]);
                  }
                  // any suffix? (ie 'word' is only a prefix
                  for( int i = 0; i < suffixLength; ++i ) {
                     bin[wordsOffset++] = encode(suffix[i]);
                  }
                  // end of word?
                  if( c == WORD_DELIMITER_CHAR ) {
                     bin[wordsOffset++] = WORD_DELIMITER_ENCODED;
                     parseState = SEPARATOR;
                  } else if ( c == SEPARATOR_CHAR ) {
                     parseState = START_ALLOS_DEFN;
                  } else {
                     errorc(
                        "Found '%c' while looking for "
                        "end of word ('<') or separator ('=')",
                        c);
                  }
               } else { // number or symbol
                  while( 'Z' >= currentLetter ) {  // add FFs for unused letters
                     if( currentLetter != '@' ) {  // at end of alphabet
                        bin[wordsOffset++] = 0xFF; // and update the letter index
                     }
                     bin[letterIndexOffset++] = (wordsOffset + base) >> 8;
                     bin[letterIndexOffset++] = (wordsOffset + base) & 0xFF;
                     ++currentLetter;
                  }
                  // does the symbol have to be at the start of a word?
                  if( startFlag ) {
                     bin[wordsOffset++] = WORD_DELIMITER_ENCODED;
                  }
                  if( wordLength == 1 ) {
                     bin[wordsOffset++] = 
                        START_DEFN_ENCODED + 
                        END_DEFN_ENCODED + 
                        encode(word[0]);
                  } else {
                     error("Symbol 'words' can only be 1 character");
                  }
                  if( c == WORD_DELIMITER_CHAR ) {
                     bin[wordsOffset++] = WORD_DELIMITER_ENCODED;
                     parseState = SEPARATOR;
                  } else if ( c == SEPARATOR_CHAR ) {
                     parseState = START_ALLOS_DEFN;
                  } else {
                     errorc(
                        "Found '%c' while looking for "
                        "end of symbol ('<)' or separator ('='))",
                        c);
                  }
               }
            } else {
               errorc(
                  "Found '%c' while looking for "
                  "end of word definition (']') or separator ('=')",
                  c);
            }
            break;
         case SEPARATOR:
            if( c == SEPARATOR_CHAR ) {
               parseState = START_ALLOS_DEFN;
            } else if( !isblank(c) ) {
               errorc(
                  "Found '%c' while looking for "
                  "separator ('=')",
                  c);
            }
            break;
         case START_ALLOS_DEFN:
            if( c == START_DEFN_CHAR ) {
               parseState = IN_ALLOPHONES;
               allophoneLength = 0;
               numAllophones = 0;
               memset(allophone, '\x00', sizeof allophone);
            } else if( !isblank(c) ) {
               errorc(
                  "Found '%c' while looking for "
                  "end of allophones definition (']')",
                  c);
            }
            break;
         case IN_ALLOPHONES:
            if( isalpha(c) || isdigit(c) ) {
               if( allophoneLength >= sizeof allophone - 1) {
                  errors("Allophone '%s' is too long", allophone);
               }
               allophone[allophoneLength++] = c;
            } else if( isblank(c) || c == END_DEFN_CHAR ) {
               if( allophoneLength ) {
                  // search list of valid allophone names
                  uint8_t i;
                  for( i=0; i < NUM_ALLOPHONES; ++i ) {
                     if( !strcmp(allophones[i], allophone) ) {
                        // found it: add it to output list
                        // and encode with [ if 1st allophone
                        // for this word
                        bin[wordsOffset] = i;
                        if( !numAllophones ) {
                           bin[wordsOffset] += START_DEFN_ENCODED;
                        }
                        ++wordsOffset;
                        ++numAllophones;
                        break;
                     }
                  }
                  if( i == NUM_ALLOPHONES ) {
                     errors("Unknown allophone '%s'",allophone);
                  }
                  // get ready for next allophone
                  allophoneLength = 0;
                  memset(allophone, '\x00', sizeof allophone);
               }
               // last one?
               if( c == END_DEFN_CHAR ) {
                  bin[wordsOffset - 1] += END_DEFN_ENCODED;
                  parseState = IGNORE_TO_EOL;
               }
            } else {
               errorc("Unexpected character ('%c') in allophone list",c);
            }
            break;
      }
      c = getchar();
   }
   // fill in remainder of letter offset list if no
   // symbols were defined
   while( 'Z' >= currentLetter ) {
      if( currentLetter != '@' ) {
         bin[wordsOffset++] = 0xFF;
      }
      bin[letterIndexOffset++] = (wordsOffset + base) >> 8;
      bin[letterIndexOffset++] = (wordsOffset + base) & 0xFF;
      ++currentLetter;
   }
   // output result as Intel hex file starting at address 0
   hexout(bin, 0x0000, EPROM_LENGTH);

   return EXIT_SUCCESS;
}
