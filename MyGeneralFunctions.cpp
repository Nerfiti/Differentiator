#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "MyGeneralFunctions.hpp"

const void *const JUST_FREE_PTR = "JUST_FREE";

const double MIN_POSITIVE_DOUBLE_VALUE = 10e-8; 

//--------------------------------------------------------------------------------------

static pid_t PID_Wait(pid_t PID);

//--------------------------------------------------------------------------------------

void MG_qsort(void *arr, size_t arr_size, size_t item_size, comp_t comp)
{
    assert(arr != nullptr);

    if (arr_size <= item_size)
    {
        return;
    }

    int last = 1;
    char *array = (char *)arr;

    for (int i = 1; i < arr_size/item_size; ++i)
    {
        if (comp(array, array + i*item_size) > 0)
        {
            MG_swap(array + i*item_size, array + last*item_size, item_size);
            last++;
        }
    }
    MG_swap(array, array + (last - 1)*item_size, item_size);
    MG_qsort(array, (last-1)*item_size, item_size, comp);
    MG_qsort(array + last*item_size, arr_size - last*item_size, item_size, comp);
}

void MG_swap(void *item1, void *item2, size_t item_size)
{
    char *ITEM1 = (char *)item1;
    char *ITEM2 = (char *)item2;

    for (int i = 0; i < item_size; ++i)
    {
        char temp = *(ITEM1 + i);
        *(ITEM1 + i) = *(ITEM2 + i);
        *(ITEM2 + i) = temp;
    }
}

pid_t CMD_Speak(const char *format, ...)
{
    va_list ptr;
    va_start(ptr, format);

    pid_t PID = CMD_Speak(format, ptr);

    va_end(ptr);
    return PID;
}

pid_t CMD_Speak(const char *format, va_list ptr)
{
    const int MAX_SPEECH_LEN = 300;
    char msg[MAX_SPEECH_LEN] = "";
    vsprintf(msg, format, ptr);

    pid_t PID = fork();
    if (PID == 0) 
    {
        execlp("espeak", "espeak", "-s", "180", msg, (char *)0);
        perror("Error running espeak: ");
        exit(1);
    }

    printf("%s", msg);

    fflush(stdout);
    
    return PID;
}

void CMD_SpeakAndWait(const char *format, ...)
{
    va_list ptr;
    va_start(ptr, format);

    CMD_SpeakAndWait(format, ptr);

    va_end(ptr);
}

void CMD_SpeakAndWait(const char *format, va_list ptr)
{
    pid_t PID = CMD_Speak(format, ptr);
    PID_Wait(PID);
}

void GetLine(char *target)
{
    scanf("%[^\n]", target);
    getchar();
}

bool isEqualDoubleNumbers(double first_num, double second_num)
{
    double difference = first_num - second_num;
    return (-MIN_POSITIVE_DOUBLE_VALUE < difference && difference < MIN_POSITIVE_DOUBLE_VALUE); 
}

int getFileSize(FILE *file)
{
    
    struct stat info = {};
    fstat(fileno(file), &info);

    return info.st_size;
}

unsigned long long factorial(int number)
{
    unsigned long long result = 1;
    for (int i = 1; i <= number; ++i)
    {
        result *= i;
    }

    return result;
}

//--------------------------------------------------------------------------------------

static pid_t PID_Wait(pid_t PID)
{
    int status = 0;
    return waitpid(PID, &status, 0);
}
