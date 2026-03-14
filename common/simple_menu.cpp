/* 2452487 信安 胡中芃 */
#include <iostream>
#include <conio.h>
#include <cstring>
#include "../include/cmd_console_tools.h"
#include "../include/simple_menu.h"
using namespace std;

char simple_menu(const char* items[], const char* choice)
{
	char sel;
	while (1) {
		cct_cls();
		for (int i = 0; items[i]; i++) {
			cout << items[i] << endl;
		}
		sel = _getche();//_getche可显示菜单值
		if (strchr(choice, sel) != NULL)//等于NULL时说明菜单结束，跳出循环
			break;
	}
	return sel;
}

char simple_menu(const char items[][80], const char* choice)
{
	char sel;
	while (1) {
		cct_cls();
		for (int i = 0; items[i][0] != 0; i++) {
			cout << items[i] << endl;
		}
		sel = _getche();//_getche可显示菜单值
		if (strchr(choice, sel) != NULL)//等于NULL时说明菜单结束，跳出循环
			break;
	}
	return sel;
}

char simple_menu(const char* items, const char* choice)
{
	char sel;
	while (1) {
		cct_cls();

		cout << items << endl;

		sel = _getche();//_getche可显示菜单值
		if (strchr(choice, sel) != NULL)//等于NULL时说明菜单结束，跳出循环
			break;
	}
	return sel;
}