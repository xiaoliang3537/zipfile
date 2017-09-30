/************关于本文档********************************************
*filename: fnmatch.c
*purpose: 说明用fnmatch进行字符匹配的方法
*wrote by: zhoulifa(zhoulifa@163.com) 周立发(http://zhoulifa.bokee.com)
Linux爱好者 Linux知识传播者 SOHO族 开发者 最擅长C语言
*date time:2008-01-27 20:33 上海大雪天，据说是多年不遇
*Note: 任何人可以任意复制代码并运用这些文档，当然包括你的商业用途
* 但请遵循GPL
*Thanks to:
*                Ubuntu 本程序在Ubuntu 7.10系统上测试完全正常
*                Google.com 我通常通过google搜索发现许多有用的资料
*Hope:希望越来越多的人贡献自己的力量，为科学技术发展出力
* 科技站在巨人的肩膀上进步更快！感谢有开源前辈的贡献！

添加注释: wenhao
编译方式: gcc fnmatch.c -o fnmatch -Wall
输入指令: ./fnmatch "*.c" .
*********************************************************************/
#include <locale.h>
#include "fnmatch.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#define BOOL int
#define TRUE 1
#define FALSE 0


BOOL match(const char *pattern, const char *content) {

    int ret = fnmatch(pattern, content, FNM_PATHNAME|FNM_PERIOD);//匹配成功返回0
    if(ret == 0)
    {
        return TRUE;
    }else{
        return FALSE;
    }
}
void test(const char *pattern, const char *content)
{
    if (NULL == pattern || NULL == content)
        puts("no");
    match(pattern, content) ? puts("yes") : puts("no");
}


void test_in()
{
    test("g*ks", "geeks"); // Yes
    test("ge?ks*", "geeksforgeeks"); // Yes
    test("g*k", "gee");  // No because 'k' is not in second
    test("*pqrs", "pqrst"); // No because 't' is not in first
    test("abc*bcd", "abcdhghgbcd"); // Yes
    test("abc*c?d", "abcd"); // No because second must have 2 instances of 'c'
    test("*c*d", "abcd"); // Yes
    test("*?c*d", "abcd"); // Yes

}

int main(int argc, char *argv[])
{
    //  ./test_fnMatch "*.cpp"   ./
    char    *pattern;
    DIR     *dir;
    struct  dirent *entry;
    int     ret;

    if(1)
    {
        test_in();
        return 0;
    }

    dir = opendir(argv[2]); //打开argv[2]文件夹
    pattern = argv[1];      //将argv[1]测试条件放在pattern中.

    printf("the str is %s\n",pattern);

    if(dir != NULL)
    {
        while( (entry = readdir(dir)) != NULL)//判断是否将本文件夹内容读完
        {

            ret = fnmatch(pattern, entry->d_name, FNM_PATHNAME|FNM_PERIOD);//匹配成功返回0
            if(ret == 0)
            {
                printf("=========>%s\n", entry->d_name);//成功打印文件名
            }
            else if(ret == FNM_NOMATCH)
            {

                continue; //不成功继续匹配
            }
            else
            {
                printf("error file=%s\n", entry->d_name);
            }
        }
        closedir(dir);//关闭打开的文件夹
    }

    return 0;
}
