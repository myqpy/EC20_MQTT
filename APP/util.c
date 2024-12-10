#include "util.h"
#include <stdbool.h> 
#include <string.h>



// 双字节大小段互换
unsigned short EndianSwap16(unsigned short u16val)
{
  return (((u16val & 0x00FF) << 8) +
          ((u16val & 0xFF00) >> 8));
}


char *myitoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;
 
    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }
 
    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }
 
    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';
 
        /* Make the value positive. */
        value *= -1;
    }
 
    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;
 
        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }
 
    /* Null terminate the string. */
    *ptr = 0;
 
    return string;
 
}

//处理字符串，返回子串在母串的第一个字符的位置
int strStr(const char * haystack,  const char * needle)
{
	int len1=strlen(haystack);
	int len2=strlen(needle);
	int i=0,j=0,k=0;
	bool jieguo=true;
	if(len1<len2)
	{
		jieguo=false;
	}
	else
	{
		for(i=0;i<=len1-len2;i++)
	{
	for(j=i,k=0;j<i+len2;j++,k++)
	{
		if(haystack[j]!=needle[k])
		{
			jieguo=false;
			break;
		}
		if(haystack[j]==needle[k])
		{
			jieguo=true;
		}
	}
	if(jieguo==true)
	{
		break;
	}
	}
	}
	if(jieguo==false)
	{
		return -1;
	}
	else
	{
		return i;
	}
}
