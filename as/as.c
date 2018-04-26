#include "../teocpu-vm.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define SIZE_SYMBOL (64 + (16 * 4))

#define errmsg(s) puts(s); return -1;

typedef struct {
	char name[64];
	uint32_t attribute[16];
} symbol_t;

char *teocpu_assembly[] = {
	"0nop",
	"1lr",
	"2li",
	"1sr",
	"0stb",
	"0stw",
	"0std",
	"0ldb",
	"0ldw",
	"0ldd",
	"0add",
	"0sub",
	"0mul",
	"0div",
	"0muli",
	"0divi",
	"0neg",
	"0abs",
	"0sxb",
	"0sxw",
	"0and",
	"0or",
	"0xor",
	"0not",
	"0cmp",
	"0cmpi",
	"0tst",
	"0tsti",
	"0ce",
	"0cg",
	"0cl",
	"0ceg",
	"0cel",
	"0b",
	"0bc",
	"0c",
	"0cc",
	NULL,
};

int instruction_size[] = {
	1, 2, 5
};

typedef enum {
	tok_mode_nil,
	tok_mode_ins,
	tok_mode_arg,
	tok_mode_macro,
} tok_mode;

typedef struct {
	int offset;
	char *name;
} label_t;

label_t label_list[2048];
int last_label = 0;

symbol_t symbol[4096];
uint32_t replace_addr[65536];
int last_replace = 0;
int last_symbol = 0;
int org_offset = 0;
int now_offset = 0;

void replacepoint(int offset)
{
	replace_addr[last_replace] = now_offset + offset;
	last_replace++;
}

int get_symboladdr(char *s)
{
	int i;
	char *d = s;

	for(i=0;i<4096;i++) {
		if(symbol[i].name[0] != 0 && strcmp(d,symbol[i].name)==0) return i;
	}

	return -1;
}

