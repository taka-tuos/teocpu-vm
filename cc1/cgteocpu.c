/*
Copyright (c) 2012-2015, Alexey Frunze
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*****************************************************************************/
/*                                                                           */
/*                                Smaller C                                  */
/*                                                                           */
/*                 A simple and small single-pass C compiler                 */
/*                                                                           */
/*                          teocpu code generator                            */
/*                                                                           */
/*****************************************************************************/
#include <stdint.h>

enum {
	EF_X=0x0000,
	EF_Z=0x0001,
	EF_E=0x0002,
	EF_A=0x0004,
	EF_B=0x0008,
};

#define MAX_GLOBALS_TABLE_LEN 16384

STATIC
void GenInit(void)
{
	// initialization of target-specific code generator
	SizeOfWord = 4;
	OutputFormat = FormatSegmented;
	/*CodeHeaderFooter[0] = "\tsection .text";
	DataHeaderFooter[0] = "\tsection .data";
	RoDataHeaderFooter[0] = "\tsection .rodata";
	BssHeaderFooter[0] = "\tsection .bss";*/
	CodeHeaderFooter[0] = "";
	DataHeaderFooter[0] = "";
	RoDataHeaderFooter[0] = "";
	BssHeaderFooter[0] = "";
}

STATIC
int GenInitParams(int argc, char** argv, int* idx)
{
	(void)argc;
	(void)argv;
	(void)idx;
	// initialization of target-specific code generator with parameters
	
	return 0;
}

STATIC
void GenInitFinalize(void)
{
	// finalization of initialization of target-specific code generator
}

STATIC
void GenStartCommentLine(void)
{
	printf2("\t; ");
}

STATIC
void GenWordAlignment(int bss)
{
	(void)bss;
	printf2("\talign #2\n");
}

STATIC
void GenLabel(char* Label, int Static)
{
	{
		printf2("._%s\n", Label);
		if (!Static && GenExterns)
			printf2("\tglobal ._%s\n", Label);
	}
	GenAddGlobal(Label, 1);
}

STATIC
void GenPrintLabel(char* Label)
{
	{
		if (isdigit(*Label))
			printf2(".L%s", Label);
		else
			printf2("._%s", Label);
	}
}

STATIC
void GenNumLabel(int Label)
{
	printf2(".L%d\n", Label);
}

STATIC
void GenPrintNumLabel(int label)
{
	printf2(".L%d", label);
}

STATIC
void GenZeroData(unsigned Size, int bss)
{
	(void)bss;
	printf2("\tfill #%d\n", Size);
}

STATIC
void GenIntData(int Size, int Val)
{
	Val = truncInt(Val);
	if (Size == 1)
		printf2("\tdb #%d\n", Val);
	else if (Size == 2)
		printf2("\tdw #%d\n", Val);
	else if (Size == 4)
		printf2("\tdd #%d\n", Val);
}

STATIC
void GenStartAsciiString(void)
{
	printf2("\ttext(");
}

STATIC
void GenEndAsciiString(void)
{
	printf2(")\n");
}

char GlobalsTable[MAX_GLOBALS_TABLE_LEN];
int GlobalsTableLen = 0;


STATIC
void GenAddGlobal(char* s, int use)
{
	int i = 0;
	int l;
	if (GenExterns)
	{
		while (i < GlobalsTableLen)
		{
			if (!strcmp(GlobalsTable + i + 2, s))
			{
				GlobalsTable[i] |= use;
				return;
			}
			i += GlobalsTable[i + 1] + 2;
		}
		l = strlen(s) + 1;
		if (GlobalsTableLen + l + 2 > MAX_GLOBALS_TABLE_LEN)
			error("Table of globals exhausted\n");
		GlobalsTable[GlobalsTableLen++] = use;
		GlobalsTable[GlobalsTableLen++] = l;
		memcpy(GlobalsTable + GlobalsTableLen, s, l);
		GlobalsTableLen += l;
	}
}

STATIC
void GenAddrData(int Size, char* Label, int ofs)
{
	ofs = truncInt(ofs);
	if (Size == 1)
		printf2("\t.db\t");
	else if (Size == 2)
		printf2("\t.dw\t");
	else if (Size == 4)
		printf2("\t.dd\t");
	GenPrintLabel(Label);
	if (ofs)
		printf2(" %+d", ofs);
	puts2("");
	if (!isdigit(*Label))
		GenAddGlobal(Label, 2);
}

STATIC
int GenFxnSizeNeeded(void)
{
	return 0;
}

STATIC
void GenRecordFxnSize(char* startLabelName, int endLabelNo)
{
	(void)startLabelName;
	(void)endLabelNo;
}

#define teocInstrNop		0x00

#define teocInstrMov		0x80

#define teocInstrLi			0x80
#define teocInstrLr			0x81
#define teocInstrSr			0x82

#define teocInstrSxB		0x83
#define teocInstrSxW		0x84

#define teocInstrLB			0x02
#define teocInstrLW			0x03
#define teocInstrL			0x04
#define teocInstrSB			0x05
#define teocInstrSW			0x06
#define teocInstrS			0x07

#define teocInstrAdd		0x0A
#define teocInstrSub		0x0B
#define teocInstrRSub		0x0C
#define teocInstrAnd		0x0D
#define teocInstrOr			0x0E
#define teocInstrXor		0x0F
#define teocInstrNot		0x10
#define teocInstrLls		0x11
#define teocInstrLrs		0x12
#define teocInstrMul		0x13
#define teocInstrMuli		0x14
#define teocInstrDivi		0x15
#define teocInstrDiv		0x16

#define teocInstrJmp		0x17
#define teocInstrCall		0x18
#define teocInstrCJmp		0x19
#define teocInstrCCall		0x1A

#define teocInstrCe			0x20
#define teocInstrCeg		0x21
#define teocInstrCel		0x22
#define teocInstrCg			0x23
#define teocInstrCl			0x24
#define teocInstrTest		0x25
#define teocInstrTesti		0x26
#define teocInstrCmp		0x27
#define teocInstrCmpi		0x28

