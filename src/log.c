#include "log.h"
#include <stdarg.h>
#include<stdlib.h>
const char *levelMap[] = {"DEBUG", "NORMAL", "WARNING", "ERROR", "FATAL"};
char *create_error_message(const char *msg){
    const char *error_str=strerror(errno);
    size_t msg_len = strlen(msg);
    size_t err_len=strlen(error_str);
    size_t total_len = msg_len + err_len;
    char *fullmsg=(char *)malloc(total_len);
    if(fullmsg==NULL)
        return NULL;
    snprintf(fullmsg,total_len,"%s:%s",msg,error_str);
    return fullmsg;
}
/*
检测level是否是配置文件中要求要输入到文件中的日志
*/
static int shouldLogToFile(int level, const char *configFile)
{
    FILE *file = fopen(configFile, "r");
    if (!file)
    {
        perror("fopen failed");
        return 0;
    }
    int logToFile = 0;
    char line[128];
    // 不停的从文件中读取一行
    while (fgets(line, sizeof(line), file))
    {
        int configLevel;
        // 按照指定的格式将行数据读取到指定的变量中，sscanf的返回值是一个整数，表示成功读取和转换的输入项数，0表示没有成功读取和转换任何项目，-1代表发生了错误
        if (sscanf(line, "%d", &configLevel) == 1 && configLevel == level)
        {
            logToFile = 1;
            break;
        }
    }
    fclose(file);
    return logToFile;
}

void logMessage(int level, const char *file, int line, const char *format, ...)
{
#ifdef NO_DEBUG
    if (level == DEBUG)
        return;
#endif
    // Ensure the log level is valid
    if (level < 0 || level >= sizeof(levelMap) / sizeof(levelMap[0]))
    {
        fprintf(stderr, "Invalid log level: %d\n", level);
        return;
    }

    char fixBuffer[512];
    time_t currentTime;
    if (time(&currentTime) == (time_t)-1)
    {
        perror("time failed");
        return;
    }

    char *timestr = ctime(&currentTime);
    if (timestr == NULL)
    {
        perror("ctime failed");
        return;
    }

    // Remove the newline character from ctime result
    timestr[strcspn(timestr, "\n")] = '\0';

    snprintf(fixBuffer, sizeof(fixBuffer), "<%s>==[file->%s][line->%d][time->%s]", levelMap[level], file, line, timestr);

    char defBuffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(defBuffer, sizeof(defBuffer), format, args);
    va_end(args);

    const char *configFile = CONFIGFILE;
    if (shouldLogToFile(level, configFile))
    {
        // 当前函数指定的输出级别被要求输入到文件
        FILE *logFile = fopen(LOGFILE, "a");
        if (!logFile)
        {
            perror("fopen failed, please check LOGFILE path, exiting...");
            exit(1);
        }
        fprintf(logFile, "%s:%s\n", fixBuffer, defBuffer);
        fclose(logFile);
    }
    else
    {
        printf("%s:%s\n", fixBuffer, defBuffer);
    }
}
