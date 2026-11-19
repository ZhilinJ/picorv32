/*
 * Adapted from PicoRV32's dhrystone/dhry_1.c and dhry_2.c.
 * Dhrystone 2.1, Reinhold P. Weicker, May 25, 1988.
 * The procedure bodies and benchmark loop follow the repository sources.
 * This BRAM demo uses static records, short output and automatic result checks.
 * It is intended for functional validation, not standardized DMIPS reporting.
 */
#include <stdint.h>
#include <string.h>
#include "console.h"
#define TIME
#define USE_MYSTDLIB
#include "../../dhrystone/dhry.h"
#define REG
#define NUMBER_OF_RUNS 100

Rec_Pointer Ptr_Glob, Next_Ptr_Glob;
int Int_Glob;
Boolean Bool_Glob;
char Ch_1_Glob, Ch_2_Glob;
int Arr_1_Glob[50];
int Arr_2_Glob[50][50];
static Rec_Type records[2];

static void __attribute__((noinline)) Proc_1(Rec_Pointer Ptr_Val_Par);
static void __attribute__((noinline)) Proc_2(One_Fifty *Int_Par_Ref);
static void __attribute__((noinline)) Proc_3(Rec_Pointer *Ptr_Ref_Par);
static void __attribute__((noinline)) Proc_4(void);
static void __attribute__((noinline)) Proc_5(void);
static void __attribute__((noinline)) Proc_6(Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par);
static void __attribute__((noinline)) Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty *Int_Par_Ref);
static void __attribute__((noinline)) Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val);
static Enumeration __attribute__((noinline)) Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val);
static Boolean __attribute__((noinline)) Func_2(Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref);
static Boolean __attribute__((noinline)) Func_3(Enumeration Enum_Par_Val);

static void Proc_1(Rec_Pointer Ptr_Val_Par)
{
  REG Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;

  structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
  Ptr_Val_Par->variant.var_1.Int_Comp = 5;
  Next_Record->variant.var_1.Int_Comp
        = Ptr_Val_Par->variant.var_1.Int_Comp;
  Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
  Proc_3 (&Next_Record->Ptr_Comp);

  if (Next_Record->Discr == Ident_1)

  {
    Next_Record->variant.var_1.Int_Comp = 6;
    Proc_6 (Ptr_Val_Par->variant.var_1.Enum_Comp,
           &Next_Record->variant.var_1.Enum_Comp);
    Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
    Proc_7 (Next_Record->variant.var_1.Int_Comp, 10,
           &Next_Record->variant.var_1.Int_Comp);
  }
  else
    structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
}

static void Proc_2(One_Fifty *Int_Par_Ref)
{
  One_Fifty  Int_Loc;
  Enumeration   Enum_Loc;

  Int_Loc = *Int_Par_Ref + 10;
  do
    if (Ch_1_Glob == 'A')

    {
      Int_Loc -= 1;
      *Int_Par_Ref = Int_Loc - Int_Glob;
      Enum_Loc = Ident_1;
    }
  while (Enum_Loc != Ident_1);
}

static void Proc_3(Rec_Pointer *Ptr_Ref_Par)
{
  if (Ptr_Glob != Null)

    *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
  Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
}

static void Proc_4(void)
{
  Boolean Bool_Loc;

  Bool_Loc = Ch_1_Glob == 'A';
  Bool_Glob = Bool_Loc | Bool_Glob;
  Ch_2_Glob = 'B';
}

static void Proc_5(void)
{
  Ch_1_Glob = 'A';
  Bool_Glob = false;
}

static void Proc_6(Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par)
{
  *Enum_Ref_Par = Enum_Val_Par;
  if (! Func_3 (Enum_Val_Par))

    *Enum_Ref_Par = Ident_4;
  switch (Enum_Val_Par)
  {
    case Ident_1:
      *Enum_Ref_Par = Ident_1;
      break;
    case Ident_2:
      if (Int_Glob > 100)

      *Enum_Ref_Par = Ident_1;
      else *Enum_Ref_Par = Ident_4;
      break;
    case Ident_3:
      *Enum_Ref_Par = Ident_2;
      break;
    case Ident_4: break;
    case Ident_5:
      *Enum_Ref_Par = Ident_3;
      break;
  }
}