STATIC
void GenPrintInstr(int instr, int val)
{
	char *p="nop";
	
	switch (instr)
	{
	case teocInstrNop	: p = "nop"; break;

	case teocInstrLi	: p = "li"; break;
	case teocInstrLr	: p = "lr"; break;
	case teocInstrSr	: p = "sr"; break;
	
	case teocInstrSxB	: p = "sxb"; break;
	case teocInstrSxW	: p = "sxw"; break;

	case teocInstrLB	: p = "ldb"; break;
	case teocInstrLW	: p = "ldw"; break;
	case teocInstrL		: p = "ldd"; break;
	case teocInstrSB	: p = "stb"; break;
	case teocInstrSW	: p = "stw"; break;
	case teocInstrS		: p = "std"; break;
	
	case teocInstrAdd	: p = "add"; break;
	case teocInstrSub	: p = "sub"; break;
	case teocInstrRSub	: p = "rsub"; break;
	case teocInstrAnd	: p = "and"; break;
	case teocInstrOr	: p = "or"; break;
	case teocInstrXor	: p = "xor"; break;
	case teocInstrNot	: p = "not"; break;
	case teocInstrLls	: p = "lls"; break;
	case teocInstrLrs	: p = "lrs"; break;
	case teocInstrMul	: p = "mul"; break;
	case teocInstrMuli	: p = "muli"; break;
	case teocInstrDiv	: p = "div"; break;
	case teocInstrDivi	: p = "divi"; break;

	case teocInstrJmp	: p = "b"; break;
	case teocInstrCall	: p = "c"; break;
	case teocInstrCJmp	: p = "bc"; break;
	case teocInstrCCall	: p = "cc"; break;

	case teocInstrCmp	: p = "cmp"; break;
	case teocInstrCmpi	: p = "cmpi"; break;
	
	case teocInstrCe	: p = "ce"; break;
	case teocInstrCeg	: p = "ceg"; break;
	case teocInstrCel	: p = "cel"; break;
	case teocInstrCg	: p = "cg"; break;
	case teocInstrCl	: p = "cl"; break;
	
	case teocInstrTest	: p = "tst"; break;
	case teocInstrTesti	: p = "tsti"; break;
	}

	printf2("\t%s", p);
}

#define teocOpReg0											0x00
//...
//#define teocOpRegY											0x21
#define teocOpRegBp											0x3f
#define teocOpRegSp											0x41
//...
//#define teocOpRegFlags										0x23

#define teocOpIndReg0										0x50
//...
//#define teocOpIndRegY										0x51
#define teocOpIndRegBp										0x8f
#define teocOpIndRegSp										0x91
//...
//#define teocOpIndRegFlags									0x53

#define teocOpConst											0x100
#define teocOpLabel											0x101
#define teocOpNumLabel										0x102

#define MAX_TEMP_REGS 29 // this many temp registers used beginning with R1 to hold subexpression results
#define TEMP_REG_A 30 // two temporary registers used for momentary operations, similarly to the MIPS AT/R1 register
#define TEMP_REG_B 31

#define tokRevMinus		0x100 // reversed subtraction, RSB
#define tokRevIdent		0x101
#define tokRevLocalOfs	0x102

int GenRegsUsed; // bitmask of registers used by the function being compiled

int GenMemPrefix[] = { 'x', 'b', 'w', 'x', 'd' };

STATIC
void GenPrintOperand(int op, long val)
{
	if (op >= teocOpReg0 && op <= teocOpRegSp)
	{
		GenRegsUsed |= 1 << op;
		printf2("%%%d", op);
	}
	else if (op >= teocOpIndReg0 && op <= teocOpIndRegSp)
	{
		printf2("%%%d", op - teocOpIndReg0);
	}
	else
	{
		switch (op)
		{
		case teocOpConst: printf2("#%10d", truncInt(val)); break;
		case teocOpLabel: GenPrintLabel(IdentTable + val); break;
		case teocOpNumLabel: GenPrintNumLabel(val); break;

		default:
			//error("WTF!\n");
			errorInternal(100);
			break;
		}
	}
}

STATIC
void GenPrintOperandSeparator(void)
{
	printf2(",");
}

STATIC
void GenPrintOperandStarter(void)
{
	printf2(" ");
}

STATIC
void GenPrintOperandEnder(void)
{
	printf2("");
}

STATIC
void GenPrintNewLine(void)
{
	puts2("");
}

STATIC
void GenPrintInstrNoOperand(int instr, long instrval)
{
	GenPrintInstr(instr, instrval);
	GenPrintOperandStarter();
	GenPrintOperandEnder();
	GenPrintNewLine();
}

STATIC
void GenPrintInstrOneOperand(int instr, int instrval, int operand, long operandval)
{
	GenPrintInstr(instr, instrval);
	GenPrintOperandStarter();
	GenPrintOperand(operand, operandval);
	GenPrintOperandEnder();
	GenPrintNewLine();
}
/*
STATIC
void GenPrintInstr1Operand(int instr, int instrval, int operand, long operandval);
STATIC
void GenPrintInstr2Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val);
STATIC
void GenPrintInstr3Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val, int operand3, long operand3val);

STATIC
void GenPrintInstr1Operand(int instr, int instrval, int operand, long operandval)
{
	if(instr == teocInstrNot)
	{
		GenPrintInstr2Operands(instr, instrval,
														operand, operandval,
														operand, operandval);
	} else {
		GenPrintInstrOneOperand(operand < teocOpConst ? teocInstrPr : teocInstrPi, 0, operand, operandval);
		
		GenPrintInstrNoOperand(instr, instrval);
	}
}

STATIC
void GenPrintInstr2Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val)
{
	if(instr == teocInstrMov) {
		GenPrintInstrOneOperand(operand2 < teocOpConst ? teocInstrPr : teocInstrPi, 0, operand2, operand2val);
		
		GenPrintInstrOneOperand(teocInstrLr, 0, operand1, operand1val);
	} else {
		if(instr >= teocInstrAdd && instr <= teocInstrDiv) {
			GenPrintInstr3Operands(instr, instrval,
														operand1, operand1val,
														operand1, operand1val,
														operand2, operand2val);
		} else if(instr >= teocInstrLB && instr <= teocInstrL) {
			GenPrintInstr3Operands(instr, instrval,
														operand1, operand1val,
														operand2, operand2val,
														teocOpConst, operand2 < teocOpConst ? operand2val : 0);
		} else if(instr >= teocInstrSB && instr <= teocInstrS) {
			GenPrintInstr3Operands(instr, instrval,
														operand2, operand2val,
														operand1, operand1val,
														teocOpConst, operand1 < teocOpConst ? operand1val : 0);
		} else {
			GenPrintInstrOneOperand(operand2 < teocOpConst ? teocInstrPr : teocInstrPi, 0, operand2, operand2val);
			GenPrintInstrOneOperand(operand1 < teocOpConst ? teocInstrPr : teocInstrPi, 0, operand1, operand1val);
			
			GenPrintInstrNoOperand(instr, instrval);
		}
	}
}

STATIC
void GenPrintInstr3Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val, int operand3, long operand3val)
{
	GenPrintInstrOneOperand(operand3 < teocOpConst ? teocInstrPr : teocInstrPi, 0, operand3, operand3val);
	GenPrintInstrOneOperand(operand2 < teocOpConst ? teocInstrPr : teocInstrPi, 0, operand2, operand2val);
	GenPrintInstrOneOperand(operand1 < teocOpConst ? teocInstrPr : teocInstrPi, 0, operand1, operand1val);
	
	GenPrintInstrNoOperand(instr, instrval);
}
*/
STATIC
void GenExtendRegIfNeeded(int reg, int opSz)
{
	if (opSz == -1)
	{
		GenPrintInstrOneOperand(teocInstrLr, 0, reg, 0);
		GenPrintInstrNoOperand(teocInstrSxB, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, reg, 0);
	}
	else if (opSz == 1)
	{
		GenPrintInstrOneOperand(teocInstrLr, 0, reg, 0);
		GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0xff);
		GenPrintInstrNoOperand(teocInstrAnd, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, reg, 0);
	}
	else if (opSz == -2)
	{
		GenPrintInstrOneOperand(teocInstrLr, 0, reg, 0);
		GenPrintInstrNoOperand(teocInstrSxW, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, reg, 0);
	}
	else if (opSz == 2)
	{
		GenPrintInstrOneOperand(teocInstrLr, 0, reg, 0);
		GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0xffff);
		GenPrintInstrNoOperand(teocInstrAnd, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, reg, 0);
	}
}