int main(int argc, char *argv[]) {
	int coff;
	
	FILE *src = fopen(argv[1], "rt");
	FILE *dst = fopen(argv[2], "wb");
	
	char **tok_list = NULL;
	int tok_len = 0;
	
	if(strcmp(argv[3],"-flat") == 0) coff = 0;
	else if(strcmp(argv[3],"-coff") == 0) coff = 1;
	
	do {
		char l[256];
		if(fgets(l, 256, src) == NULL) break;
		
		char *p = l;
		while(isspace(*p) && *p) p++;
		char *q = p;
		
		while(!isspace(*p) && *p) p++;
		
		*p = 0;
		p++;
		
		if(q[strlen(q)-1] == '\n') q[strlen(q)-1] = 0;
		
		tok_list = realloc(tok_list, sizeof(char *) * (tok_len + 1));
		tok_list[tok_len] = strdup(q);
		tok_len++;
		
		puts(q);
		
		if(*p != 0) {
			if(p[strlen(p)-1] == '\n') p[strlen(p)-1] = 0;
			
			tok_list = realloc(tok_list, sizeof(char *) * (tok_len + 1));
			tok_list[tok_len] = strdup(p);
			tok_len++;
			
			puts(p);
		}
	} while(!feof(src));
	
	int prev_is_inst = 0;
	
	for(int tokptr = 0; tokptr < tok_len; tokptr++) {
		char *tok = tok_list[tokptr];
		if(isalpha(tok[0])) {
			prev_is_inst = 1;
			char *ins = strdup(tok);
			for(char *p = ins; *p; p++) *ins = tolower(*ins);
			for(int i = 0;; i++) {
				if(strcmp("db", ins) == 0) {
					now_offset++;
					break;
				} else if(strcmp("global", ins) == 0) {
					tokptr++;
					if(tokptr < tok_len) {
						if(tok[0] == '.') {
							int addr = -1;
							for(int i = 0;; i++) {
								if(i == last_label) {
									printf("Unknown label \"%s\"\n", tok);
									return -1;
								}
								if(strcmp(tok, label_list[i].name) == 0) {
									uint8_t p[4];
									addr = label_list[i].offset;
									break;
								}
							}
							if(addr < 0) {
								if(get_symboladdr(tok) >= 0) errmsg("symbols already declared");
								for(i=0;i<256 && tok[i]!=0x0d && tok[i]!=0x0a && tok[i]!=0;i++) {
									symbol[last_symbol].name[i]=tok[i];
								}
								symbol[last_symbol].attribute[0] = last_symbol + 0x40000000;
								printf("EXTERN %s : %08x\n",symbol[last_symbol].name,symbol[last_symbol].attribute[0]);
								last_symbol++;
							} else {
								if(get_symboladdr(tok) >= 0) errmsg("symbols already declared");
								for(i=0;i<256 && tok[i]!=0x0d && tok[i]!=0x0a && tok[i]!=0;i++) {
									symbol[last_symbol].name[i]=tok[i];
								}
								symbol[last_symbol].attribute[0] = addr - org_offset;
								printf("GLOBAL %s : %08x\n",symbol[last_symbol].name,symbol[last_symbol].attribute[0]);
								last_symbol++;
							}
						}
					}
					break;
				} else if(strcmp("ascii", ins) == 0) {
					tokptr++;
					if(tokptr < tok_len) {
						now_offset += strlen(tok_list[tokptr]) - 1;
						if(tok_list[tokptr][strlen(tok_list[tokptr])-1] == '\'') now_offset--;
					}
					break;
				} else if(teocpu_assembly[i] == NULL) {
					printf("label_scan: Unknown instruction \"%s\"\n", ins);
					return -1;
				}
				if(strcmp(ins, teocpu_assembly[i] + 1) == 0) {
					now_offset += instruction_size[teocpu_assembly[i][0] - '0'];
					if(teocpu_assembly[i][0] - '0' == 0) prev_is_inst = 0;
					break;
				}
			}
		} else if(tok[0] == '.' && !prev_is_inst) {
			prev_is_inst = 0;
			label_list[last_label].name = strdup(tok);
			label_list[last_label].offset = now_offset;
			last_label++;
			printf("label_scan: found \"%s\"\n", tok);
		} else {
			prev_is_inst = 0;
		}
	}
	
	if(coff) {
		uint32_t repaddr = 4 * 4 + last_symbol * SIZE_SYMBOL;
		fprintf(dst,"TEOC");
		uint8_t ls_le[4];
		uint8_t ra_le[4];
		uint8_t lr_le[4];
		
		printf("entno : %d\nrepaddr : %x\nrepno : %d\n\n",last_symbol,repaddr,last_replace);
		
		teocpu_write32_unpaged(ls_le,last_symbol);
		teocpu_write32_unpaged(ra_le,repaddr);
		teocpu_write32_unpaged(lr_le,last_replace);
		
		fwrite(ls_le,1,4,dst);
		fwrite(ra_le,1,4,dst);
		fwrite(lr_le,1,4,dst);
		
		for(int i = 0; i < last_symbol; i++) {
			fwrite(symbol[i].name,1,64,dst);
			uint8_t le[4];
			for(int j = 0; j < 16; j++) {
				teocpu_write32_unpaged(le,symbol[i].attribute[j]);
				fwrite(le,1,4,dst);
			}
		}
		
		for(int i = 0; i < last_replace; i++) {
			uint8_t le[4];
			teocpu_write32_unpaged(le,replace_addr[i]);
			fwrite(le,1,4,dst);
		}
	}
	
	now_offset = 0;
	
	tok_mode m = tok_mode_nil;
	tok_mode prevm = tok_mode_nil;
	int ins_type = 0;
	
	for(int tokptr = 0; tokptr < tok_len; tokptr++) {
		char *tok = tok_list[tokptr];
		prevm = m;
		if(isalpha(tok[0])) {
			m = tok_mode_ins;
		} else if((tok[0] == '.' && (prevm == tok_mode_ins && ins_type != 0)) || tok[0] == '#' || tok[0] == '%' || tok[0] == '\'' || tok[0] == '$') {
			m = tok_mode_arg;
		} else {
			m = tok_mode_nil;
		}
		
		if(m == tok_mode_ins) {
			char *ins = strdup(tok);
			for(char *p = ins; *p; p++) *ins = tolower(*ins);
			for(int i = 0;; i++) {
				if(strcmp("db", ins) == 0) {
					ins_type = -1;
					break;
				} else if(strcmp("global", ins) == 0) {
					ins_type = -3;
					break;
				} else if(strcmp("ascii", ins) == 0) {
					ins_type = -2;
					break;
				} else if(teocpu_assembly[i] == NULL) {
					printf("Unknown instruction \"%s\"\n", ins);
					return -1;
				}
				if(strcmp(ins, teocpu_assembly[i] + 1) == 0) {
					ins_type = teocpu_assembly[i][0] - '0';
					fputc(i, dst);
					now_offset++;
					break;
				}
			}
		}
		
		if(m == tok_mode_arg) {
			if( (ins_type == 1 && (tok[0] == '#' || tok[0] == '.')) ||
			(ins_type == 2 && tok[0] == '%') || ins_type == 0 || 
			((tok[0] == '$' || tok[0] == '\'') && ins_type >= 0) || 
			(!(tok[0] == '$' || tok[0] == '\'' || tok[0] == '.') && ins_type < 0) ||
			((tok[0] == '$' || tok[0] == '.') && ins_type == -2) || 
			((tok[0] == '\'' || tok[0] == '$') && ins_type == -3) || 
			((tok[0] == '\'' || tok[0] == '.') && ins_type == -1) ) {
				puts("Invalid type argments");
				return -1;
			}
			if(tok[0] == '.') {
				for(int i = 0; ins_type != -3; i++) {
					if(i == last_label) {
						printf("Unknown label \"%s\"\n", tok);
						return -1;
					}
					if(strcmp(tok, label_list[i].name) == 0) {
						uint8_t p[4];
						teocpu_write32_unpaged(p, label_list[i].offset);
						fwrite(p, 1, 4, dst);
						break;
					}
				}
			} else if(tok[0] == '#') {
				char *p = strdup(tok + 1);
				uint8_t u32[4];
				long int n = strtol(p, &p, 0);
				teocpu_write32_unpaged(u32, n);
				fwrite(u32, 1, 4, dst);
			} else if(tok[0] == '%') {
				if(!isdigit(tok[1])) {
					puts("Invalid register number");
					return -1;
				}
				char *p = strdup(tok + 1);
				int n = atoi(p);
				fputc(n, dst);
			} else if(tok[0] == '$') {
				char *p = strdup(tok + 1);
				long int n = strtol(p, &p, 0);
				n = n & 0xff;
				fputc(n, dst);
			} else if(tok[0] == '\'') {
				char *p = strdup(tok + 1);
				if(p[strlen(p)-1] == '\'') p[strlen(p)-1] = 0;
				fputs(p, dst);
			} else {
				printf("Unknown type argments \"%s\"\n",tok);
				return -1;
			}
		}
	}
	
	fclose(dst);
	fclose(src);
	
	return 0;
}
