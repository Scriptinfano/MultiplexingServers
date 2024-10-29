#pragma once
#define FILENAME __FILE__
#define LINE __LINE__
#define LOGFILE "./log/log.txt"       // 日志输出到哪个文件
#define CONFIGFILE "./etc/config.txt" // 哪些级别的日志应该输出到文件
extern const char *levelMap[];
#define LOGPARAM FILENAME,LINE

enum level
{
    DEBUG,
    NORMAL,
    WARNING,
    ERROR,
    FATAL
};
/*
@brief 将msg代表的自定义字符串和此时errno所代表的错误信息拼接起来，在函数内部使用malloc申请一块存储空间，然后将拼接之后的字符串放到其中并最后返回地址
@param msg 用户自定义的字符串
@brief 拼接之后的字符串缓冲区地址
*/
char *create_error_message(const char *msg);
/*
@brief 日志输出函数，输出格式化之后的日志信息
@param level 日志输出级别
@param file 直接填入FILENAME常值即可，代表当前文件名字
@param line 直接填入LINE常值即可，代表当前行号
@param format 格式化字符串
@param ... 可变形式参数，填入上一个格式化字符串的填充参数
*/
void logMessage(int level, const char *file, int line, const char *format, ...);