STATIC
void GenJumpUncond(int label)
{
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpNumLabel, label);
	GenPrintInstrNoOperand(teocInstrJmp, 0);
}

extern int GenWreg; // GenWreg is defined below

#ifndef USE_SWITCH_TAB
STATIC
void GenJumpIfEqual(int val, int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfEqual\n");
#endif
	GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, val);
	GenPrintInstrNoOperand(teocInstrCmp, 0);
	GenPrintInstrNoOperand(teocInstrCe, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpNumLabel, label);
	GenPrintInstrNoOperand(teocInstrCJmp, 0);
}
#endif

STATIC
void GenJumpIfZero(int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfZero\n");
#endif
	GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
	GenPrintInstrNoOperand(teocInstrTest, 0);
	GenPrintInstrNoOperand(teocInstrCe, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpNumLabel, label);
	GenPrintInstrNoOperand(teocInstrCJmp, 0);
}

STATIC
void GenJumpIfNotZero(int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfNotZero\n");
#endif
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 2);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 4);
	GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
	GenPrintInstrNoOperand(teocInstrTest, 0);
	GenPrintInstrNoOperand(teocInstrXor, 0);
	GenPrintInstrNoOperand(teocInstrLrs, 0);
	GenPrintInstrNoOperand(teocInstrCe, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpNumLabel, label);
	GenPrintInstrNoOperand(teocInstrCJmp, 0);
}

STATIC
void GenSaveRestoreRegs(int save)
{
	int rstart, rstop, rinc, r;
	uint64_t mask = GenRegsUsed;
	mask &= ~(1 << teocOpReg0); // not preserved
//	mask &= ~(1 << teocOpRegY); // TBD??? Y is preserved, right???
	mask &= ~(1 << teocOpRegBp); // taken care of
	mask &= ~(1 << teocOpRegSp); // taken care of
	//mask &= ~(1 << teocOpRegFlags); // TBD??? flags aren't preserved, right???

	if (save)
		rstart = teocOpReg0, rstop = teocOpRegSp, rinc = 1;
	else
		rstart = teocOpRegSp, rstop = teocOpReg0, rinc = -1;

	for (r = rstart; r != rstop + rinc; r += rinc)
	{
		int used = (mask & (1 << r)) != 0;
		if (save || used)
		{
			if(used) printf2(save ? "\tlr %%%02d \n" : "\tlr %%%02d \n",r);
			else printf2(save ? "\t;lr %%%02d\n" : "\t;sr %%%02d\n",r);
		}
	}
	GenRegsUsed = mask; // undo changes in GenRegsUsed by GenPrintOperand()
}

void GenIsrProlog(void)
{
	// TBD???
}

void GenIsrEpilog(void)
{
	// TBD???
}

long GenPrologPos;
long GenPrologSiz;

STATIC
void GenWriteFrameSize(void)
{
	unsigned size = -CurFxnMinLocalOfs;
	//if(!size) GenStartCommentLine();
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, size*4);
	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpRegSp, 0);
	GenPrintInstrNoOperand(teocInstrSub, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, teocOpRegSp, 0);
	GenSaveRestoreRegs(1);
}

STATIC
void GenUpdateFrameSize(void)
{
	long pos;
	pos = ftell(OutFile);
	fseek(OutFile, GenPrologPos, SEEK_SET);
	GenWriteFrameSize();
	fseek(OutFile, GenPrologSiz, SEEK_SET);
	GenPrologSiz -= GenPrologPos;
	fseek(OutFile, pos, SEEK_SET);
}

STATIC
void GenFxnProlog(void)
{
	GenRegsUsed = 0;
	
	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpRegBp, 0);

	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpRegSp, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, teocOpRegBp, 0);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 4);
	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpRegBp, 0);
	GenPrintInstrNoOperand(teocInstrAdd, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, teocOpRegBp, 0);

	GenPrologPos = ftell(OutFile);
	GenWriteFrameSize();
	GenPrologSiz = ftell(OutFile);
	GenPrologSiz -= GenPrologPos;
}

STATIC
void GenGrowStack(int size)
{
	if (!size)
		return;
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, size);
	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpRegSp, 0);
	GenPrintInstrNoOperand(teocInstrSub, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, teocOpRegSp, 0);
}

STATIC
void GenFxnEpilog(void)
{
	GenUpdateFrameSize();

	GenSaveRestoreRegs(0);

	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 4);
	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpRegBp, 0);
	GenPrintInstrNoOperand(teocInstrSub, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, teocOpRegBp, 0);
	
	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpRegBp, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, teocOpRegSp, 0);
	
	GenPrintInstrOneOperand(teocInstrSr, 0, teocOpRegBp, 0);

	GenPrintInstrNoOperand(teocInstrJmp, 0);
}

STATIC
int GenMaxLocalsSize(void)
{
	return 0x7FFFFFFF;
}

STATIC
int GenGetBinaryOperatorInstr(int tok)
{
	switch (tok)
	{
	case tokPostAdd:
	case tokAssignAdd:
	case '+':
		return teocInstrAdd;
	case tokPostSub:
	case tokAssignSub:
	case '-':
		return teocInstrSub;
	case tokRevMinus:
		return teocInstrRSub;
	case '&':
	case tokAssignAnd:
		return teocInstrAnd;
	case '^':
	case tokAssignXor:
		return teocInstrXor;
	case '|':
	case tokAssignOr:
		return teocInstrOr;
	case '<':
	case '>':
	case tokLEQ:
	case tokGEQ:
	case tokEQ:
	case tokNEQ:
	case tokULess:
	case tokUGreater:
	case tokULEQ:
	case tokUGEQ:
		return teocInstrNop;
	case '*':
	case tokAssignMul:
		return teocInstrMul;
	case '/':
	case '%':
	case tokAssignDiv:
	case tokAssignMod:
		return teocInstrDivi;
	case tokUDiv:
	case tokUMod:
	case tokAssignUDiv:
	case tokAssignUMod:
		return teocInstrDiv;
	case tokLShift:
	case tokAssignLSh:
		return teocInstrLls;
	case tokRShift:
	case tokAssignRSh:
		return teocInstrLrs;
	case tokURShift:
	case tokAssignURSh:
		return teocInstrLrs;

	default:
		//error("Error: Invalid operator\n");
		errorInternal(101);
		return 0;
	}
}

