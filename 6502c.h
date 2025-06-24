#ifndef C6502c_H
#define C6502c_H
//==================================================================
#define DEBUG_CPU //ON,OFF 
//==================================================================
#define logindex 0
//==================================================================
#ifdef  DEBUG_CPU
#define cpu_print(fmt, ...) { \
  printf(fmt, __VA_ARGS__); \
  log( logindex,  fmt, __VA_ARGS__); \
}
#endif 
//==================================================================
#define _STOP_ 0
#define _RUN_  1
#define ENABLE  1
#define DISABLE 0
//==================================================================
//6502 MEMORY MAP
//==================================================================
//(MAP) For RAM and ROM ,...A0-A15
//Example
#define _KB 1024
#define RAM_ADDRESS  0
#define RAM_SIZE     32 * _KB
#define ROM_ADDRESS  RAM_SIZE
#define RAM_SIZE     32 * _KB
//==================================================================
#define EMPTY_FF   0xFF
#define EMPTY_00   0x00
//==================================================================
#define STACK_BASE 0x0100  // STACK_BASE
//==================================================================
#define IMM 0
#define ZP0 1
#define ZPX 2
#define ZPY 3
#define ABS 4
#define ABX 5
#define ABY 6
#define IND 7
#define IZX 8
#define IZY 9
#define REL 10
#define IMP 11
#define ACC 12
#define XxX 13
//==================================================================
#define Addressing_Mode_String(mode) \
   ((mode) == 0 ? "IMM" : \
    (mode) == 1 ? "ZP0" : \
    (mode) == 2 ? "ZPX" : \
    (mode) == 3 ? "ZPY" : \
    (mode) == 4 ? "ABS" : \
    (mode) == 5 ? "ABX" : \
    (mode) == 6 ? "ABY" : \
    (mode) == 7 ? "IND" : \
    (mode) == 8 ? "IZX" : \
    (mode) == 9 ? "IZY" : \
    (mode) == 10 ? "REL": \
    (mode) == 11 ? "IMP": \
    (mode) == 12 ? "ACC": \
    (mode) == 13 ? "XxX": "INVALID")
//==================================================================
#define get_Status(cpu) ( \
    ((cpu)->Carry        << 0)  | \
    ((cpu)->Zero         << 1)  | \
    ((cpu)->Interrupt    << 2)  | \
    ((cpu)->DecimalMode  << 3)  | \
/*(cpu)->BreakCommand*/(0<< 4)  | \
/*(cpu)->Reserved*/    (1<< 5)  | \
    ((cpu)->Overflow     << 6)  | \
    ((cpu)->Negative     << 7))
//==================================================================
#define set_Status(cpu, value)     \
    (cpu)->Carry        = ((value) >> 0) & 1; \
    (cpu)->Zero         = ((value) >> 1) & 1; \
    (cpu)->Interrupt    = ((value) >> 2) & 1; \
    (cpu)->DecimalMode  = ((value) >> 3) & 1; \
    (cpu)->BreakCommand =                  0; \
    (cpu)->Reserved     =                  1; \
    (cpu)->Overflow     = ((value) >> 6) & 1; \
    (cpu)->Negative     = ((value) >> 7) & 1;  
