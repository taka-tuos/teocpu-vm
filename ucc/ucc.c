#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define OPTION(s) strcmp(argv[i],s) == 0

char *getpath(char *path)
{
	char *p = path + strlen(path);
	for(;p!=path&&*p!='/'&&*p!='\\';p--);
	p++;
	*p=0;
	return path;
}

char *getpure(char *path)
{
	char *p = path + strlen(path);
	for(;p!=path&&*p!='/'&&*p!='\\';p--);
	return p;
}

void execute_cmd(char *cmd, char *name, int enlog, FILE *logfp)
{
	char **stdout_d = NULL;
	int stdout_n = 0;
	char stdout_b[1024];
	int i;
	int child_ret = -2017;
	
	if(enlog) fprintf(logfp,"%s Stage\nCommand line : %s\n>>>>BEGIN\n",name,cmd);
	FILE *fp = popen(cmd,"r");
	while(fp && fgets(stdout_b,1024,fp)) {
		stdout_d = (char **)realloc(stdout_d, sizeof(char *) * (stdout_n + 1));
		stdout_d[stdout_n] = strdup(stdout_b);
		if(enlog) fputs(stdout_d[stdout_n],logfp);
		stdout_n++;
	}
	if(!fp || (child_ret = pclose(fp)) != 0) {
		if(child_ret != -2017) fprintf(stderr,"%s Returned %d\n",name,child_ret);
		else fprintf(stderr,"%s execute failed\n",name);
		fprintf(stderr,"%s execute stage failed.\n",name);
		fprintf(stderr,"stdout :\n");
		for(i = 0; i < stdout_n; i++) printf(stdout_d[i]);
		if(enlog) {
			fprintf(logfp,"%s Returned %d\n",name,child_ret);
			fprintf(logfp,">>>>END(FAILED)\n\n");
			fclose(logfp);
		}
		exit(-1);
	}
	if(enlog) fprintf(logfp,">>>>END\n\n");
}

int main(int argc, char *argv[], char *envp[])
{
	char cc1[4096];
	char cpp[4096];
	char as[4096];
	
	char binname[2048];
	char logname[2048] = "";
	
	char sz[512];
	char option_format[64] = "-flat";
	int i;
	int next = 0;
	int ret;
	int ptr = 0;
	int enlog = 0;
	
	char argvpath[2048];
	
	readlink("/proc/self/exe", argvpath, sizeof(argvpath)-1);
	
	char *dumppath = strdup(argvpath);
	char *path = getpath(dumppath);
	
	sprintf(as, "%s/../as/as ",path);
	sprintf(cc1,"%s/../cc1/cc1 ",path);
	sprintf(cpp, "%s/../cpp/cpp ",path);
	
	if(argc < 3) {
		printf("usage : %s [option] source output\n",argv[0]);
		puts("Error : too few argments");
		return 3;
	}
	
	for(i = 1; i < argc; i++) {
		if(next == 1) {
			strcat(cpp,argv[i]);
			strcat(cpp," ");
			next = 0;
			continue;
		}
		if(next == 2) {
			strcpy(logname,argv[i]);
			next = 0;
			continue;
		}
		if(
			OPTION("-I") ||
			OPTION("-J")
		) {
			strcat(cpp,argv[i]);
			strcat(cpp," ");
			next = 1;
		} else if(
			OPTION("-C") ||
			OPTION("-s") ||
			OPTION("-l") ||
			OPTION("-CC") ||
			OPTION("-a") ||
			OPTION("-na") ||
			OPTION("-V") ||
			OPTION("-u") ||
			OPTION("-X") ||
			OPTION("-c90") ||
			OPTION("-w") ||
			OPTION("-zl") ||
			OPTION("-M") ||
			OPTION("-D") ||
			OPTION("-U") ||
			OPTION("-A") ||
			OPTION("-B") ||
			OPTION("-Y") ||
			OPTION("-Z") ||
			OPTION("-d") ||
			OPTION("-e") ||
			OPTION("-v") ||
			OPTION("-h")
		) {
			strcat(cpp,argv[i]);
			strcat(cpp," ");
		} else if(
			OPTION("-flat") ||
			OPTION("-coff")
		) {
			strcpy(option_format,argv[i]);
		} else if(
			OPTION("-log")
		) {
			next = 2;
			enlog = 1;
		} else if(
			OPTION("-help")
		) {
			puts("micro portable cc `UCC` by kagura1050");
			puts("Version : git");
			printf("usage : %s [option] source output\n",argv[0]);
			return 1;
		} else {
			if(ptr < 2) {
				if(ptr) {
					strcpy(binname,argv[i]);
				}
				if(!ptr) {
					strcat(cpp,argv[i]);
					strcat(cpp," ");
				}
			} else {
				puts("Error : too many filename");
				return 1;
			}
			ptr++;
		}
	}
	
	if(ptr < 2) {
		puts("Error : unable to get output filename");
		return 2;
	}
	
	if(logname[0] == 0 && enlog) {
		puts("Error : log file is not specified");
		return 4;
	}
	
	int tmp = time(NULL);
	
	char *allname = strdup(binname);
	char *purename = getpure(allname);
	
	sprintf(sz,"_ucc_%08x_%s_asm.tmp %s %s",tmp,purename,binname,option_format);
	strcat(as,sz);
	
	sprintf(sz,"-o _ucc_%08x_%s_cxx.tmp",tmp,purename);
	strcat(cpp,sz);
	
	sprintf(sz,"_ucc_%08x_%s_cxx.tmp _ucc_%08x_%s_asm.tmp",tmp,purename,tmp,purename);
	strcat(cc1,sz);
	
	int child_ret = -2017;
	FILE *fp;
	
	FILE *logfp;
	
	if(enlog) {
		logfp = fopen(logname,"wt");
	}
	
	execute_cmd(cpp,"Preprocessor",enlog,logfp);
	execute_cmd(cc1,"Compiler",enlog,logfp);
	execute_cmd(as,"Assembler",enlog,logfp);
	
	if(enlog) fclose(logfp);
	
	return 0;
}