STATIC
void GenReadIdent(int regDst, int opSz, int label)
{
	int instr = teocInstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = teocInstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = teocInstrLW;
	}
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpLabel, label);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);

	if (opSz == -1) {
		GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
		GenPrintInstrNoOperand(teocInstrSxB, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	} else if (opSz == -2) {
		GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
		GenPrintInstrNoOperand(teocInstrSxW, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	}
}

STATIC
void GenReadLocal(int regDst, int opSz, int ofs)
{
	int instr = teocInstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = teocInstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = teocInstrLW;
	}
	
	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpIndRegBp, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, ofs);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);

	if (opSz == -1) {
		GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
		GenPrintInstrNoOperand(teocInstrSxB, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	} else if (opSz == -2) {
		GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
		GenPrintInstrNoOperand(teocInstrSxW, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	}
}

STATIC
void GenReadIndirect(int regDst, int regSrc, int opSz)
{
	int instr = teocInstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = teocInstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = teocInstrLW;
	}
	
	GenPrintInstrOneOperand(teocInstrLr, 0, regSrc + teocOpIndReg0, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);

	if (opSz == -1) {
		GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
		GenPrintInstrNoOperand(teocInstrSxB, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	} else if (opSz == -2) {
		GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
		GenPrintInstrNoOperand(teocInstrSxW, 0);
		GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	}
}

STATIC
void GenWriteIdent(int regSrc, int opSz, int label)
{
	int instr = teocInstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = teocInstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = teocInstrSW;
	}
	
	GenPrintInstrOneOperand(teocInstrLr, 0, regSrc, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpLabel, label);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0);
	GenPrintInstrNoOperand(instr, 0);
}

STATIC
void GenWriteLocal(int regSrc, int opSz, int ofs)
{
	int instr = teocInstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = teocInstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = teocInstrSW;
	}
	
	GenPrintInstrOneOperand(teocInstrLr, 0, regSrc, 0);
	GenPrintInstrOneOperand(teocInstrLr, 0, teocOpIndRegBp, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, ofs);
	GenPrintInstrNoOperand(instr, 0);
}

STATIC
void GenWriteIndirect(int regDst, int regSrc, int opSz)
{
	int instr = teocInstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = teocInstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = teocInstrSW;
	}
	
	GenPrintInstrOneOperand(teocInstrLr, 0, regSrc, 0);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst + teocOpIndReg0, 0);
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0);
	GenPrintInstrNoOperand(instr, 0);
}

STATIC
void GenIncDecIdent(int regDst, int opSz, int label, int tok)
{
	int instr = teocInstrAdd;

	if (tok != tokInc)
		instr = teocInstrSub;

	GenReadIdent(regDst, opSz, label);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);

	GenWriteIdent(regDst, opSz, label);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenIncDecLocal(int regDst, int opSz, int ofs, int tok)
{
	int instr = teocInstrAdd;

	if (tok != tokInc)
		instr = teocInstrSub;

	GenReadLocal(regDst, opSz, ofs);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);

	GenWriteLocal(regDst, opSz, ofs);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenIncDecIndirect(int regDst, int regSrc, int opSz, int tok)
{
	int instr = teocInstrAdd;

	if (tok != tokInc)
		instr = teocInstrSub;

	GenReadIndirect(regDst, regSrc, opSz);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);

	GenWriteIndirect(regSrc, regDst, opSz);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecIdent(int regDst, int opSz, int label, int tok)
{
	int instr = teocInstrAdd;

	if (tok != tokPostInc)
		instr = teocInstrSub;

	GenReadIdent(regDst, opSz, label);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	
	GenWriteIdent(regDst, opSz, label);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, -1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecLocal(int regDst, int opSz, int ofs, int tok)
{
	int instr = teocInstrAdd;

	if (tok != tokPostInc)
		instr = teocInstrSub;

	GenReadLocal(regDst, opSz, ofs);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	
	GenWriteLocal(regDst, opSz, ofs);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, -1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecIndirect(int regDst, int regSrc, int opSz, int tok)
{
	int instr = teocInstrAdd;

	if (tok != tokPostInc)
		instr = teocInstrSub;

	GenReadIndirect(regDst, regSrc, opSz);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	
	GenWriteIndirect(regSrc, regDst, opSz);
	
	GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, -1);
	GenPrintInstrOneOperand(teocInstrLr, 0, regDst, 0);
	GenPrintInstrNoOperand(instr, 0);
	GenPrintInstrOneOperand(teocInstrSr, 0, regDst, 0);
	
	GenExtendRegIfNeeded(regDst, opSz);
}

int CanUseTempRegs;
int TempsUsed;
int GenWreg = teocOpReg0; // current working register (R0, R1, R2, etc)
int GenLreg, GenRreg; // left operand register and right operand register after GenPopReg()

/*
	General idea behind GenWreg, GenLreg, GenRreg:

	- In expressions w/o function calls:

		Subexpressions are evaluated in R0, R1, R2, ..., R<MAX_TEMP_REGS>. If those registers
		aren't enough, the stack is used additionally.

		The expression result ends up in R0, which is handy for returning from
		functions.

		In the process, GenWreg is the current working register and is one of: R0, R1, R2, ... .
		All unary operators are evaluated in the current working register.

		GenPushReg() and GenPopReg() advance GenWreg as needed when handling binary operators.

		GenPopReg() sets GenWreg, GenLreg and GenRreg. GenLreg and GenRreg are the registers
		where the left and right operands of a binary operator are.

		When the exression runs out of the temporary registers, the stack is used. While it is being
		used, GenWreg remains equal to the last temporary register, and GenPopReg() sets GenLreg = TEMP_REG_A.
		Hence, after GenPopReg() the operands of the binary operator are always in registers and can be
		directly manipulated with.

		Following GenPopReg(), binary operator evaluation must take the left and right operands from
		GenLreg and GenRreg and write the evaluated result into GenWreg. Care must be taken as GenWreg
		will be the same as either GenLreg (when the popped operand comes from R1-R<MAX_TEMP_REGS>)
		or GenRreg (when the popped operand comes from the stack in TEMP_REG_A).

	- In expressions with function calls:

		GenWreg is always R0. R1-R<MAX_TEMP_REGS> are not used. Instead the stack and TEMP_REG_A and TEMP_REG_B
		are used.
*/

STATIC
void GenWregInc(int inc)
{
	if (inc > 0)
	{
		// Advance the current working register to the next available temporary register
		GenWreg++;
	}
	else
	{
		// Return to the previous current working register
		GenWreg--;
	}
}

STATIC
void GenPushReg(void)
{
	if (CanUseTempRegs && TempsUsed < MAX_TEMP_REGS)
	{
		GenWregInc(1);
		TempsUsed++;
		return;
	}
	
	GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);

	TempsUsed++;
}

STATIC
void GenPopReg(void)
{
	TempsUsed--;

	if (CanUseTempRegs && TempsUsed < MAX_TEMP_REGS)
	{
		GenRreg = GenWreg;
		GenWregInc(-1);
		GenLreg = GenWreg;
		return;
	}

	GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);

	GenLreg = TEMP_REG_A;
	GenRreg = GenWreg;
}

