#ifndef TOOLS_H
#define TOOLS_H
//==================================================================
void log_init(char* fn, int  index);
void log_term(int  index);
void log(int  index, const char* format, ...);
//==================================================================
void dump_printf_hex( unsigned char *buffer, unsigned int size_KB , int index);
void dump_printf(const unsigned char *buffer, unsigned int size_bytes, int index);
//==================================================================
#endif