//==================================================================
typedef union
{
	struct {
		// -----------------------
		// CPU Emulation & Commands...
		// -----------------------
		unsigned short Address;
		unsigned char  Data;
		unsigned char  Mode;
		unsigned char  Opcode;
		   const char *OpcodeString;
		unsigned char  OpcodeStep;
		unsigned short OldPC;
		// Cycle
		unsigned char Opcode_Cycle;
		unsigned long long Cycle;
		//IRQ & NMI
		unsigned char  NMI_Enable;
		unsigned char  IRQ_Enable;
		// Registers 8bit
		unsigned char Accumulator;
		unsigned char X;
		unsigned char Y;
		// Program Counter 16bit
		unsigned short PC;
		// Stack
		unsigned char SP;
		// Processor Status Flags (bit field)
		unsigned char Carry        : 1;
		unsigned char Zero         : 1;
		unsigned char Interrupt    : 1;
		unsigned char DecimalMode  : 1;
		unsigned char BreakCommand : 1;
		unsigned char Reserved     : 1;
		unsigned char Overflow     : 1;
		unsigned char Negative     : 1;

		unsigned char Memory[0x10000]; //Best Thing: Make a Full Memory For 16-bit Address (A0–A15)
	};
} Cpu6502;
//==================================================================
//                              Logical
//ORA,AND,EOR,BIT,ADC,SBC,CMP,CPY
//==================================================================
static inline void ORA(Cpu6502 *cpu);
static inline void AND(Cpu6502 *cpu);
static inline void EOR(Cpu6502 *cpu);
static inline void BIT(Cpu6502 *cpu);
//==================================================================
//                            Arithmetic
//ADC,SBC,CMP,CPY
//==================================================================
static inline void ADC(Cpu6502 *cpu);
static inline void SBC(Cpu6502 *cpu);
static inline void CMP(Cpu6502 *cpu);
static inline void CPY(Cpu6502 *cpu);
static inline void CPX(Cpu6502* cpu);
//==================================================================
//                       Increments & Decrements
//INC,INX,INY,DEC,DEX,DEY
//==================================================================
static inline void INC(Cpu6502 *cpu);
static inline void INX(Cpu6502 *cpu);
static inline void INY(Cpu6502 *cpu);
static inline void DEC(Cpu6502 *cpu);
static inline void DEX(Cpu6502 *cpu);
static inline void DEY(Cpu6502 *cpu);
//==================================================================
//                             Shifts
//ASL,LSR,ROL,ROR
//==================================================================
static inline void ASL(Cpu6502 *cpu);
static inline void LSR(Cpu6502 *cpu);
static inline void ROL(Cpu6502 *cpu);
static inline void ROR(Cpu6502 *cpu);
//==================================================================
//                          Jumps & Calls
//JMP,JSR,RTS 
//==================================================================
static inline void JMP(Cpu6502 *cpu);
static inline void JSR(Cpu6502 *cpu);
static inline void RTS(Cpu6502 *cpu);
//==================================================================
//                         Branches
//BCC,BCS,BEQ,BMI,BNE,BPL,BVC,BVS,
//==================================================================
static inline void BCC(Cpu6502 *cpu);
static inline void BCS(Cpu6502 *cpu);
static inline void BEQ(Cpu6502 *cpu);
static inline void BMI(Cpu6502 *cpu);
static inline void BNE(Cpu6502 *cpu);
static inline void BPL(Cpu6502 *cpu);
static inline void BVC(Cpu6502 *cpu);
static inline void BVS(Cpu6502 *cpu);
//==================================================================
//                         Status Flag Changes
//CLC,SEC,CLI,SEI,CLV,CLD,SED
//==================================================================
static inline void CLC(Cpu6502 *cpu);
static inline void SEC(Cpu6502 *cpu);
static inline void CLI(Cpu6502 *cpu);
static inline void SEI(Cpu6502 *cpu);
static inline void CLV(Cpu6502 *cpu);
static inline void CLD(Cpu6502 *cpu);
static inline void SED(Cpu6502 *cpu);
//==================================================================
//                          System Functions
//BRK,NOP,RTI 
//==================================================================
static inline void BRK(Cpu6502 *cpu);
static inline void NOP(Cpu6502 *cpu);
static inline void RTI(Cpu6502 *cpu);
//==================================================================
//                         Register Transfers
//TXA,TYA,TAY,TAX 
//==================================================================
static inline void TXA(Cpu6502 *cpu);
static inline void TYA(Cpu6502 *cpu);
static inline void TAY(Cpu6502 *cpu);
static inline void TAX(Cpu6502 *cpu);
//==================================================================
//                        Stack Operations
//PHP,PLP,PHA,PLA,TXS,TSX
//==================================================================
static inline void PHP(Cpu6502 *cpu);
static inline void PLP(Cpu6502 *cpu);
static inline void PHA(Cpu6502 *cpu);
static inline void PLA(Cpu6502 *cpu);
static inline void TXS(Cpu6502 *cpu);
static inline void TSX(Cpu6502 *cpu);
//==================================================================
//                        Load/Store Operations
//LDA,LDX,LDY,STA,STX,STY,
//==================================================================
static inline void LDA(Cpu6502 *cpu);
static inline void LDX(Cpu6502 *cpu);
static inline void LDY(Cpu6502 *cpu);
static inline void STA(Cpu6502 *cpu);
static inline void STX(Cpu6502 *cpu);
static inline void STY(Cpu6502 *cpu);
//==================================================================
//                            Unofficial OpCodes
//==================================================================
static inline void LAX(Cpu6502* cpu);
static inline void SAX(Cpu6502* cpu);
static inline void DCP(Cpu6502* cpu);
static inline void ISB(Cpu6502* cpu);
static inline void SLO(Cpu6502* cpu);
static inline void RLA(Cpu6502* cpu);
static inline void SRE(Cpu6502* cpu);
static inline void RRA(Cpu6502* cpu);
//==================================================================
static inline void XXX(Cpu6502* cpu);
//==================================================================
static inline void Addressing_Mode(Cpu6502 *cpu);
static inline int Execute_Opcode(Cpu6502 *cpu);
//==================================================================
unsigned char Cpu_Map_Bus_Read(Cpu6502 *cpu, unsigned short address);
void Cpu_Map_Bus_Write(Cpu6502 *cpu, unsigned short address, unsigned char data);
void Cpu_Data_Read(Cpu6502* cpu);
void Cpu_Data_Write(Cpu6502* cpu);
//==================================================================
void CPU_Reset(Cpu6502 *cpu);
void CPU_Init(Cpu6502 *cpu);
//==================================================================
void CPU_IRQ_DISABLE(Cpu6502 *cpu);
void CPU_IRQ_ENABLE(Cpu6502 *cpu);
void CPU_NMI_DISABLE(Cpu6502 *cpu);
void CPU_NMI_ENABLE(Cpu6502 *cpu);
//==================================================================
int CPU_Clock(Cpu6502 *cpu);
//==================================================================
#endif