STATIC
void GenPrep(int* idx)
{
	int tok;
	int oldIdxRight, oldIdxLeft, t0, t1;

	if (*idx < 0)
		//error("GenPrep(): idx < 0\n");
		errorInternal(100);

	tok = stack[*idx][0];

	oldIdxRight = --*idx;

	switch (tok)
	{
	case tokAssignMul:
	case tokUDiv:
	case tokUMod:
	case tokAssignUDiv:
	case tokAssignUMod:
		if (stack[oldIdxRight][0] == tokNumInt || stack[oldIdxRight][0] == tokNumUint)
		{
			unsigned m = truncUint(stack[oldIdxRight][1]);
			if (m && !(m & (m - 1)))
			{
				// Change multiplication to left shift, this helps indexing arrays of ints/pointers/etc
				if (tok == tokAssignMul)
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = tokAssignLSh;
				}
				// Change unsigned division to right shift and unsigned modulo to bitwise and
				else if (tok == tokUMod || tok == tokAssignUMod)
				{
					stack[oldIdxRight][1] = (int)(m - 1);
					tok = (tok == tokUMod) ? '&' : tokAssignAnd;
				}
				else
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = (tok == tokUDiv) ? tokURShift : tokAssignURSh;
				}
				stack[oldIdxRight + 1][0] = tok;
			}
		}
	}

	switch (tok)
	{
	case tokNumUint:
		stack[oldIdxRight + 1][0] = tokNumInt; // reduce the number of cases since tokNumInt and tokNumUint are handled the same way
		// fallthrough
	case tokNumInt:
	case tokIdent:
	case tokLocalOfs:
		break;

	case tokPostAdd:
	case tokPostSub:
	case '/':
	case '-':
	case '%':
	case tokUDiv:
	case tokUMod:
	case tokLShift:
	case tokRShift:
	case tokURShift:
	case tokLogAnd:
	case tokLogOr:
	case tokComma: {
		int xor;
	//case '-':
		switch(tok) {
			case '-':
				xor = '-' ^ tokRevMinus; break;
		}
		tok ^= xor;
		GenPrep(idx);
		// fallthrough
	}
	case tokShortCirc:
	case tokGoto:
	case tokUnaryStar:
	case tokInc:
	case tokDec:
	case tokPostInc:
	case tokPostDec:
	case '~':
	case tokUnaryPlus:
	case tokUnaryMinus:
	case tok_Bool:
	case tokVoid:
	case tokUChar:
	case tokSChar:
	case tokShort:
	case tokUShort:
		GenPrep(idx);
		break;

	case '=':
	//case '-':
	case tokAssignAdd:
	case tokAssignSub:
	case tokAssignMul:
	case tokAssignDiv:
	case tokAssignUDiv:
	case tokAssignMod:
	case tokAssignUMod:
	case tokAssignLSh:
	case tokAssignRSh:
	case tokAssignURSh:
	case tokAssignAnd:
	case tokAssignXor:
	case tokAssignOr:
		GenPrep(idx);
		oldIdxLeft = *idx;
		GenPrep(idx);
		// If the left operand is an identifier (with static or auto storage), swap it with the right operand
		// and mark it specially, so it can be used directly
		if ((t0 = stack[oldIdxLeft][0]) == tokIdent || t0 == tokLocalOfs)
		{
			t1 = stack[oldIdxLeft][1];
			memmove(stack[oldIdxLeft], stack[oldIdxLeft + 1], (oldIdxRight - oldIdxLeft) * sizeof(stack[0]));
			stack[oldIdxRight][0] = (t0 == tokIdent) ? tokRevIdent : tokRevLocalOfs;
			stack[oldIdxRight][1] = t1;
		}
		break;

	case '+':
	//case '-':
	case '*':
	case '&':
	case '^':
	case '|':
	case tokEQ:
	case tokNEQ:
	case '<':
	case '>':
	case tokLEQ:
	case tokGEQ:
	case tokULess:
	case tokUGreater:
	case tokULEQ:
	case tokUGEQ:
		GenPrep(idx);
		oldIdxLeft = *idx;
		GenPrep(idx);
		// If the right operand isn't a constant, but the left operand is, swap the operands
		// so the constant can become an immediate right operand in the instruction
		t1 = stack[oldIdxRight][0];
		t0 = stack[oldIdxLeft][0];
		if (t1 != tokNumInt && t0 == tokNumInt)
		{
			int xor;

			t1 = stack[oldIdxLeft][1];
			memmove(stack[oldIdxLeft], stack[oldIdxLeft + 1], (oldIdxRight - oldIdxLeft) * sizeof(stack[0]));
			stack[oldIdxRight][0] = t0;
			stack[oldIdxRight][1] = t1;

			switch (tok)
			{
			case '<':
			case '>':
				xor = '<' ^ '>'; break;
			case tokLEQ:
			case tokGEQ:
				xor = tokLEQ ^ tokGEQ; break;
			case tokULess:
			case tokUGreater:
				xor = tokULess ^ tokUGreater; break;
			case tokULEQ:
			case tokUGEQ:
				xor = tokULEQ ^ tokUGEQ; break;
			case '-':
				xor = '-' ^ tokRevMinus; break;
			default:
				xor = 0; break;
			}
			tok ^= xor;
		}
		if (stack[oldIdxRight][0] == tokNumInt)
		{
			unsigned m = truncUint(stack[oldIdxRight][1]);
			switch (tok)
			{
			case '*':
				// Change multiplication to left shift, this helps indexing arrays of ints/pointers/etc
				if (m && !(m & (m - 1)))
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = tokLShift;
				}
				break;
			case tokULEQ:
				// Change left <= const to left < const+1, but const+1 must be <=0xFFFFFFFFu
				if (m != 0xFFFFFFFF)
				{
					stack[oldIdxRight][1]++;
					tok = tokULess;
				}
				break;
			case tokUGEQ:
				// Change left >= const to left > const-1, but const-1 must be >=0u
				if (m)
				{
					stack[oldIdxRight][1]--;
					tok = tokUGreater;
				}
				break;
			}
		}
		stack[oldIdxRight + 1][0] = tok;
		break;

	case ')':
		while (stack[*idx][0] != '(')
		{
			GenPrep(idx);
			if (stack[*idx][0] == ',')
				--*idx;
		}
		--*idx;
		break;

	default:
		//error("GenPrep: unexpected token %s\n", GetTokenName(tok));
		errorInternal(101);
	}
}

