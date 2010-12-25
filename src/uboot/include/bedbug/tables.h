/* $Id: tables.h,v 1.1.1.1 2004/10/27 17:21:34 sturgesa Exp $ */

#ifndef TABLES_H
#define TABLES_H

/* This is only included by common/bedbug.c, and depends on the following
 * files to already be included
 *   common.h
 *   bedbug/bedbug.h
 *   bedbug/ppc.h
 *   bedbug/regs.h
 */

struct operand operands[] = {
  /*Field    Name     Bits  Shift  Hint 		   Position	*/
  /*-----    ------   ----- -----  ---- 	           ------------ */
  { O_AA,    "O_AA",    1,     1,  OH_SILENT },		/*   30 	*/
  { O_BD,    "O_BD",   14,     2,  OH_ADDR },		/* 16-29	*/
  { O_BI,    "O_BI",    5,    16,  0 },			/* 11-15	*/
  { O_BO,    "O_BO",    5,    21,  0 },			/*  6-10	*/
  { O_crbD,  "O_crbD",  5,    21,  0 },			/*  6-10	*/
  { O_crbA,  "O_crbA",  5,    16,  0 },			/* 11-15	*/
  { O_crbB,  "O_crbB",  5,    11,  0 },			/* 16-20	*/
  { O_CRM,   "O_CRM",   8,    12,  0 },			/* 12-19	*/
  { O_d,     "O_d",    15,     0,  OH_OFFSET },		/* 16-31	*/
  { O_frC,   "O_frC",   5,     6,  0 },			/* 21-25	*/
  { O_frD,   "O_frD",   5,    21,  0 },			/*  6-10	*/
  { O_frS,   "O_frS",   5,    21,  0 },			/*  6-10	*/
  { O_IMM,   "O_IMM",   4,    12,  0 },			/* 16-19	*/
  { O_LI,    "O_LI",   24,     2,  OH_ADDR },		/*  6-29	*/
  { O_LK,    "O_LK",    1,     0,  OH_SILENT },		/*   31		*/
  { O_MB,    "O_MB",    5,     6,  0 },			/* 21-25	*/
  { O_ME,    "O_ME",    5,     1,  0 },			/* 26-30	*/
  { O_NB,    "O_NB",    5,    11,  0 },			/* 16-20	*/
  { O_OE,    "O_OE",    1,    10,  OH_SILENT },		/*   21		*/
  { O_rA,    "O_rA",    5,    16,  OH_REG },		/* 11-15	*/
  { O_rB,    "O_rB",    5,    11,  OH_REG },		/* 16-20	*/
  { O_Rc,    "O_Rc",    1,     0,  OH_SILENT },		/*   31		*/
  { O_rD,    "O_rD",    5,    21,  OH_REG },		/*  6-10	*/
  { O_rS,    "O_rS",    5,    21,  OH_REG },		/*  6-10	*/
  { O_SH,    "O_SH",    5,    11,  0 },			/* 16-20	*/
  { O_SIMM,  "O_SIMM", 16,     0,  0 },			/* 16-31	*/
  { O_SR,    "O_SR",    4,    16,  0 },			/* 12-15	*/
  { O_TO,    "O_TO",    5,    21,  0 },			/*  6-10	*/
  { O_UIMM,  "O_UIMM", 16,     0,  0 },			/* 16-31	*/
  { O_crfD,  "O_crfD",  3,    23,  0 },			/*  6- 8	*/
  { O_crfS,  "O_crfS",  3,    18,  0 },			/* 11-13	*/
  { O_L,     "O_L",     1,    21,  0 },			/*   10		*/
  { O_spr,   "O_spr",  10,    11,  OH_SPR },		/* 11-20	*/
  { O_tbr,   "O_tbr",  10,    11,  OH_TBR },		/* 11-20	*/
  { O_cr2,   "O_cr2",   0,     0,  OH_LITERAL },        /* "cr2"        */
};

const unsigned int n_operands = sizeof(operands) / sizeof(operands[0]);