static void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty *Int_Par_Ref)
{
  One_Fifty Int_Loc;

  Int_Loc = Int_1_Par_Val + 2;
  *Int_Par_Ref = Int_2_Par_Val + Int_Loc;
}

static void Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val)
{
  REG One_Fifty Int_Index;
  REG One_Fifty Int_Loc;

  Int_Loc = Int_1_Par_Val + 5;
  Arr_1_Par_Ref [Int_Loc] = Int_2_Par_Val;
  Arr_1_Par_Ref [Int_Loc+1] = Arr_1_Par_Ref [Int_Loc];
  Arr_1_Par_Ref [Int_Loc+30] = Int_Loc;
  for (Int_Index = Int_Loc; Int_Index <= Int_Loc+1; ++Int_Index)
    Arr_2_Par_Ref [Int_Loc] [Int_Index] = Int_Loc;
  Arr_2_Par_Ref [Int_Loc] [Int_Loc-1] += 1;
  Arr_2_Par_Ref [Int_Loc+20] [Int_Loc] = Arr_1_Par_Ref [Int_Loc];
  Int_Glob = 5;
}

static Enumeration Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val)
{
  Capital_Letter        Ch_1_Loc;
  Capital_Letter        Ch_2_Loc;

  Ch_1_Loc = Ch_1_Par_Val;
  Ch_2_Loc = Ch_1_Loc;
  if (Ch_2_Loc != Ch_2_Par_Val)

    return (Ident_1);
  else
  {
    Ch_1_Glob = Ch_1_Loc;
    return (Ident_2);
   }
}

static Boolean Func_2(Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref)
{
  REG One_Thirty        Int_Loc;
      Capital_Letter    Ch_Loc;

  Int_Loc = 2;
  while (Int_Loc <= 2)
    if (Func_1 (Str_1_Par_Ref[Int_Loc],
                Str_2_Par_Ref[Int_Loc+1]) == Ident_1)

    {
      Ch_Loc = 'A';
      Int_Loc += 1;
    }
  if (Ch_Loc >= 'W' && Ch_Loc < 'Z')

    Int_Loc = 7;
  if (Ch_Loc == 'R')

    return (true);
  else
  {
    if (strcmp (Str_1_Par_Ref, Str_2_Par_Ref) > 0)

    {
      Int_Loc += 7;
      Int_Glob = Int_Loc;
      return (true);
    }
    else
      return (false);
  }
}

static Boolean Func_3(Enumeration Enum_Par_Val)
{
  Enumeration Enum_Loc;

  Enum_Loc = Enum_Par_Val;
  if (Enum_Loc == Ident_3)

    return (true);
  else
    return (false);
}



/* Small string routines keep the demo independent of a hosted C library. */
void *memcpy(void *destination, const void *source, size_t length)
{
    unsigned char *output = destination;
    const unsigned char *input = source;
    for (size_t index = 0; index < length; ++index)
        output[index] = input[index];
    return destination;
}

char *strcpy(char *destination, const char *source)
{
    char *start = destination;
    do {
        *destination++ = *source;
    } while (*source++);
    return start;
}

int strcmp(const char *left, const char *right)
{
    while (*left && *left == *right) {
        ++left;
        ++right;
    }
    return (unsigned char)*left - (unsigned char)*right;
}

static void print_dec(uint32_t value)
{
    char text[11];
    char *position = text + sizeof(text) - 1;
    *position = '\0';
    do {
        *--position = '0' + value % 10;
        value /= 10;
    } while (value);
    console_puts(position);
}