STATIC
void GenCmp(int* idx, int instr)
{
	// constness: 0 = zero const, 1 = non-zero const, 2 = non-const
	int constness = (stack[*idx - 1][0] == tokNumInt) ? (stack[*idx - 1][1] != 0) : 2;
	int constval = (constness == 1) ? truncInt(stack[*idx - 1][1]) : 0;
	// condbranch: 0 = no conditional branch, 1 = branch if true, 2 = branch if false
	int condbranch = (*idx + 1 < sp) ? (stack[*idx + 1][0] == tokIf) + (stack[*idx + 1][0] == tokIfNot) * 2 : 0;
	int label = condbranch ? stack[*idx + 1][1] : 0;

	GenStartCommentLine(); printf2("GenCmp const=%d cond=%d\n",constness,condbranch);

	uint32_t sig = instr & 0x10000 ? 1 : 0;
	uint32_t inv = instr & 0x20000 ? 1 : 0;
	uint32_t msk = 0;
	
	instr &= 0xffff;
	
	switch(instr) {
		case teocInstrCe : msk = 4; break;
		case teocInstrCl : msk = 2; break;
		case teocInstrCg : msk = 1; break;
		case teocInstrCel : msk = 6; break;
		case teocInstrCeg : msk = 5; break;
	}

	if (constness == 2)
		GenPopReg();

	if (condbranch)
	{
		if (constness == 2) {
			GenPrintInstrOneOperand(teocInstrLr, 0, GenRreg, 0);
			GenPrintInstrOneOperand(teocInstrLr, 0, GenLreg, 0);
			GenPrintInstrNoOperand(sig ? teocInstrCmp : teocInstrCmpi, 0);
		} else {
			GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, constval);
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrNoOperand(sig ? teocInstrCmp : teocInstrCmpi, 0);
		}
		
		GenPrintInstrNoOperand(instr, 0);
		
		if((condbranch != 1 ? 1 : 0) ^ inv) {
			if(instr != teocInstrCel && instr != teocInstrCeg) {
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, msk);
				GenPrintInstrNoOperand(teocInstrXor, 0);
			} else {
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);
				
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 2);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrNoOperand(teocInstrLrs, 0);
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
				GenPrintInstrNoOperand(teocInstrAnd, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, instr == teocInstrCeg ? 0 : 1);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_B, 0);
				GenPrintInstrNoOperand(teocInstrLrs, 0);
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
				GenPrintInstrNoOperand(teocInstrAnd, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);
				
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_B, 0);
				GenPrintInstrNoOperand(teocInstrOr, 0);
				
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
				GenPrintInstrNoOperand(teocInstrXor, 0);
				
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				
				
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 2);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrNoOperand(teocInstrLls, 0);
			}
			//GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, msk);
			//GenPrintInstrNoOperand(teocInstrXor, 0);
		}
		
		GenPrintInstrOneOperand(teocInstrLi, 0, teocOpNumLabel, label);
		GenPrintInstrNoOperand(teocInstrCJmp, 0);
	}
	else
	{
		// Slow, general, catch-all implementation
		if (constness == 2) {
			GenPrintInstrOneOperand(teocInstrLr, 0, GenRreg, 0);
			GenPrintInstrOneOperand(teocInstrLr, 0, GenLreg, 0);
			GenPrintInstrNoOperand(sig ? teocInstrCmp : teocInstrCmpi, 0);
		} else {
			GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, constval);
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrNoOperand(sig ? teocInstrCmp : teocInstrCmpi, 0);
		}
		
		GenPrintInstrNoOperand(instr, 0);
		
		if(inv) {
			if(instr != teocInstrCel && instr != teocInstrCeg) {
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, msk);
				GenPrintInstrNoOperand(teocInstrXor, 0);
			} else {
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);
				
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 2);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrNoOperand(teocInstrLrs, 0);
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
				GenPrintInstrNoOperand(teocInstrAnd, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, instr == teocInstrCeg ? 0 : 1);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_B, 0);
				GenPrintInstrNoOperand(teocInstrLrs, 0);
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
				GenPrintInstrNoOperand(teocInstrAnd, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);
				
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_B, 0);
				GenPrintInstrNoOperand(teocInstrOr, 0);
				
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 1);
				GenPrintInstrNoOperand(teocInstrXor, 0);
				
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				
				
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 2);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrNoOperand(teocInstrLls, 0);
			}
		}
		
		GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
		
		//GenPrintInstrOneOperand(teocInstrLi, 0, teocOpNumLabel, label);
		//GenPrintInstrNoOperand(teocInstrCJmp, 0);
	}

	*idx += condbranch != 0;
}

STATIC
int GenIsCmp(int t)
{
	return
		t == '<' ||
		t == '>' ||
		t == tokGEQ ||
		t == tokLEQ ||
		t == tokULess ||
		t == tokUGreater ||
		t == tokUGEQ ||
		t == tokULEQ ||
		t == tokEQ ||
		t == tokNEQ;
}

