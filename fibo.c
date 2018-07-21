#include "xprintf.h"

void putchar(unsigned char c)
{
	asm("\tpi($8)\n");
	asm("\tpr(%d34)\n");
	asm("\tpr(%d24)\n");
	asm("\tldd()\n");
	asm("\tpr(%d24)\n");
	asm("\ttx()\n");
}

int main(void)
{
	/* 変数の宣言 */
	int n;
	int f0, f1, f2;

	xdev_out(putchar);

	f0 = 0;
	f1 = 1;

	/* フィボナッチ数(n=0)の出力 */
	xprintf("%d\n", f0);

	/* フィボナッチ数の計算 */
	while(f1<100000) {
		// フィボナッチ数の出力(n>0)
		xprintf("%d\n", f1);
		// フィボナッチ数の計算
		f2 = f1 + f0;
		// 変数の代入
		f0 = f1;
		f1 = f2;
	}

	return 0;
}