/* A note about the fields array in the opcodes structure:
   The operands are listed in the order they appear in the output.

   This table is arranged in numeric order of the opcode.  Note that some
   opcodes have defined bits in odd places so not all forms of a command
   will be in the same place.  This is done so that a binary search can be
   done to find the opcodes.  Note that table D.2 in the MPC860 User's
   Manual "Instructions Sorted by Opcode" does not account for these
   bit locations */

struct opcode opcodes[] = {
  { D_OPCODE(3),           D_MASK,   {O_TO, O_rA, O_SIMM, 0},
    0,               "twi",          0 },
  { D_OPCODE(7),           D_MASK,   {O_rD, O_rA, O_SIMM, 0},
    0,               "mulli",        0 },
  { D_OPCODE(8),           D_MASK,   {O_rD, O_rA, O_SIMM, 0},
    0,               "subfic",       0 },
  { D_OPCODE(10),          D_MASK,   {O_crfD, O_L, O_rA, O_UIMM, 0},
    0,               "cmpli",        0 },
  { D_OPCODE(11),          D_MASK,   {O_crfD, O_L, O_rA, O_SIMM, 0},
    0,               "cmpi",         0 },
  { D_OPCODE(12),          D_MASK,   {O_rD, O_rA, O_SIMM, 0},
    0,               "addic",        0 },
  { D_OPCODE(13),          D_MASK,   {O_rD, O_rA, O_SIMM, 0},
    0,               "addic.",       0 },
  { D_OPCODE(14),          D_MASK,   {O_rD, O_rA, O_SIMM, 0},
    0,               "addi",         H_RA0_IS_0 },
  { D_OPCODE(15),          D_MASK,   {O_rD, O_rA, O_SIMM, 0},
    0,               "addis",        H_RA0_IS_0|H_IMM_HIGH },
  { B_OPCODE(16,0,0),      B_MASK,   {O_BO, O_BI, O_BD, O_AA, O_LK, 0},
    handle_bc,       "bc",           H_RELATIVE },
  { B_OPCODE(16,0,1),      B_MASK,   {O_BO, O_BI, O_BD, O_AA, O_LK, 0},
    0,               "bcl",          H_RELATIVE },
  { B_OPCODE(16,1,0),      B_MASK,   {O_BO, O_BI, O_BD, O_AA, O_LK, 0},
    0,               "bca",          0 },
  { B_OPCODE(16,1,1),      B_MASK,   {O_BO, O_BI, O_BD, O_AA, O_LK, 0},
    0,               "bcla",         0 },
  { SC_OPCODE(17),         SC_MASK,  {0},
    0,               "sc",           0 },
  { I_OPCODE(18,0,0),      I_MASK,   {O_LI, O_AA, O_LK, 0},
    0,               "b",            H_RELATIVE },
  { I_OPCODE(18,0,1),      I_MASK,   {O_LI, O_AA, O_LK, 0},
    0,               "bl",           H_RELATIVE },
  { I_OPCODE(18,1,0),      I_MASK,   {O_LI, O_AA, O_LK, 0},
    0,               "ba",           0 },
  { I_OPCODE(18,1,1),      I_MASK,   {O_LI, O_AA, O_LK, 0},
    0,               "bla",          0 },
  { XL_OPCODE(19,0,0),     XL_MASK,  {O_crfD, O_crfS},
    0,               "mcrf",         0 },
  { XL_OPCODE(19,16,0),    XL_MASK,  {O_BO, O_BI, O_LK, 0},
    0,               "bclr",         0 },
  { XL_OPCODE(19,16,1),    XL_MASK,  {O_BO, O_BI, O_LK, 0},
    0,               "bclrl",        0 },
  { XL_OPCODE(19,33,0),    XL_MASK,  {O_crbD, O_crbA, O_crbB, 0},
    0,               "crnor",        0 },
  { XL_OPCODE(19,50,0),    XL_MASK,  {0},
    0,               "rfi",          0 },
  { XL_OPCODE(19,129,0),   XL_MASK,  {O_crbD, O_crbA, O_crbB, 0},
    0,               "crandc",       0 },
  { XL_OPCODE(19,150,0),   XL_MASK,  {0},
    0,               "isync",        0 },
  { XL_OPCODE(19,193,0),   XL_MASK,  {O_crbD, O_crbA, O_crbB, 0},
    0,               "crxor",        0 },
  { XL_OPCODE(19,225,0),   XL_MASK,  {O_crbD, O_crbA, O_crbB, 0},
    0,               "crnand",       0 },
  { XL_OPCODE(19,257,0),   XL_MASK,  {O_crbD, O_crbA, O_crbB, 0},
    0,               "crand",        0 },
  { XL_OPCODE(19,289,0),   XL_MASK,  {O_crbD, O_crbA, O_crbB, 0},
    0,               "creqv",        0 },
  { XL_OPCODE(19,417,0),   XL_MASK,  {O_crbD, O_crbA, O_crbB, 0},
    0,               "crorc",        0 },
  { XL_OPCODE(19,449,0),   XL_MASK,  {O_crbD, O_crbA, O_crbB, 0},
    0,               "cror",         0 },
  { XL_OPCODE(19,528,0),   XL_MASK,  {O_BO, O_BI, O_LK, 0},
    0,               "bcctr",        0 },
  { XL_OPCODE(19,528,1),   XL_MASK,  {O_BO, O_BI, O_LK, 0},
    0,               "bcctrl",       0 },
  { M_OPCODE(20,0),        M_MASK,   {O_rA, O_rS, O_SH, O_MB, O_ME, O_Rc, 0},
    0,               "rlwimi",       0 },
  { M_OPCODE(20,1),        M_MASK,   {O_rA, O_rS, O_SH, O_MB, O_ME, O_Rc, 0},
    0,               "rlwimi.",      0 },
  { M_OPCODE(21,0),        M_MASK,   {O_rA, O_rS, O_SH, O_MB, O_ME, O_Rc, 0},
    0,               "rlwinm",       0 },
  { M_OPCODE(21,1),        M_MASK,   {O_rA, O_rS, O_SH, O_MB, O_ME, O_Rc, 0},
    0,               "rlwinm.",      0 },
  { M_OPCODE(23,0),        M_MASK,   {O_rA, O_rS, O_rB, O_MB, O_ME, O_Rc, 0},
    0,               "rlwnm",        0 },
  { M_OPCODE(23,1),        M_MASK,   {O_rA, O_rS, O_rB, O_MB, O_ME, O_Rc, 0},
    0,               "rlwnm.",       0 },
  { D_OPCODE(24),          D_MASK,   {O_rA, O_rS, O_UIMM, 0},
    0,               "ori",          0 },
  { D_OPCODE(25),          D_MASK,   {O_rA, O_rS, O_UIMM, 0},
    0,               "oris",         H_IMM_HIGH },
  { D_OPCODE(26),          D_MASK,   {O_rA, O_rS, O_UIMM, 0},
    0,               "xori",         0 },
  { D_OPCODE(27),          D_MASK,   {O_rA, O_rS, O_UIMM, 0},
    0,               "xoris",        H_IMM_HIGH },
  { D_OPCODE(28),          D_MASK,   {O_rA, O_rS, O_UIMM, 0},
    0,               "andi.",        0 },
  { D_OPCODE(29),          D_MASK,   {O_rA, O_rS, O_UIMM, 0},
    0,               "andis.",       H_IMM_HIGH },
  { X_OPCODE(31,0,0),      X_MASK,   {O_crfD, O_L, O_rA, O_rB, 0},
    0,               "cmp",          0 },
  { X_OPCODE(31,4,0),      X_MASK,   {O_TO, O_rA, O_rB, 0},
    0,               "tw",           0 },
  { XO_OPCODE(31,8,0,0),   XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfc",        0 },
  { XO_OPCODE(31,8,0,1),   XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfc.",       0 },
  { XO_OPCODE(31,10,0,0),  XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "addc",         0 },
  { XO_OPCODE(31,10,0,1),  XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "addc.",        0 },
  { XO_OPCODE(31,11,0,0),  XO_MASK,  {O_rD, O_rA, O_rB, O_Rc, 0},
    0,               "mulhwu",       0 },
  { XO_OPCODE(31,11,0,1),  XO_MASK,  {O_rD, O_rA, O_rB, O_Rc, 0},
    0,               "mulhwu.",      0 },
  { X_OPCODE(31,19,0),     X_MASK,   {O_rD, 0},
    0,               "mfcr",         0 },
  { X_OPCODE(31,20,0),     X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lwarx",        H_RA0_IS_0 },
  { X_OPCODE(31,23,0),     X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lwzx",         H_RA0_IS_0 },
  { X_OPCODE(31,24,0),     X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "slw",          0 },
  { X_OPCODE(31,24,1),     X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "slw.",         0 },
  { X_OPCODE(31,26,0),     X_MASK,   {O_rA, O_rS, O_Rc, 0 },
    0,               "cntlzw",       0 },
  { X_OPCODE(31,26,1),     X_MASK,   {O_rA, O_rS, O_Rc, 0},
    0,               "cntlzw.",      0 },
  { X_OPCODE(31,28,0),     X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "and",          0 },
  { X_OPCODE(31,28,1),     X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "and.",         0 },
  { X_OPCODE(31,32,0),     X_MASK,   {O_crfD, O_L, O_rA, O_rB, 0},
    0,               "cmpl",         0 },
  { XO_OPCODE(31,40,0,0),  XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subf",         0 },
  { XO_OPCODE(31,40,0,1),  XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subf.",        0 },
  { X_OPCODE(31,54,0),     X_MASK,   {O_rA, O_rB, 0},
    0,               "dcbst",        H_RA0_IS_0 },
  { X_OPCODE(31,55,0),     X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lwzux",        0 },
  { X_OPCODE(31,60,0),     X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "andc",         0 },
  { X_OPCODE(31,60,1),     X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "andc.",        0 },
  { XO_OPCODE(31,75,0,0),  XO_MASK,  {O_rD, O_rA, O_rB, O_Rc, 0},
    0,               "mulhw",        0 },
  { XO_OPCODE(31,75,0,1),  XO_MASK,  {O_rD, O_rA, O_rB, O_Rc, 0},
    0,               "mulhw.",       0 },
  { X_OPCODE(31,83,0),     X_MASK,   {O_rD, 0},
    0,               "mfmsr",        0 },
  { X_OPCODE(31,86,0),     X_MASK,   {O_rA, O_rB, 0},
    0,               "dcbf",         H_RA0_IS_0 },
  { X_OPCODE(31,87,0),     X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lbzx",         H_RA0_IS_0 },
  { XO_OPCODE(31,104,0,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "neg",          0 },
  { XO_OPCODE(31,104,0,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "neg.",         0 },
  { X_OPCODE(31,119,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lbzux",        0 },
  { X_OPCODE(31,124,0),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "nor",          0 },
  { X_OPCODE(31,124,1),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "nor.",         0 },
  { XO_OPCODE(31,136,0,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfe",        0 },
  { XO_OPCODE(31,136,0,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfe.",       0 },
  { XO_OPCODE(31,138,0,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "adde",         0 },
  { XO_OPCODE(31,138,0,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "adde.",        0 },
  { XFX_OPCODE(31,144,0),  XFX_MASK, {O_CRM, O_rS, 0},
    0,               "mtcrf",        0 },
  { X_OPCODE(31,146,0),    X_MASK,   {O_rS, 0},
    0,               "mtmsr",        0 },
  { X_OPCODE(31,150,1),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "stwcx.",       0 },
  { X_OPCODE(31,151,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "stwx",         0 },
  { X_OPCODE(31,183,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "stwux",        0 },
  { XO_OPCODE(31,200,0,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "subfze",       0 },
  { XO_OPCODE(31,200,0,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "subfze.",      0 },
  { XO_OPCODE(31,202,0,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "addze",        0 },
  { XO_OPCODE(31,202,0,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "addze.",       0 },
  { X_OPCODE(31,210,0),    X_MASK,   {O_SR, O_rS, 0},
    0,               "mtsr",         0 },
  { X_OPCODE(31,215,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "stbx",         H_RA0_IS_0 },
  { XO_OPCODE(31,232,0,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "subfme",       0 },
  { XO_OPCODE(31,232,0,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "subfme.",      0 },
  { XO_OPCODE(31,234,0,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "addme",        0 },
  { XO_OPCODE(31,234,0,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "addme.",       0 },
  { XO_OPCODE(31,235,0,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "mullw",        0 },
  { XO_OPCODE(31,235,0,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "mullw.",       0 },
  { X_OPCODE(31,242,0),    X_MASK,   {O_rS, O_rB, 0},
    0,               "mtsrin",       0 },
  { X_OPCODE(31,246,0),    X_MASK,   {O_rA, O_rB, 0},
    0,               "dcbtst",       H_RA0_IS_0 },
  { X_OPCODE(31,247,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "stbux",        0 },
  { XO_OPCODE(31,266,0,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "add",          0 },
  { XO_OPCODE(31,266,0,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "add.",         0 },
  { X_OPCODE(31,278,0),    X_MASK,   {O_rA, O_rB, 0},
    0,               "dcbt",         H_RA0_IS_0 },
  { X_OPCODE(31,279,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lhzx",         H_RA0_IS_0 },
  { X_OPCODE(31,284,0),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "eqv",          0 },
  { X_OPCODE(31,284,1),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "eqv.",         0 },
  { X_OPCODE(31,306,0),    X_MASK,   {O_rB, 0},
    0,               "tlbie",        0 },
  { X_OPCODE(31,310,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "eciwx",        H_RA0_IS_0 },
  { X_OPCODE(31,311,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lhzux",        0 },
  { X_OPCODE(31,316,0),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "xor",          0 },
  { X_OPCODE(31,316,1),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "xor.",         0 },
  { XFX_OPCODE(31,339,0),  XFX_MASK, {O_rD, O_spr, 0},
    0,               "mfspr",        0 },
  { X_OPCODE(31,343,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lhax",         H_RA0_IS_0 },
  { X_OPCODE(31,370,0),    X_MASK,   {0},
    0,               "tlbia",        0 },
  { XFX_OPCODE(31,371,0),  XFX_MASK, {O_rD, O_tbr, 0},
    0,               "mftb",         0 },
  { X_OPCODE(31,375,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lhaux",        0 },
  { X_OPCODE(31,407,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "sthx",         H_RA0_IS_0 },
  { X_OPCODE(31,412,0),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "orc",          0 },
  { X_OPCODE(31,412,1),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "orc.",         0 },
  { X_OPCODE(31,438,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "ecowx",        H_RA0_IS_0 },
  { X_OPCODE(31,439,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "sthux",        0 },
  { X_OPCODE(31,444,0),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "or",           0 },
  { X_OPCODE(31,444,1),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "or.",          0 },
  { XO_OPCODE(31,459,0,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "divwu",        0 },
  { XO_OPCODE(31,459,0,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "divwu.",       0 },
  { XFX_OPCODE(31,467,0),  XFX_MASK, {O_spr, O_rS, 0},
    0,               "mtspr",        0 },
  { X_OPCODE(31,470,0),    X_MASK,   {O_rA, O_rB, 0},
    0,               "dcbi",         H_RA0_IS_0 },
  { X_OPCODE(31,476,0),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "nand",         0 },
  { X_OPCODE(31,476,1),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc,0},
    0,               "nand.",        0 },
  { XO_OPCODE(31,491,0,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "divw",         0 },
  { XO_OPCODE(31,491,0,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "divw.",        0 },
  { X_OPCODE(31,512,0),    X_MASK,   {O_crfD, 0},
    0,               "mcrxr",        0 },
  { XO_OPCODE(31,8,1,0),   XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfco",       0 },
  { XO_OPCODE(31,8,1,1),   XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfco.",      0 },
  { XO_OPCODE(31,10,1,0),  XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "addco",        0 },
  { XO_OPCODE(31,10,1,1),  XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "addco.",       0 },
  { X_OPCODE(31,533,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lswx",         H_RA0_IS_0 },
  { X_OPCODE(31,534,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lwbrx",        H_RA0_IS_0 },
  { X_OPCODE(31,536,0),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "srw",          0 },
  { X_OPCODE(31,536,1),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "srw.",         0 },
  { XO_OPCODE(31,40,1,0),  XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfo",        0 },
  { XO_OPCODE(31,40,1,1),  XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfo.",       0 },
  { X_OPCODE(31,566,0),    X_MASK,   {0},
    0,               "tlbsync",      0 },
  { X_OPCODE(31,595,0),    X_MASK,   {O_rD, O_SR, 0},
    0,               "mfsr",         0 },
  { X_OPCODE(31,597,0),    X_MASK,   {O_rD, O_rA, O_NB, 0},
    0,               "lswi",         H_RA0_IS_0 },
  { X_OPCODE(31,598,0),    X_MASK,   {0},
    0,               "sync",         0 },
  { XO_OPCODE(31,104,1,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "nego",         0 },
  { XO_OPCODE(31,104,1,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "nego.",        0 },
  { XO_OPCODE(31,136,1,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfeo",       0 },
  { XO_OPCODE(31,136,1,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "subfeo.",      0 },
  { XO_OPCODE(31,138,1,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "addeo",        0 },
  { XO_OPCODE(31,138,1,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "addeo.",       0 },
  { X_OPCODE(31,659,0),    X_MASK,   {O_rD, O_rB, 0},
    0,               "mfsrin",       0 },
  { X_OPCODE(31,661,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "stswx",        H_RA0_IS_0 },
  { X_OPCODE(31,662,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "stwbrx",       H_RA0_IS_0 },
  { XO_OPCODE(31,200,1,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "subfzeo",      0 },
  { XO_OPCODE(31,200,1,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "subfzeo.",     0 },
  { XO_OPCODE(31,202,1,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "addzeo",       0 },
  { XO_OPCODE(31,202,1,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "addzeo.",      0 },
  { X_OPCODE(31,725,0),    X_MASK,   {O_rS, O_rA, O_NB, 0},
    0,               "stswi",        H_RA0_IS_0 },
  { XO_OPCODE(31,232,1,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "subfmeo",      0 },
  { XO_OPCODE(31,232,1,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "subfmeo.",     0 },
  { XO_OPCODE(31,234,1,0), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "addmeo",       0 },
  { XO_OPCODE(31,234,1,1), XO_MASK,  {O_rD, O_rA, O_OE, O_Rc, 0},
    0,               "addmeo.",      0 },
  { XO_OPCODE(31,235,1,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "mullwo",       0 },
  { XO_OPCODE(31,235,1,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "mullwo.",      0 },
  { XO_OPCODE(31,266,1,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "addo",         0 },
  { XO_OPCODE(31,266,1,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "addo.",        0 },
  { X_OPCODE(31,790,0),    X_MASK,   {O_rD, O_rA, O_rB, 0},
    0,               "lhbrx",        H_RA0_IS_0 },
  { X_OPCODE(31,792,0),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "sraw",         0 },
  { X_OPCODE(31,792,1),    X_MASK,   {O_rA, O_rS, O_rB, O_Rc, 0},
    0,               "sraw.",        0 },
  { X_OPCODE(31,824,0),    X_MASK,   {O_rA, O_rS, O_SH, O_Rc, 0},
    0,               "srawi",        0 },
  { X_OPCODE(31,824,1),    X_MASK,   {O_rA, O_rS, O_SH, O_Rc, 0},
    0,               "srawi.",       0 },
  { X_OPCODE(31,854,0),    X_MASK,   {0},
    0,               "eieio",        0 },
  { X_OPCODE(31,918,0),    X_MASK,   {O_rS, O_rA, O_rB, 0},
    0,               "sthbrx",       H_RA0_IS_0 },
  { X_OPCODE(31,922,0),    X_MASK,   {O_rA, O_rS, O_Rc, 0},
    0,               "extsh",        0 },
  { X_OPCODE(31,922,1),    X_MASK,   {O_rA, O_rS, O_Rc, 0},
    0,               "extsh.",       0 },
  { X_OPCODE(31,954,0),    X_MASK,   {O_rA, O_rS, O_Rc, 0},
    0,               "extsb",        0 },
  { X_OPCODE(31,954,1),    X_MASK,   {O_rA, O_rS, O_Rc, 0},
    0,               "extsb.",       0 },
  { XO_OPCODE(31,459,1,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "divwuo",       0 },
  { XO_OPCODE(31,459,1,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "divwuo.",      0 },
  { X_OPCODE(31,978,0),    X_MASK,   {O_rB, 0},
    0,               "tlbld",        0 },
  { X_OPCODE(31,982,0),    X_MASK,   {O_rA, O_rB, 0},
    0,               "icbi",         H_RA0_IS_0 },
  { XO_OPCODE(31,491,1,0), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "divwo",        0 },
  { XO_OPCODE(31,491,1,1), XO_MASK,  {O_rD, O_rA, O_rB, O_OE, O_Rc, 0},
    0,               "divwo.",       0 },
  { X_OPCODE(31,1010,0),   X_MASK,   {O_rB, 0},
    0,               "tlbli",        0 },
  { X_OPCODE(31,1014,0),   X_MASK,   {O_rA, O_rB, 0},
    0,               "dcbz",         H_RA0_IS_0 },
  { D_OPCODE(32),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lwz",          H_RA0_IS_0 },
  { D_OPCODE(33),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lwzu",         0 },
  { D_OPCODE(34),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lbz",          H_RA0_IS_0 },
  { D_OPCODE(35),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lbzu",         0 },
  { D_OPCODE(36),          D_MASK,   {O_rS, O_d, O_rA, 0},
    0,               "stw",          H_RA0_IS_0 },
  { D_OPCODE(37),          D_MASK,   {O_rS, O_d, O_rA, 0},
    0,               "stwu",         0 },
  { D_OPCODE(38),          D_MASK,   {O_rS, O_d, O_rA, 0},
    0,               "stb",          H_RA0_IS_0 },
  { D_OPCODE(39),          D_MASK,   {O_rS, O_d, O_rA, 0},
    0,               "stbu",         0 },
  { D_OPCODE(40),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lhz",          H_RA0_IS_0 },
  { D_OPCODE(41),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lhzu",         0 },
  { D_OPCODE(42),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lha",          H_RA0_IS_0 },
  { D_OPCODE(43),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lhau",         0 },
  { D_OPCODE(44),          D_MASK,   {O_rS, O_d, O_rA, 0},
    0,               "sth",          H_RA0_IS_0 },
  { D_OPCODE(45),          D_MASK,   {O_rS, O_d, O_rA, 0},
    0,               "sthu",         0 },
  { D_OPCODE(46),          D_MASK,   {O_rD, O_d, O_rA, 0},
    0,               "lmw",          H_RA0_IS_0 },
  { D_OPCODE(47),          D_MASK,   {O_rS, O_d, O_rA, 0},
    0,               "stmw",         H_RA0_IS_0 },
};

const unsigned int n_opcodes = sizeof(opcodes) / sizeof(opcodes[0]);

struct spr_info spr_map[] = {
  { SPR_XER,	"XER" },
  { SPR_LR,	"LR" },
  { SPR_CTR,	"CTR" },
  { SPR_DSISR,	"DSISR" },
  { SPR_DAR,	"DAR" },
  { SPR_DEC,	"DEC" },
  { SPR_SRR0,	"SRR0" },
  { SPR_SRR1,	"SRR1" },
  { SPR_EIE,	"EIE" },
  { SPR_EID,	"EID" },
  { SPR_CMPA,	"CMPA" },
  { SPR_CMPB,	"CMPB" },
  { SPR_CMPC,	"CMPC" },
  { SPR_CMPD,	"CMPD" },
  { SPR_ICR,	"ICR" },
  { SPR_DER,	"DER" },
  { SPR_COUNTA,	"COUNTA" },
  { SPR_COUNTB,	"COUNTB" },
  { SPR_CMPE,	"CMPE" },
  { SPR_CMPF,	"CMPF" },
  { SPR_CMPG,	"CMPG" },
  { SPR_CMPH,	"CMPH" },
  { SPR_LCTRL1,	"LCTRL1" },
  { SPR_LCTRL2,	"LCTRL2" },
  { SPR_ICTRL,	"ICTRL" },
  { SPR_BAR,	"BAR" },
  { SPR_USPRG0,	"USPRG0" },
  { SPR_SPRG4_RO,	"SPRG4_RO" },
  { SPR_SPRG5_RO,	"SPRG5_RO" },
  { SPR_SPRG6_RO,	"SPRG6_RO" },
  { SPR_SPRG7_RO,	"SPRG7_RO" },
  { SPR_SPRG0,	"SPRG0" },
  { SPR_SPRG1,	"SPRG1" },
  { SPR_SPRG2,	"SPRG2" },
  { SPR_SPRG3,	"SPRG3" },
  { SPR_SPRG4,	"SPRG4" },
  { SPR_SPRG5,	"SPRG5" },
  { SPR_SPRG6,	"SPRG6" },
  { SPR_SPRG7,	"SPRG7" },
  { SPR_EAR,	"EAR" },
  { SPR_TBL,	"TBL" },
  { SPR_TBU,	"TBU" },
  { SPR_IC_CST,	"IC_CST" },
  { SPR_IC_ADR,	"IC_ADR" },
  { SPR_IC_DAT,	"IC_DAT" },
  { SPR_DC_CST,	"DC_CST" },
  { SPR_DC_ADR,	"DC_ADR" },
  { SPR_DC_DAT,	"DC_DAT" },
  { SPR_DPDR,	"DPDR" },
  { SPR_IMMR,	"IMMR" },
  { SPR_MI_CTR,	"MI_CTR" },
  { SPR_MI_AP,	"MI_AP" },
  { SPR_MI_EPN,	"MI_EPN" },
  { SPR_MI_TWC,	"MI_TWC" },
  { SPR_MI_RPN,	"MI_RPN" },
  { SPR_MD_CTR,	"MD_CTR" },
  { SPR_M_CASID,	"M_CASID" },
  { SPR_MD_AP,	"MD_AP" },
  { SPR_MD_EPN,	"MD_EPN" },
  { SPR_M_TWB,	"M_TWB" },
  { SPR_MD_TWC,	"MD_TWC" },
  { SPR_MD_RPN,	"MD_RPN" },
  { SPR_M_TW,	"M_TW" },
  { SPR_MI_DBCAM,	"MI_DBCAM" },
  { SPR_MI_DBRAM0,	"MI_DBRAM0" },
  { SPR_MI_DBRAM1,	"MI_DBRAM1" },
  { SPR_MD_DBCAM,	"MD_DBCAM" },
  { SPR_MD_DBRAM0,	"MD_DBRAM0" },
  { SPR_MD_DBRAM1,	"MD_DBRAM1" },
  { SPR_ZPR,	"ZPR" },
  { SPR_PID,	"PID" },
  { SPR_CCR0,	"CCR0" },
  { SPR_IAC3,	"IAC3" },
  { SPR_IAC4,	"IAC4" },
  { SPR_DVC1,	"DVC1" },
  { SPR_DVC2,	"DVC2" },
  { SPR_SGR,	"SGR" },
  { SPR_DCWR,	"DCWR" },
  { SPR_SLER,	"SLER" },
  { SPR_SU0R,	"SU0R" },
  { SPR_DBCR1,	"DBCR1" },
  { SPR_ICDBDR,	"ICDBDR" },
  { SPR_ESR,	"ESR" },
  { SPR_DEAR,	"DEAR" },
  { SPR_EVPR,	"EVPR" },
  { SPR_TSR,	"TSR" },
  { SPR_TCR,	"TCR" },
  { SPR_PIT,	"PIT" },
  { SPR_SRR2,	"SRR2" },
  { SPR_SRR3,	"SRR3" },
  { SPR_DBSR,	"DBSR" },
  { SPR_DBCR0,	"DBCR0" },
  { SPR_IAC1,	"IAC1" },
  { SPR_IAC2,	"IAC2" },
  { SPR_DAC1,	"DAC1" },
  { SPR_DAC2,	"DAC2" },
  { SPR_DCCR,	"DCCR" },
  { SPR_ICCR,	"ICCR" },
};

const unsigned int n_sprs = sizeof(spr_map) / sizeof(spr_map[0]);

#endif

/*
 * Copyright (c) 2000 William L. Pitts and W. Gerald Hicks
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 */