// Improved register/stack-based code generator
// DONE: test 32-bit code generation
STATIC
void GenExpr0(void)
{
	int i;
	int gotUnary = 0;
	int maxCallDepth = 0;
	int callDepth = 0;
	int t = sp - 1;

	if (stack[t][0] == tokIf || stack[t][0] == tokIfNot || stack[t][0] == tokReturn)
		t--;
	GenPrep(&t);

	for (i = 0; i < sp; i++)
		if (stack[i][0] == '(')
		{
			if (++callDepth > maxCallDepth)
				maxCallDepth = callDepth;
		}
		else if (stack[i][0] == ')')
		{
			callDepth--;
		}

	CanUseTempRegs = maxCallDepth == 0;
	TempsUsed = 0;
	if (GenWreg != teocOpReg0)
		errorInternal(102);

	for (i = 0; i < sp; i++)
	{
		int tok = stack[i][0];
		int v = stack[i][1];

#ifndef NO_ANNOTATIONS
		switch (tok)
		{
		case tokNumInt: GenStartCommentLine(); printf2("%d\n", truncInt(v)); break;
		//case tokNumUint: printf2(" ; %uu\n", truncUint(v)); break;
		case tokIdent: case tokRevIdent: GenStartCommentLine(); printf2("%s\n", IdentTable + v); break;
		case tokLocalOfs: case tokRevLocalOfs: GenStartCommentLine(); printf2("local ofs\n"); break;
		case ')': GenStartCommentLine(); printf2(") fxn call\n"); break;
		case tokUnaryStar: GenStartCommentLine(); printf2("* (read dereference)\n"); break;
		case '=': GenStartCommentLine(); printf2("= (write dereference)\n"); break;
		case tokShortCirc: GenStartCommentLine(); printf2("short-circuit "); break;
		case tokGoto: GenStartCommentLine(); printf2("sh-circ-goto "); break;
		case tokLogAnd: GenStartCommentLine(); printf2("short-circuit && target\n"); break;
		case tokLogOr: GenStartCommentLine(); printf2("short-circuit || target\n"); break;
		case tokIf: GenStartCommentLine(); printf2(__FUNCTION__ ); printf2(" if\n"); break;
		case tokIfNot: GenStartCommentLine(); printf2(__FUNCTION__ ); printf2(" !if\n"); break;
		case tokReturn: break;
		case tokRevMinus: GenStartCommentLine(); printf2("-r\n"); break;
		default: GenStartCommentLine(); printf2("%s\n", GetTokenName(tok)); break;
		}
#endif

		switch (tok)
		{
		// TBD??? forward tokNumInt and tokIdent to ',', push them directly, w/o extra moves
		case tokNumInt:
			GenStartCommentLine(); printf2("(NumInt)\n");
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == '+' ||
													 t == '-' ||
													 t == tokRevMinus ||
													 t == '*' ||
													 t == '&' ||
													 t == '^' ||
													 t == '|' ||
													 t == tokLShift ||
													 t == tokRShift ||
													 t == tokURShift ||
													 t == '/' ||
													 t == tokUDiv ||
													 t == '%' ||
													 t == tokUMod ||
													 GenIsCmp(t))))
			{
				if (gotUnary)
					GenPushReg();

				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, v);
				GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, v);
			}
			gotUnary = 1;
			break;

		case tokIdent:
			if (gotUnary)
				GenPushReg();
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == ')' ||
													 t == tokUnaryStar ||
													 t == tokInc ||
													 t == tokDec ||
													 t == tokPostInc ||
													 t == tokPostDec)))
			{
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpLabel, v);
				GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, v);
			}
			gotUnary = 1;
			break;

		case tokLocalOfs:
			if (gotUnary)
				GenPushReg();
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == tokUnaryStar ||
													 t == tokInc ||
													 t == tokDec ||
													 t == tokPostInc ||
													 t == tokPostDec)))
			{
				GenPrintInstrOneOperand(teocInstrLr, 0, teocOpRegBp, 0);
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, v);
				GenPrintInstrNoOperand(teocInstrAdd, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, v);
			}
			gotUnary = 1;
			break;

		case '(':
			if (gotUnary)
				GenPushReg();
			gotUnary = 0;
			break;

		case ',':
			break;

		case ')':
			if (stack[i - 1][0] == tokIdent)
			{
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpLabel, stack[i - 1][1]);
				GenPrintInstrNoOperand(teocInstrCall, 0);
			}
			else
			{
				GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
				GenPrintInstrNoOperand(teocInstrCall, 0);
			}
			GenGrowStack(-v);
			break;

		case tokUnaryStar:
			if (stack[i - 1][0] == tokIdent)
				GenReadIdent(GenWreg, v, stack[i - 1][1]);
			else if (stack[i - 1][0] == tokLocalOfs)
				GenReadLocal(GenWreg, v, stack[i - 1][1]);
			else
				GenReadIndirect(GenWreg, GenWreg, v);
			break;

		case tokUnaryPlus:
			break;
		case '~':
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrNoOperand(teocInstrNot, 0);
			GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			break;
		case tokUnaryMinus:
			GenStartCommentLine(); printf2("(Unary)\n");
			GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0);
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrNoOperand(teocInstrSub, 0);
			GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			break;

		case '+':
		case '-':
		case tokRevMinus:
		case '*':
		case '&':
		case '^':
		case '|':
		case tokLShift:
		case tokRShift:
		case tokURShift:
		case '/':
		case tokUDiv:
		case '%':
		case tokUMod:
			GenStartCommentLine(); printf2("(Normal)\n");
			if (stack[i - 1][0] == tokNumInt)
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				//GenPopReg();
				GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
				GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, stack[i - 1][1]);
				GenPrintInstrNoOperand(instr, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			}
			else
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				GenPopReg();
				
				GenPrintInstrOneOperand(teocInstrLr, 0, GenRreg, 0);
				GenPrintInstrOneOperand(teocInstrLr, 0, GenLreg, 0);
				GenPrintInstrNoOperand(instr, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			}
			if (tok == '/' || tok == tokUDiv)
				GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			if (tok == '%' || tok == tokUMod)
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
			break;
		case tokInc:
		case tokDec:
			if (stack[i - 1][0] == tokIdent)
			{
				GenIncDecIdent(GenWreg, v, stack[i - 1][1], tok);
			}
			else if (stack[i - 1][0] == tokLocalOfs)
			{
				GenIncDecLocal(GenWreg, v, stack[i - 1][1], tok);
			}
			else
			{
				GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				GenIncDecIndirect(GenWreg, TEMP_REG_A, v, tok);
			}
			break;
		case tokPostInc:
		case tokPostDec:
			if (stack[i - 1][0] == tokIdent)
			{
				GenPostIncDecIdent(GenWreg, v, stack[i - 1][1], tok);
			}
			else if (stack[i - 1][0] == tokLocalOfs)
			{
				GenPostIncDecLocal(GenWreg, v, stack[i - 1][1], tok);
			}
			else
			{
				GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				GenPostIncDecIndirect(GenWreg, TEMP_REG_A, v, tok);
			}
			break;

		case tokPostAdd:
		case tokPostSub:
			GenStartCommentLine(); printf2("(Post)\n");
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				GenPopReg();
				if (GenWreg == GenLreg)
				{
					GenPrintInstrOneOperand(teocInstrLr, 0, GenLreg, 0);
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);

					GenReadIndirect(GenWreg, TEMP_REG_B, v);
					
					GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
					
					GenPrintInstrOneOperand(teocInstrLr, 0, GenRreg, 0);
					GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
					GenPrintInstrNoOperand(instr, 0);
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
					
					GenWriteIndirect(TEMP_REG_B, TEMP_REG_A, v);
				}
				else
				{
					// GenWreg == GenRreg here
					GenPrintInstrOneOperand(teocInstrLr, 0, GenRreg, 0);
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);
					
					GenReadIndirect(GenWreg, GenLreg, v);
					
					GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_B, 0);
					GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
					GenPrintInstrNoOperand(instr, 0);
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);
					
					GenWriteIndirect(GenLreg, TEMP_REG_B, v);
				}
			}
			break;

		case tokAssignAdd:
		case tokAssignSub:
		case tokAssignMul:
		case tokAssignAnd:
		case tokAssignXor:
		case tokAssignOr:
		case tokAssignLSh:
		case tokAssignRSh:
		case tokAssignURSh:
		case tokAssignDiv:
		case tokAssignUDiv:
		case tokAssignMod:
		case tokAssignUMod:
		//case '-':
		//case tokRevMinus:
			GenStartCommentLine(); printf2("(Assign)\n");
			if (stack[i - 1][0] == tokRevLocalOfs || stack[i - 1][0] == tokRevIdent)
			{
				int instr = GenGetBinaryOperatorInstr(tok);

				if (stack[i - 1][0] == tokRevLocalOfs)
					GenReadLocal(TEMP_REG_B, v, stack[i - 1][1]);
				else
					GenReadIdent(TEMP_REG_B, v, stack[i - 1][1]);
				
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_B, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_A, 0);
				
				GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
				GenPrintInstrOneOperand(teocInstrLr, 0, TEMP_REG_A, 0);
				GenPrintInstrNoOperand(instr, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
				
				if (tok == tokAssignDiv || tok == tokAssignUDiv)
					GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
				if (tok == tokAssignMod || tok == tokAssignUMod)
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B+1, 0);

				if (stack[i - 1][0] == tokRevLocalOfs)
					GenWriteLocal(GenWreg, v, stack[i - 1][1]);
				else
					GenWriteIdent(GenWreg, v, stack[i - 1][1]);
			}
			else
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				int lsaved, rsaved;
				GenPopReg();
				if (GenWreg == GenLreg)
				{
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);
					GenPrintInstrOneOperand(teocInstrLr, 0, GenLreg, 0);
					lsaved = TEMP_REG_B;
					rsaved = GenRreg;
				}
				else
				{
					// GenWreg == GenRreg here
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B, 0);
					GenPrintInstrOneOperand(teocInstrLr, 0, GenRreg, 0);
					rsaved = TEMP_REG_B;
					lsaved = GenLreg;
				}

				GenReadIndirect(GenWreg, GenLreg, v); // destroys either GenLreg or GenRreg because GenWreg coincides with one of them
				
				GenPrintInstrOneOperand(teocInstrLr, 0, rsaved, 0);
				GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
				GenPrintInstrNoOperand(instr, 0);
				GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);

				if (tok == tokAssignDiv || tok == tokAssignUDiv)
					GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
				if (tok == tokAssignMod || tok == tokAssignUMod)
					GenPrintInstrOneOperand(teocInstrSr, 0, TEMP_REG_B+1, 0);

				GenWriteIndirect(lsaved, GenWreg, v);
			}
			GenExtendRegIfNeeded(GenWreg, v);
			break;

		case '=':
			if (stack[i - 1][0] == tokRevLocalOfs)
			{
				GenStartCommentLine(); printf2("(local)\n");
				GenWriteLocal(GenWreg, v, stack[i - 1][1]);
			}
			else if (stack[i - 1][0] == tokRevIdent)
			{
				GenStartCommentLine(); printf2("(ident)\n");
				GenWriteIdent(GenWreg, v, stack[i - 1][1]);
			}
			else
			{
				GenStartCommentLine(); printf2("(other)\n");
				GenPopReg();
				GenWriteIndirect(GenLreg, GenRreg, v);
				if (GenWreg != GenRreg) {
					GenPrintInstrOneOperand(teocInstrLr, 0, GenRreg, 0);
					GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
				}
			}
			GenExtendRegIfNeeded(GenWreg, v);
			break;

		case '<':				GenCmp(&i, teocInstrCl); break;
		case tokLEQ:			GenCmp(&i, teocInstrCel); break;
		case '>':				GenCmp(&i, teocInstrCg); break;
		case tokGEQ:			GenCmp(&i, teocInstrCeg); break;
		case tokULess:			GenCmp(&i, teocInstrCl | 0x10000); break;
		case tokULEQ:			GenCmp(&i, teocInstrCel | 0x10000); break;
		case tokUGreater:		GenCmp(&i, teocInstrCg | 0x10000); break;
		case tokUGEQ:			GenCmp(&i, teocInstrCeg | 0x10000); break;
		case tokEQ:				GenCmp(&i, teocInstrCe | 0x10000); break;
		case tokNEQ:			GenCmp(&i, teocInstrCe | 0x10000 | 0x20000); break;

		case tok_Bool:
			GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 2);
			GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 4);
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrNoOperand(teocInstrTest, 0);
			GenPrintInstrNoOperand(teocInstrCe, 0);
			GenPrintInstrNoOperand(teocInstrXor, 0);
			GenPrintInstrNoOperand(teocInstrLrs, 0);
			GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			break;

		case tokSChar:
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrNoOperand(teocInstrSxB, 0);
			GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			break;
		case tokUChar:
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0xff);
			GenPrintInstrNoOperand(teocInstrAnd, 0);
			GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			break;
		case tokShort:
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrNoOperand(teocInstrSxW, 0);
			GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			break;
		case tokUShort:
			GenPrintInstrOneOperand(teocInstrLi, 0, teocOpConst, 0xffff);
			GenPrintInstrOneOperand(teocInstrLr, 0, GenWreg, 0);
			GenPrintInstrNoOperand(teocInstrAnd, 0);
			GenPrintInstrOneOperand(teocInstrSr, 0, GenWreg, 0);
			break;

		case tokShortCirc:
#ifndef NO_ANNOTATIONS
			if (v >= 0)
				printf2("&&\n");
			else
				printf2("||\n");
#endif
			if (v >= 0) {
				printf2("\t; tokShortCirc +\n");
				GenJumpIfZero(v); // &&
			} else {
				printf2("\t; tokShortCirc -\n");
				GenJumpIfNotZero(-v); // ||
			}
			gotUnary = 0;
			break;
		case tokGoto:
#ifndef NO_ANNOTATIONS
			printf2("goto\n");
#endif
			GenJumpUncond(v);
			gotUnary = 0;
			break;
		case tokLogAnd:
			printf2("\t; tokLogAnd\n");
			GenNumLabel(v);
			break;
		case tokLogOr:
			GenNumLabel(v);
			break;

		case tokVoid:
			gotUnary = 0;
			break;

		case tokRevIdent:
		case tokRevLocalOfs:
		case tokComma:
			break;
		case tokReturn:
			printf2("\t; tokReturn\n");
			break;

		case tokIf:
			printf2("\t; tokIf\n");
			GenJumpIfNotZero(stack[i][1]);
			break;
		case tokIfNot:
			printf2("\t; tokIfNot\n");
			GenJumpIfZero(stack[i][1]);
			break;

		default:
			//error("Error: Internal Error: GenExpr0(): unexpected token %s\n", GetTokenName(tok));
			errorInternal(103);
			break;
		}
	}

	if (GenWreg != teocOpReg0)
		printf("GenWreg=%d\n",GenWreg), errorInternal(104);
}

STATIC
void GenDumpChar(int ch)
{
	if(ch > 0) printf2("\tdb #%u\n", ch & 0xFFu);
}

STATIC
void GenExpr(void)
{
	if (GenExterns)
	{
		int i;
		for (i = 0; i < sp; i++)
			if (stack[i][0] == tokIdent && !isdigit(IdentTable[stack[i][1]]))
			GenAddGlobal(IdentTable + stack[i][1], 2);
	}
	GenExpr0();
}

STATIC
void GenFin(void)
{
	if (StructCpyLabel)
	{
		int lbl = LabelCnt++;

		puts2(CodeHeaderFooter[0]);

		GenNumLabel(StructCpyLabel);
		puts2("\tpush(%r1)\n"
					"\tpush(%r2)\n"
					"\tpush(%r3)\n"
					"\tmov(%r1,d@[%sp:$16])\n" // size
					"\tmov(%r2,d@[%sp:$20])\n" // source
					"\tmov(%r3,d@[%sp:$24])"); // destination
		GenNumLabel(lbl);
		puts2("\tmov(%r0,b%r2)\n"
					"\tadd(%r2,$1)\n"
					"\tadd(%r1,$-1)\n"
					"\tmov(b%r3,%r0)\n"
					"\tadd(%r3,$1)\n"
					"\tcmp(%r1,$0)");
		printf2("\tjne("); GenPrintNumLabel(lbl);
		puts2(")");
		puts2("\tmov(%r0,d@[%sp:$24])\n" // destination
					"\tpop(%r3)\n"
					"\tpop(%r2)\n"
					"\tpop(%r1)\n"
					"\tret()");

		puts2(CodeHeaderFooter[1]);
	}
	
	if (GenExterns)
	{
		int i = 0;

		puts2("");
		while (i < GlobalsTableLen)
		{
			if (GlobalsTable[i] == 2)
			{
				//printf2("\textern\t");
				//GenPrintLabel(GlobalsTable + i + 2);
				printf2("\tglobal ._%s\n", GlobalsTable + i + 2);
				//puts2("");
			}
			i += GlobalsTable[i + 1] + 2;
		}
	}
}
