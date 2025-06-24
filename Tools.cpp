// Tools.cpp
#include "6502C.h"
#ifdef  DEBUG_CPU
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "Tools.h"
//==================================================================
FILE  *ifl[4];
//==================================================================
void dump_printf_hex( unsigned char *buffer, unsigned int size_KB , int index)
{
	fwrite(buffer , size_KB , 1, ifl[index]);
}
//==================================================================
void dump_printf(const unsigned char *buffer, unsigned int size_bytes, int index)
{
	for (unsigned int i = 0; i < size_bytes; i++)
	{
		if (i > 0 && i % 16 == 0)
			fwrite("\n", 1, 1, ifl[index]);

		char byteStr[4];
		snprintf(byteStr, sizeof(byteStr), " %02X", buffer[i]);
		fwrite(byteStr, 3, 1, ifl[index]);
	}

	fwrite("\n", 1, 1, ifl[index]);
}
//==================================================================
void log_init(char* fn , int  index)
{
	ifl[index] = fopen(fn, "wb+");  
}
//==================================================================
void log_term(int  index)
{ 
     fclose(ifl[index]);
}
//==================================================================
void log(int  index , const char* format, ...)
{
		char s[128], st[10000];

		//SYSTEMTIME t;
		//GetLocalTime(&t);
		//sprintf(s, "%d.%d.%d[%.2d:%.2d:%.2d] : " ,t.wYear ,t.wMonth ,t.wDay ,t.wHour ,t.wMinute ,t.wSecond);

		va_list arg_ptr;
		va_start(arg_ptr, format);
		vsprintf(st, format, arg_ptr);
		va_end(arg_ptr);
		/*
		int l;
		for(l=0;l<sizeof(s);l++){
		if(s[l]==0){
		break;
		}
		}
		fwrite(s ,l ,1 ,ifl);
		*/
		int i;
		for (i = 0;i<sizeof(st);i++) {
			if (st[i] == 0) {
				//st[i]='\n';
				break;
			}
		}
		fwrite(st, i, 1,  ifl[index]);
}
//==================================================================
#endif