int main(void)
{
    One_Fifty Int_1_Loc = 0, Int_2_Loc = 0, Int_3_Loc = 0;
    Capital_Letter Ch_Index;
    Enumeration Enum_Loc = Ident_1;
    Str_30 Str_1_Loc, Str_2_Loc;
    int Run_Index;
    const int Number_Of_Runs = NUMBER_OF_RUNS;
    uint32_t cycles_begin, cycles_end, instructions_begin, instructions_end;

    Ptr_Glob = &records[0];
    Next_Ptr_Glob = &records[1];
    Ptr_Glob->Ptr_Comp = Next_Ptr_Glob;
    Ptr_Glob->Discr = Ident_1;
    Ptr_Glob->variant.var_1.Enum_Comp = Ident_3;
    Ptr_Glob->variant.var_1.Int_Comp = 40;
    strcpy(Ptr_Glob->variant.var_1.Str_Comp, "DHRYSTONE PROGRAM, SOME STRING");
    strcpy(Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");
    Arr_2_Glob[8][7] = 10;

    console_puts("Dhrystone 2.1 BRAM demo: 100 runs\n");
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_begin) : : "memory");
    __asm__ volatile ("rdinstret %0" : "=r"(instructions_begin) : : "memory");
  for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index)
  {

    Proc_5();
    Proc_4();
      /* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
    Int_1_Loc = 2;
    Int_2_Loc = 3;
    strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
    Enum_Loc = Ident_2;
    Bool_Glob = ! Func_2 (Str_1_Loc, Str_2_Loc);
      /* Bool_Glob == 1 */
    while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */
    {
      Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
        /* Int_3_Loc == 7 */
      Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
        /* Int_3_Loc == 7 */
      Int_1_Loc += 1;
    } /* while */
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Proc_8 (Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
      /* Int_Glob == 5 */
    Proc_1 (Ptr_Glob);
    for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index)
                             /* loop body executed twice */
    {
      if (Enum_Loc == Func_1 (Ch_Index, 'C'))
          /* then, not executed */
        {
        Proc_6 (Ident_1, &Enum_Loc);
        strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
        Int_2_Loc = Run_Index;
        Int_Glob = Run_Index;
        }
    }
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Int_2_Loc = Int_2_Loc * Int_1_Loc;
    Int_1_Loc = Int_2_Loc / Int_3_Loc;
    Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
      /* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
    Proc_2 (&Int_1_Loc);
      /* Int_1_Loc == 5 */

  } /* loop "for Run_Index" */


    __asm__ volatile ("rdcycle %0" : "=r"(cycles_end) : : "memory");
    __asm__ volatile ("rdinstret %0" : "=r"(instructions_end) : : "memory");

    int passed =
        Int_Glob == 5 && Bool_Glob == 1 &&
        Ch_1_Glob == 'A' && Ch_2_Glob == 'B' &&
        Arr_1_Glob[8] == 7 && Arr_2_Glob[8][7] == Number_Of_Runs + 10 &&
        Ptr_Glob->Ptr_Comp == Next_Ptr_Glob &&
        Ptr_Glob->Discr == Ident_1 &&
        Ptr_Glob->variant.var_1.Enum_Comp == Ident_3 &&
        Ptr_Glob->variant.var_1.Int_Comp == 17 &&
        strcmp(Ptr_Glob->variant.var_1.Str_Comp, "DHRYSTONE PROGRAM, SOME STRING") == 0 &&
        Next_Ptr_Glob->Ptr_Comp == Next_Ptr_Glob &&
        Next_Ptr_Glob->Discr == Ident_1 &&
        Next_Ptr_Glob->variant.var_1.Enum_Comp == Ident_2 &&
        Next_Ptr_Glob->variant.var_1.Int_Comp == 18 &&
        strcmp(Next_Ptr_Glob->variant.var_1.Str_Comp, "DHRYSTONE PROGRAM, SOME STRING") == 0 &&
        Int_1_Loc == 5 && Int_2_Loc == 13 && Int_3_Loc == 7 &&
        Enum_Loc == Ident_2 &&
        strcmp(Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING") == 0 &&
        strcmp(Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING") == 0;

    uint32_t cycles = cycles_end - cycles_begin;
    uint32_t instructions = instructions_end - instructions_begin;
    console_puts("Cycles: ");
    print_dec(cycles);
    console_puts("\nInstructions: ");
    print_dec(instructions);
    console_puts("\nArray check: Arr_2_Glob[8][7] = ");
    print_dec(Arr_2_Glob[8][7]);
    console_puts("\n");
    if (!passed || instructions == 0) {
        console_puts("DHRYSTONE FAIL\n");
        return 1;
    }
    console_puts("DHRYSTONE PASS\n");
    return 0;
}
