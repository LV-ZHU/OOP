/* 2452487 信安 胡中芃 */
#pragma once

//重载simple_menu函数，实现三种不同参数类型的菜单显示和选择功能
char simple_menu(const char* items[], const char* choice);
char simple_menu(const char items[][80], const char* choice);
char simple_menu(const char* items, const char* choice);