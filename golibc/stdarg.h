/* copyright(C) 2003 H.Kawai (under KL-01). */

#if (!defined(STDARG_H))

#define STDARG_H	1

#if (defined(__cplusplus))
	extern "C" {
#endif

typedef char    *__va_list[1];

#define va_start(ap,pn)		((ap)[0]=(char *)&pn+\
	((sizeof(pn)+sizeof(int)-1)&~(sizeof(int)-1)),(void)0)
#define va_end(ap)			((ap)[0]=0,(void)0)
#define va_arg(ap,type)		((ap)[0]+=\
	((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)),\
	(*(type *)((ap)[0]-((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)))))
#define va_copy(dest,src)	((dest)[0]=(src)[0],(void)0)
typedef	__va_list				va_list;

#if (defined(__cplusplus))
	}
#endif

#endif
