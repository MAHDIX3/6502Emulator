//==================================================================
//6502 Processor Emulator (Ricoh 2A03 and 2A07...) Written By Mahdi Zakariyazadeh(2025) 
//Pluse Getting Help From The Internet!...
//Email : mahdi07860@gmail.com
//6502Emulator(Simple With High Accuracy) V1.0
//6502 8bit Data(Pin D0-D7) & 16bit Address(Pin A0-A15) 
//This is a simple 6502 CPU simulator used as the base for classic systems like the (Atari2600,Atari8-bit,AppleII,Nintendo8Bit System,Commodore64,AtariLynx,BBC Micro and ...)
//==================================================================
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "Tools.h"
#include "6502C.h"
#include "Instruction6502C.h"
//==================================================================
Cpu6502 cpu;
//==================================================================
#define CYCLE   //ON,OFF
#define BUG6502 //(only used by Indirect JMP) 
#define STOPOPCODE "BRK" //Stop Emulator ,...
//==================================================================
//PageCrossing (C=ABX=ABY=IZY=0)&(C=REL=1) Branch instructions 
//================================================================== 
#define PageCrossing(A,B,C) if((A&0xFF00)==(B&0xFF00)){cpu->Opcode_Cycle+=(C);}else{cpu->Opcode_Cycle+=(C+1);}
//==================================================================
//
//                    PART1:CPU Memory & STACK Read/Write
//
//==================================================================
//==================================================================
//                         Read_Operation1Byte
//==================================================================
#define Read_Operand1Byte(cpu) (Cpu_Map_Bus_Read(cpu ,cpu->PC+1)) 
//==================================================================
//                         Read_Operation2Byte
//==================================================================
#define Read_Operand2Byte(cpu) (Cpu_Map_Bus_Read(cpu ,cpu->PC+1) | (Cpu_Map_Bus_Read(cpu ,cpu->PC+2) << 8))
//==================================================================
//                          STACK PUSH Write
//==================================================================
#define Push(cpu , data) (Cpu_Map_Bus_Write(cpu,STACK_BASE+(cpu->SP--),data))
//==================================================================
//                          STACK PULL Read
//==================================================================
#define Pull(cpu) (Cpu_Map_Bus_Read(cpu,STACK_BASE+(++cpu->SP)))  
//==================================================================
//                         Cpu_Map_Bus_Read
//ORG 6502 Map Address
//RAM : 0000 - 3FFF
//I/O : 4000 - 7FFF
//ROM : 8000 - FFFF
//Other Devices Can be Mapped Here
//==================================================================
unsigned char Cpu_Map_Bus_Read(Cpu6502 *cpu, unsigned short address)
{
	return cpu->Memory[address];
}
//==================================================================
//                           Cpu_Map_Bus_Write
//ORG 6502 Map Address
//RAM : 0000 - 3FFF
//I/O : 4000 - 7FFF
//ROM : 8000 - FFFF
//Other Devices Can be Mapped Here
//==================================================================
void Cpu_Map_Bus_Write(Cpu6502 *cpu, unsigned short address, unsigned char data)
{
	cpu->Memory[address] = data;
}
//==================================================================
//
//                          PART2:CPU Opcodes
//
//==================================================================
//==================================================================
//                            Logical
//ORA  01 05 09 0D 11 15 19 1D
//AND  21 25 29 2D 31 35 39 3D
//EOR  41 45 49 4D 51 55 59 5D
//BIT  24 2C
//==================================================================
// AND (Logical AND with Accumulator)
static inline void AND(Cpu6502 *cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Accumulator &= cpu->Data;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
// EOR (Exclusive OR with Accumulator)
static inline void EOR(Cpu6502 *cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Accumulator ^= cpu->Data;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
// ORA (Logical OR with Accumulator)
static inline void ORA(Cpu6502 *cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Accumulator |= cpu->Data;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
// BIT (Test Bits)
static inline void BIT(Cpu6502 *cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Zero = ((cpu->Accumulator & cpu->Data) == 0);
	cpu->Negative = (cpu->Data >> 7) & 1;
	cpu->Overflow = (cpu->Data >> 6) & 1;
}
//==================================================================
//                            Arithmetic
//ADC  61 65 69 6D 71 75 79 7D
//SBC  E1 E5 E9 ED F1 F5 F9 FD
//CMP  C1 C5 C9 CD D1 D5 D9 DD
//CPY  C0 C4 CC
//CPX  E0 E4 EC
//==================================================================
//ADC (Add with Carry)
static inline void ADC(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	short value = cpu->Accumulator + cpu->Data + cpu->Carry;
	cpu->Carry = (value & 0x100);
	cpu->Overflow = (cpu->Accumulator ^ value) & (cpu->Data ^ value) & 0x80;;
	cpu->Accumulator = value & 0xFF;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
// SBC (Subtract with Borrow)
static inline void SBC(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	short value = cpu->Accumulator - cpu->Data - !cpu->Carry;
	cpu->Carry = !(value & 0x100);
	cpu->Overflow = (cpu->Accumulator ^ value) & (~cpu->Data ^ value) & 0x80;
	cpu->Accumulator = value & 0xFF;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
// CMP (Compare with Accumulator)
static inline void CMP(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	unsigned char value = cpu->Accumulator - cpu->Data;
	cpu->Carry = (cpu->Accumulator >= cpu->Data);
	cpu->Zero = (value == 0);
	cpu->Negative = (value >> 7) & 1;
}
//==================================================================
// CPX (Compare with X Register)
static inline void CPX(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	unsigned char value = cpu->X - cpu->Data;
	cpu->Carry = (cpu->X >= cpu->Data);
	cpu->Zero = (value == 0);
	cpu->Negative = (value >> 7) & 1;
}
//==================================================================
// CPY (Compare with Y Register)
static inline void CPY(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	unsigned char value = cpu->Y - cpu->Data;
	cpu->Carry = (cpu->Y >= cpu->Data);
	cpu->Zero = (value == 0);
	cpu->Negative = (value >> 7) & 1;
}
//==================================================================
//                        Increments & Decrements
//INC E6 EE F6 FE
//INX E8
//INY C8
//DEC C6 CE D6 DE
//DEX CA
//DEY 88
//==================================================================
// INC (Increment Memory)
static inline void INC(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Data++;
	cpu->Zero = (cpu->Data == 0);
	cpu->Negative = (cpu->Data >> 7) & 1;
	Cpu_Data_Write(cpu);
}
//==================================================================
// INX (Increment X Register)
static inline void INX(Cpu6502* cpu)
{
	cpu->X++;
	cpu->Zero = (cpu->X == 0);
	cpu->Negative = (cpu->X >> 7) & 1;
}
//==================================================================
// INY (Increment Y Register)
static inline void INY(Cpu6502* cpu)
{
	cpu->Y++;
	cpu->Zero = (cpu->Y == 0);
	cpu->Negative = (cpu->Y >> 7) & 1;
}
//==================================================================
// DEC (Decrement Memory)
static inline void DEC(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Data--;
	cpu->Zero = (cpu->Data == 0);
	cpu->Negative = (cpu->Data >> 7) & 1;
	Cpu_Data_Write(cpu);
}
//==================================================================
// DEX (Decrement X Register)
static inline void DEX(Cpu6502 *cpu)
{
	cpu->X--;
	cpu->Zero = (cpu->X == 0);
	cpu->Negative = (cpu->X >> 7) & 1;
}
//==================================================================
// DEY (Decrement Y Register)
static inline void DEY(Cpu6502* cpu)
{
	cpu->Y--;
	cpu->Zero = (cpu->Y == 0);
	cpu->Negative = (cpu->Y >> 7) & 1;
}
//==================================================================
//                             Shifts
//ASL 06 0A 0E 16 1E
//LSR 46 4A 4E 56 5E
//ROL  26 2A 2E 36 3E
//ROR 66 6A 6E 6A 6E 76
//==================================================================
// ASL (Arithmetic Shift Left)
static inline void ASL(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Carry = (cpu->Data >> 7) & 1;
	cpu->Data <<= 1;
	cpu->Zero = (cpu->Data == 0);
	cpu->Negative = (cpu->Data >> 7) & 1;
	Cpu_Data_Write(cpu);
}
//==================================================================
// LSR (Logical Shift Right)
static inline void LSR(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Carry = cpu->Data & 0x01;
	cpu->Data = (cpu->Data >> 1);
	cpu->Zero = (cpu->Data == 0);
	cpu->Negative = (cpu->Data >> 7) & 1;
	Cpu_Data_Write(cpu);
}
//==================================================================
// ROL (Rotate Left)
static inline void ROL(Cpu6502 *cpu)
{
	Cpu_Data_Read(cpu);
	unsigned char old_carry = cpu->Carry;
	cpu->Carry = (cpu->Data & 0x80);
	cpu->Data = (cpu->Data << 1) | old_carry;
	cpu->Zero = (cpu->Data == 0);
	cpu->Negative = (cpu->Data >> 7) & 1;
	Cpu_Data_Write(cpu);
}
//==================================================================
// ROR (Rotate Right)
static inline void ROR(Cpu6502 *cpu)
{
	Cpu_Data_Read(cpu);
	unsigned char old_carry = cpu->Carry << 7;
	cpu->Carry = cpu->Data & 0x01;
	cpu->Data = (cpu->Data >> 1) | old_carry;
	cpu->Zero = (cpu->Data == 0);
	cpu->Negative = (cpu->Data >> 7) & 1;
	Cpu_Data_Write(cpu);
}
//==================================================================
//                        Jumps & Calls
//JMP 4C 6C
//JSR 20
//RTS 60
//==================================================================
// JMP (Jump)
static inline void JMP(Cpu6502* cpu)
{
	cpu->PC = cpu->Address;
}
//==================================================================
//Jump to a subroutine
static inline void JSR(Cpu6502* cpu)
{
	unsigned short ReturnAddress = cpu->PC;
	Push(cpu, (ReturnAddress >> 8) & 0xFF);//low byte
	Push(cpu, ReturnAddress & 0xFF);       //high byte
	cpu->PC = cpu->Address;
}
//==================================================================
// RTS (Return from Subroutine)
static inline void RTS(Cpu6502 *cpu)
{
	unsigned char PCL = Pull(cpu);//sp++
	unsigned char PCH = Pull(cpu);//sp++
	cpu->PC = (PCL | (PCH << 8));
}
//==================================================================
//                         Branches
//BCC 90
//BCS B0
//BEQ F0
//BMI 30
//BNE D0
//BPL 10
//BVC 50
//BVS 70
//==================================================================
// BCC (Branch if Carry Clear)
static inline void BCC(Cpu6502* cpu)
{
	if (cpu->Carry == 0) {
		cpu->PC += (signed char)(cpu->Address);
		PageCrossing(cpu->OldPC, cpu->PC, 1);
	}
}
//==================================================================
// BCS (Branch if Carry Set)
static inline void BCS(Cpu6502* cpu)
{
	if (cpu->Carry == 1) {
		cpu->PC += (signed char)(cpu->Address);
		PageCrossing(cpu->OldPC, cpu->PC, 1);
	}
}
//==================================================================
// BEQ (Branch if Equal)
static inline void BEQ(Cpu6502* cpu)
{
	if (cpu->Zero == 1) {
		cpu->PC += (signed char)(cpu->Address);
		PageCrossing(cpu->OldPC, cpu->PC, 1);
	}
}
//==================================================================
// BMI (Branch if Minus)
static inline void BMI(Cpu6502* cpu)
{
	if (cpu->Negative == 1) {
		cpu->PC += (signed char)(cpu->Address);
		PageCrossing(cpu->OldPC, cpu->PC, 1);
	}
}
//==================================================================
// BNE (Branch if Not Equal)
static inline void BNE(Cpu6502* cpu)
{
	if (cpu->Zero == 0) {
		cpu->PC += (signed char)(cpu->Address);
		PageCrossing(cpu->OldPC, cpu->PC, 1);
	}
}
//==================================================================
// BPL (Branch if Positive)
static inline void BPL(Cpu6502* cpu)
{
	if (cpu->Negative == 0) {
		cpu->PC += (signed char)(cpu->Address);
		PageCrossing(cpu->OldPC, cpu->PC, 1);
	}
}
//==================================================================
//BVC Branch if overflow flag clea
static inline void BVC(Cpu6502* cpu)
{
	if (cpu->Overflow == 0) {
		cpu->PC += (signed char)(cpu->Address);
		PageCrossing(cpu->OldPC, cpu->PC, 1);
	}
}
//==================================================================
//BVS(Branch if Overflow Set)
static inline void BVS(Cpu6502* cpu)
{
	if (cpu->Overflow == 1) {
		cpu->PC += (signed char)(cpu->Address);
		PageCrossing(cpu->OldPC, cpu->PC, 1);
	}
}
//==================================================================
//                         Status Flag Changes
//CLC 18
//SEC 38
//CLI 58
//SEI 78
//CLV B8
//CLD D8
//SED F8
//==================================================================
// CLC (Clear Carry Flag)
static inline void CLC(Cpu6502* cpu)
{
	cpu->Carry = 0;
}
//==================================================================
// CLD (Clear Decimal Mode)
static inline void CLD(Cpu6502* cpu)
{
	cpu->DecimalMode = 0;
}
//==================================================================
// CLI (Clear Interrupt Disable)
static inline void CLI(Cpu6502* cpu)
{
	cpu->Interrupt = 0;
}
//==================================================================
// CLV (Clear Overflow Flag)
static inline void CLV(Cpu6502* cpu)
{
	cpu->Overflow = 0;
}
//==================================================================
// SEC (Set Carry Flag)
static inline void SEC(Cpu6502 *cpu)
{
	cpu->Carry = 1;
}
//==================================================================
// SED (Set Decimal Mode) 
static inline void SED(Cpu6502 *cpu)
{
	cpu->DecimalMode = 1;
}
//==================================================================
// SEI (Set Interrupt Disable)
static inline void SEI(Cpu6502 *cpu)
{
	cpu->Interrupt = 1;
}
//==================================================================
//                          System Functions
//BRK 00
//NOP EA
//RTI 40
//==================================================================
// BRK (Break)
static inline void BRK(Cpu6502* cpu)
{
	cpu->Interrupt = 1;

	unsigned short ReturnAddress = cpu->PC;

	Push(cpu, (ReturnAddress >> 8) & 0xFF);
	Push(cpu, ReturnAddress & 0xFF);

	unsigned char status = get_Status(cpu);
	Push(cpu, status);

	unsigned char PCL = Cpu_Map_Bus_Read(cpu, 0xFFFE);
	unsigned char PCH = Cpu_Map_Bus_Read(cpu, 0xFFFF);
	cpu->PC = PCL | (PCH << 8);
 
}
//==================================================================
// NOP (No Operation)
static inline void NOP(Cpu6502 *cpu)
{
	//...
}
//==================================================================
// RTI (Return from Interrupt)
static inline void RTI(Cpu6502 *cpu)
{
    unsigned char status = Pull(cpu);//sp++
	set_Status(cpu, status);

	unsigned char PCL = Pull(cpu);//sp++
	unsigned char PCH = Pull(cpu);//sp++
	cpu->PC = (PCL | (PCH << 8));
}
//==================================================================
//                         Register Transfers
//TXA 8A
//TYA 98
//TAY A8
//TAX AA
//==================================================================
// TAX (Transfer Accumulator to X)
static inline void TAX(Cpu6502 *cpu)
{
	cpu->X = cpu->Accumulator;
	cpu->Zero = (cpu->X == 0);
	cpu->Negative = (cpu->X >> 7) & 1;
}
//==================================================================
// TAY (Transfer Accumulator to Y)
static inline void TAY(Cpu6502 *cpu)
{
	cpu->Y = cpu->Accumulator;
	cpu->Zero = (cpu->Y == 0);
	cpu->Negative = (cpu->Y >> 7) & 1;
}
//==================================================================
// TXA (Transfer X to Accumulator)
static inline void TXA(Cpu6502 *cpu)
{
	cpu->Accumulator = cpu->X;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
// TYA (Transfer Y to Accumulator)
static inline void TYA(Cpu6502 *cpu)
{
	cpu->Accumulator = cpu->Y;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
//                        Stack Operations
//PHP 08
//PLP 28
//PHA 48
//PLA 68
//TXS 8A
//TSX BA
//==================================================================
// TSX (Transfer Stack Pointer to X)
static inline void TSX(Cpu6502 *cpu)
{
	cpu->X = cpu->SP;
	cpu->Zero = (cpu->X == 0);
	cpu->Negative = (cpu->X >> 7) & 1;
}
//==================================================================
// TXS (Transfer X to Stack Pointer)
static inline void TXS(Cpu6502 *cpu)
{
	cpu->SP = cpu->X;
}
//==================================================================
// PHA (Push Accumulator)
static inline void PHA(Cpu6502 *cpu)
{
	Push(cpu, cpu->Accumulator);
}
//==================================================================
// PHP (Push Processor Status)
static inline void PHP(Cpu6502 *cpu)
{
	unsigned char status = get_Status(cpu) | 0x30;
	Push(cpu, status);
}
//==================================================================
// PLA (Pull Accumulator)
static inline void PLA(Cpu6502 *cpu)
{
	cpu->Accumulator = Pull(cpu);
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
// PLP (Pull Processor Status
static inline void PLP(Cpu6502 *cpu)
{
	unsigned char status = Pull(cpu);
	set_Status(cpu, status);
}
//==================================================================
//                        Load/Store Operations
//LDA A1 A5 A9 AD B1 B5 BD
//LDX A2 A6 AE B6 BE
//LDY A0 A4 AC B4 BC
//STA 81 85 8D 91 95 99 9D
//STX 86 8E 96
//STY 84 8C 94
//==================================================================
// LDA (Load Accumulator)
static inline void LDA(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Accumulator = cpu->Data;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}
//==================================================================
// LDX (Load X Register)
static inline void LDX(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->X = cpu->Data;
	cpu->Zero = (cpu->X == 0);
	cpu->Negative = (cpu->X >> 7) & 1;
}
//==================================================================
// LDY (Load Y Register)
static inline void LDY(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Y = cpu->Data;
	cpu->Zero = (cpu->Y == 0);
	cpu->Negative = (cpu->Y >> 7) & 1;
}
//==================================================================
//STA Store Accumulator in Memory
static inline void STA(Cpu6502* cpu)
{
	Cpu_Data_Write(cpu);
}
//==================================================================
// STX (Store RegisterX in Memory)
static inline void STX(Cpu6502 *cpu)
{
	Cpu_Data_Write(cpu);
}
//==================================================================
// STY (Store RegisterY in Memory)
static inline void STY(Cpu6502 *cpu)
{
	Cpu_Data_Write(cpu);
}
//==================================================================
//                               ...
//==================================================================
static inline void XXX(Cpu6502* cpu)
{
	//...
}
//==================================================================
//==================================================================
//                        Unofficial Instructions 
//LAX A3 A7 AF B3 B7 BF
//SAX 83 87 8F 97
//DCP C3 C7 CF D3 D7 DB DF    
//ISB E3 E7 EF F3 F7 FB FF      
//SLO 03 07 17 13 0F 1F 1B 
//RLA 23 27 2F 33 37 3F 3B  
//SRE 43 47 4F 53 57 5F 5B  
//RRA 63 67 6F 73 77 7F 7B 
//==================================================================
//LAX (Load Accumulator & X) 
static inline void LAX(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Accumulator = cpu->Data;
	cpu->X = cpu->Data;
	cpu->Zero = (cpu->Data == 0);
	cpu->Negative = (cpu->Data  >> 7) & 1;
} 
//==================================================================
//SAX (Store A AND X) 
static inline void SAX(Cpu6502* cpu)
{
	cpu->Data = cpu->Accumulator & cpu->X;
	Cpu_Data_Write(cpu);
}  
//==================================================================
//DCP (Decrement then Compare)
static inline void DCP(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Data--;
	Cpu_Data_Write(cpu);
	cpu->Zero = (cpu->Data == 0);
	cpu->Negative = (cpu->Data >> 7) & 1;
	cpu->Carry = (cpu->Accumulator >= cpu->Data);
}
//==================================================================
//ISB (Increment then Subtract with Carry) 
static inline void ISB(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Data++;
	Cpu_Data_Write(cpu);
	unsigned short value = cpu->Accumulator - cpu->Data - !cpu->Carry;
	cpu->Carry = !(value & 0x100);
	cpu->Overflow = (cpu->Accumulator ^ value) & (~cpu->Data ^ value) & 0x80;
	cpu->Accumulator = value & 0xFF;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}  
//==================================================================
//SLO (Shift Left then OR) 
static inline void SLO(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Carry = (cpu->Data & 0x80);
	cpu->Data <<= 1;
	Cpu_Data_Write(cpu);
	cpu->Accumulator |= cpu->Data;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}  
 //==================================================================
 //RLA (Rotate Left then AND)
static inline void RLA(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	unsigned char old_carry = cpu->Carry;
	cpu->Carry = cpu->Data & 0x80;
	cpu->Data = (cpu->Data << 1) | old_carry;
	Cpu_Data_Write(cpu);
	cpu->Accumulator &= cpu->Data;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}  
//==================================================================
//SRE (Shift Right then EOR)
static inline void SRE(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	cpu->Carry = cpu->Data & 0x01;
	cpu->Data = (cpu->Data >> 1) & 0x7F;
	Cpu_Data_Write(cpu);
	cpu->Accumulator ^= cpu->Data;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
} 
//==================================================================
//RRA (Rotate Right then Add with Carry) 
static inline void RRA(Cpu6502* cpu)
{
	Cpu_Data_Read(cpu);
	unsigned char old_carry = cpu->Carry << 7;
	cpu->Carry = cpu->Data & 0x01;
	unsigned char valueA = old_carry | (cpu->Data >> 1);
	Cpu_Data_Write(cpu);
	unsigned char valueB = cpu->Accumulator + valueA + cpu->Carry;
	cpu->Carry = (valueB & 0x100);
	cpu->Overflow = (cpu->Accumulator ^ valueB) & (valueA ^ valueB) & 0x80;
	cpu->Accumulator = valueB;
	cpu->Zero = (cpu->Accumulator == 0);
	cpu->Negative = (cpu->Accumulator >> 7) & 1;
}

//==================================================================
//                          CPU_IRQ
//==================================================================
void CPU_IRQ(Cpu6502 *cpu)
{
	if (cpu->Interrupt == 1) return;

	unsigned short ReturnAddress = cpu->PC;

#ifdef DEBUG_CPU
	cpu_print("CPU_IRQ->ReturnAddress[%04X]\n", ReturnAddress);
#endif

	// Push PC onto stack (high then low)
	Push(cpu, (ReturnAddress >> 8) & 0xFF);
	Push(cpu, (ReturnAddress & 0xFF));

	unsigned char status = get_Status(cpu) | 0x30;

	Push(cpu, status);

	unsigned char PCL = Cpu_Map_Bus_Read(cpu, 0xFFFE);
	unsigned char PCH = Cpu_Map_Bus_Read(cpu, 0xFFFF);
	cpu->PC = PCL | (PCH << 8);

#ifdef DEBUG_CPU
	cpu_print("CPU_IRQ->NextAddress[%04X]\n", cpu->PC);
#endif

	cpu->Cycle += 7;
}

//==================================================================
//                          CPU_NMI
//==================================================================
void CPU_NMI(Cpu6502 *cpu)
{
	unsigned short ReturnAddress = cpu->PC;

#ifdef DEBUG_CPU
	cpu_print("CPU_NMI->ReturnAddress[%04X]\n", ReturnAddress);
#endif

	// Push PC onto stack (high byte then low byte)
	Push(cpu, (ReturnAddress >> 8) & 0xFF);
	Push(cpu, (ReturnAddress & 0xFF));

	unsigned char status = get_Status(cpu) | 0x30;

	Push(cpu, status);

	unsigned char PCL = Cpu_Map_Bus_Read(cpu, 0xFFFA);
	unsigned char PCH = Cpu_Map_Bus_Read(cpu, 0xFFFB);
	cpu->PC = PCL | (PCH << 8);

#ifdef DEBUG_CPU
	cpu_print("CPU_NMI->NextAddress[%04X]\n", cpu->PC);
#endif
	cpu->Cycle += 8;
}
//==================================================================
void CPU_IRQ_DISABLE(Cpu6502 *cpu)
{
	cpu->Interrupt = 1;
	cpu->IRQ_Enable = DISABLE;
}
//==================================================================
void CPU_IRQ_ENABLE(Cpu6502 *cpu)
{
	cpu->Interrupt = 0;
	cpu->IRQ_Enable = ENABLE;
}
//==================================================================
void CPU_NMI_DISABLE(Cpu6502 *cpu)
{
	cpu->Interrupt = 1;
	cpu->NMI_Enable = DISABLE;
}
//==================================================================
void CPU_NMI_ENABLE(Cpu6502 *cpu)
{
	cpu->Interrupt = 0;
	cpu->NMI_Enable = ENABLE;
}
//==================================================================
//
//           PART3:Data Write/Read and Addressing Mode
//
//==================================================================
//==================================================================
//                            Cpu_Data_Write
//==================================================================
void Cpu_Data_Write(Cpu6502* cpu)
{
	//===============================
	//        Register Write
	//===============================
	if (cpu->Mode == ACC || cpu->Mode == IMP)  //ACC(ASL,ROL,LSR,ROR) | IMP(SBC)
	{
		cpu->Accumulator = cpu->Data;
		return;
	}

	//===============================
	//      Memory & Register Write
	//===============================
	if (cpu->OpcodeString == "STA")
	{
		Cpu_Map_Bus_Write(cpu, cpu->Address, cpu->Accumulator);
		return;
	}
	if (cpu->OpcodeString == "STX")
	{
		Cpu_Map_Bus_Write(cpu, cpu->Address, cpu->X);
		return;
	}
	if (cpu->OpcodeString == "STY")
	{
		Cpu_Map_Bus_Write(cpu, cpu->Address, cpu->Y);
		return;
	}

	//===============================
	//          Memory Write
	//===============================
	Cpu_Map_Bus_Write(cpu, cpu->Address, cpu->Data);
}

//==================================================================
//                           Cpu_Data_Read
//==================================================================
void Cpu_Data_Read(Cpu6502* cpu)
{
	//===============================
	//          Register Read
	//===============================
	if (cpu->Mode == ACC || cpu->Mode == IMP)  //ACC(ASL,ROL,LSR,ROR) | IMP(SBC)
	{
		cpu->Data = cpu->Accumulator;
		return;
	}
	//===============================
	//          Memory Read
	//===============================
	cpu->Data = Cpu_Map_Bus_Read(cpu, cpu->Address);
}

//==================================================================
//               Addressing Mode ,Return *OpcodeStep*  
//==================================================================
static inline void Addressing_Mode(Cpu6502 *cpu)
{
	unsigned char low, high;
	unsigned short Operand_address;

	cpu->Mode = Find_AddressMode(cpu);
	cpu->OldPC = cpu->PC;
	switch (cpu->Mode)
	{
	//===============================
	//         Zero Page
    //===============================
	case ZP0: // Zero Page
		cpu->Address = Read_Operand1Byte(cpu);
		cpu->OpcodeStep = 2;
		break;

	case ZPX: // Zero Page,X
		cpu->Address = Read_Operand1Byte(cpu);
		cpu->Address = (cpu->Address + cpu->X) & 0xFF;
		cpu->OpcodeStep = 2;
		break;

	case ZPY: // Zero Page,Y
		cpu->Address = Read_Operand1Byte(cpu);
		cpu->Address = (cpu->Address + cpu->Y) & 0xFF;
		cpu->OpcodeStep = 2;
		break;
	//===============================
	//         Absolute
	//===============================
	case ABS: // Absolute
		cpu->Address = Read_Operand2Byte(cpu);
		cpu->OpcodeStep = 3;
		break;

	case ABX: // Absolute,X
		Operand_address = Read_Operand2Byte(cpu);
		cpu->Address = Operand_address + cpu->X;
		PageCrossing(Operand_address, cpu->Address, 0);
		cpu->OpcodeStep = 3;
		break;

	case ABY: // Absolute,Y
		Operand_address = Read_Operand2Byte(cpu);
		cpu->Address = Operand_address + cpu->Y;
		PageCrossing(Operand_address, cpu->Address, 0);
		cpu->OpcodeStep = 3;
		break;

	//===============================
	//         Indirect
	//===============================
	case IND: // Indirect (only used by JMP)
		cpu->Address = Read_Operand2Byte(cpu);
		if ((cpu->Address & 0x00FF) == 0x00FF) {
			low = Cpu_Map_Bus_Read(cpu, cpu->Address);
			high = Cpu_Map_Bus_Read(cpu, cpu->Address & 0xFF00);
			cpu->Address = low | (high << 8);
		}
		else {
			low = Cpu_Map_Bus_Read(cpu, cpu->Address);
			high = Cpu_Map_Bus_Read(cpu, cpu->Address + 1);
			cpu->Address = low | (high << 8);
		}
		cpu->OpcodeStep = 3;
		break;

	case IZX: // IZX (Indirect,X)
		cpu->Address = Read_Operand1Byte(cpu);
		low = Cpu_Map_Bus_Read(cpu, (cpu->Address + cpu->X) & 0xFF);
		high = Cpu_Map_Bus_Read(cpu, (cpu->Address + cpu->X + 1) & 0xFF);
		cpu->Address = low | (high << 8);
		cpu->OpcodeStep = 2;
		break;

	case IZY: // IZY (Indirect),Y 
		cpu->Address = Read_Operand1Byte(cpu);
		low = Cpu_Map_Bus_Read(cpu, (cpu->Address & 0xFF));
		high = Cpu_Map_Bus_Read(cpu, (cpu->Address + 1) & 0xFF);
		Operand_address = (low | (high << 8));
		cpu->Address = Operand_address + cpu->Y;
		PageCrossing(Operand_address, cpu->Address, 0);
		cpu->OpcodeStep = 2;
		break;

	//===============================	
	//       REL ,IMM ,IMP ,ACC
	//===============================
	case REL: // Relative (for branch instructions)
		cpu->Address = Read_Operand1Byte(cpu);
		cpu->OpcodeStep = 2;
		break;

	case IMM: // Immediate
		cpu->Address = (cpu->PC + 1);
		cpu->OpcodeStep = 2;
		break;

	case IMP: // Implied
		cpu->Address = cpu->PC;
		cpu->OpcodeStep = 1;
		break;

	case ACC: // Accumulator
		cpu->OpcodeStep = 1;
		break;
	}
	cpu->PC += cpu->OpcodeStep;
}

//==================================================================
//
//                 PART4:Execute Opcode & Run...
//
//==================================================================
//==================================================================
//                        Execute Opcode  
//==================================================================
static inline int Execute_Opcode(Cpu6502 *cpu)
{
	//============================================================== 
	//  Opcode and Addressing mode and Run instruction Function
	//============================================================== 
	cpu->Opcode = Cpu_Map_Bus_Read(cpu, cpu->PC);
	cpu->OpcodeString = Find_OpcodeName(cpu);
	Addressing_Mode(cpu);
	Find_OpcodeFunction(cpu)(cpu);

#ifdef CYCLE
	//===============================
	//          Cycle Count
	//===============================
	cpu->Opcode_Cycle = Find_OpcodeCycle(cpu);
	cpu->Cycle = cpu->Cycle + cpu->Opcode_Cycle;
#endif

	//===============================
	//         Debug  Emulator
	//===============================
#ifdef DEBUG_CPU
	cpu_print("[0x%04X][", cpu->OldPC);
	for (int i = 0; i < cpu->OpcodeStep; i++){
		cpu_print(" %02X ", Cpu_Map_Bus_Read(cpu, cpu->OldPC + i));
	}
	if (cpu->OpcodeStep == 1) { cpu_print("        "); }
	if (cpu->OpcodeStep == 2) { cpu_print("    "); }
	cpu_print("]");
	cpu_print("[%s][%s]", Addressing_Mode_String(cpu->Mode), Find_OpcodeName(cpu));
	cpu_print(" A : %02X X : %02X Y : %02X ", cpu->Accumulator, cpu->X, cpu->Y);
	cpu_print(" C[%02X]Z[%02X]I[%02X]D[%02X]V[%02X]N[%02X]", \
			cpu->Carry, cpu->Zero, cpu->Interrupt, cpu->DecimalMode, \
		    cpu->Overflow, cpu->Negative);
    cpu_print("NEXTADDR[%04X]", cpu->PC);

#ifdef CYCLE
	 cpu_print("OPCYCLE[%04d]", cpu->Opcode_Cycle);
	cpu_print("CYCLE[%04d]"   , cpu->Cycle);
#endif
	cpu_print("\n");
#endif

	if (cpu->OpcodeString == STOPOPCODE)
	{
		return _STOP_;
	}
 
	return _RUN_;
}

//==================================================================
//                          CPU_Reset
//==================================================================
void CPU_Reset(Cpu6502 *cpu , unsigned short address)
{
	CPU_Init(cpu);
	//Set Program Counter
	cpu->PC = address;
}

//==================================================================
//                          CPU_Initialize
//==================================================================
void CPU_Init(Cpu6502 *cpu)
{
	//Initialize 
	memset(cpu , 0x00 , sizeof(Cpu6502));
	memset(cpu->Memory , EMPTY_00 , sizeof(cpu->Memory));
	//Stack Reset
	cpu->SP = 0xFD;
	//IRQ Disable
	cpu->Interrupt = 1;
}

//==================================================================
//                          CPU_Clock
//==================================================================
int CPU_Clock(Cpu6502 *cpu)
{
	//==============================================================
	//                        Interrupt Pins
	//==============================================================
	if (cpu->Interrupt == 0)
	{
		//=====================
		//   NMI_Enable
		//===================== 
		if (cpu->NMI_Enable == ENABLE)
		{
			CPU_NMI(cpu);
			cpu->NMI_Enable = DISABLE;
			cpu->Interrupt = 1;
			return 2;
		}
		//=====================
		//   IRQ_Enable
		//===================== 
		else if (cpu->IRQ_Enable == ENABLE)
		{
			CPU_IRQ(cpu);
			cpu->IRQ_Enable = DISABLE;
			cpu->Interrupt = 1;
			return 3;
		}
	}
	//==============================================================
	//                          ExecuteOpcode
	//==============================================================
	else if(Execute_Opcode(cpu))
	{
		return 1;
	}
 
	return 0;
}

//==================================================================
//                             Main,Test
//==================================================================
int main()
{
	Cpu6502 cpu;
	unsigned int Running = 1;
 
	unsigned char TestData[] =
	{
		0xA9, 0x00,        // LDA #$00       -> A = 00
		0xAA,              // TAX            -> X = 00
		0xA8,              // TAY            -> Y = 00

		0xA9, 0x42,        // LDA #$42       -> A = 42
		0x85, 0x20,        // STA $20        -> [$0020] = 42
		0x8D, 0x00, 0x05,  // STA $0500      -> [$0500] = 42
		0x95, 0x10,        // STA $10,X      -> [$0010] = 42 (X=0)

		0xA9, 0xFF,        // LDA #$FF       -> A = FF
		0x85, 0x30,        // STA $30        -> [$0030] = FF
		0xA9, 0x01,        // LDA #$01       -> A = 01
		0x65, 0x30,        // ADC $30        -> A = 00, C=1
		0xA9, 0x05,        // LDA #$05       -> A = 05
		0x65, 0x30,        // ADC $30        -> A = 05 + FF + C = 05

		0xA9, 0xAA,        // LDA #$AA       -> A = AA
		0x29, 0x0F,        // AND #$0F       -> A = 0A
		0x09, 0xF0,        // ORA #$F0       -> A = FA
		0x49, 0xFF,        // EOR #$FF       -> A = 05

		0x0A,              // ASL A          -> A = 0A
		0x4A,              // LSR A          -> A = 05
		0x2A,              // ROL A          -> A = 0A ( C=0 )
		0x6A,              // ROR A          -> A = 05 ( C=0 )

		0xC9, 0x05,        // CMP #$05       -> Z=1
		0xF0, 0x02,        // BEQ +2         -> BEQ
		0xA9, 0x99,        // LDA #$99   
		0xA9, 0x11,        // LDA #$11       -> A = 11

		0xA0, 0xD3,         // LDY $D3
		0x8C, 0x11,0x40,   // LDA #$4011

		0x00,              // BRK
	};

#ifdef  DEBUG_CPU
	log_init("6502log.txt" , logindex);
	log_init("6502RAM.Bin" , logindex+1);
#endif
	//=====================
	// Set Emulator Values ...
	//===================== 
	CPU_Reset(&cpu , ROM_ADDRESS);
	memcpy(&cpu.Memory[ROM_ADDRESS], TestData, sizeof(TestData));
	cpu.Accumulator = 0;
	cpu.X = 0;
	cpu.Y = 0;

	//=====================
	//     ExecuteOpcode
	//===================== 
	while (Running)
	{
		Running = CPU_Clock(&cpu);
	}
#ifdef  DEBUG_CPU
	dump_printf_hex(&cpu.Memory[0] , sizeof(cpu.Memory), logindex+1);
	log_term(logindex);
	log_term(logindex+1);
	getch();
#endif 
	return 0;
}
//==================================================================

