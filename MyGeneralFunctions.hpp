#ifndef MYGENERALFUNCTIONS_HPP
#define MYGENERALFUNCTIONS_HPP

//----------------------------------------------------------------------------------------------------------------

#include "stdio.h"
#include <sys/types.h>

//----------------------------------------------------------------------------------------------------------------

///If the pointer value matches this value, then the memory allocated to this pointer had been freed
extern const void *const JUST_FREE_PTR;

extern const double MIN_POSITIVE_DOUBLE_VALUE;

//----------------------------------------------------------------------------------------------------------------

typedef int comp_t(const void *item1, const void *item2);

//----------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------
//! Sort array
//!
//! \param [in, out] arr array of the lines
//! \param [in] arr_size size of the array in bytes
//! \param [in] item_size size of the array item in bytes
//! \param [in] comp function-comparator
//-----------------------------------------------------------
void MG_qsort(void *arr, size_t arr_size, size_t item_size, comp_t comp);

//-----------------------------------------------------------
//! Swap 2 array elements
//!
//! \param [in, out] item1 first item
//! \param [in, out] item second item
//! \param [in] item_size size of the array item in bytes
//-----------------------------------------------------------
void MG_swap(void *item1, void *item2, size_t item_size);

//-----------------------------------------------------------
//! Print text to the stdout and run espeak for say this text
//! \param [in] format pointer to the constant line with format like in printf
//! \param [optional] ... arguments for printf and speech
//-----------------------------------------------------------
pid_t CMD_Speak(const char *format, ...);
pid_t CMD_Speak(const char *format, va_list ptr);

//-----------------------------------------------------------
//! Print text to the stdout and run espeak for say this text. Wait until saying speech
//! \param [in] format pointer to the constant line with format like in printf
//! \param [optional] ... arguments for printf and speech
//-----------------------------------------------------------
void CMD_SpeakAndWait(const char *format, ...);

void CMD_SpeakAndWait(const char *format, va_list ptr);

void GetLine(char *target);

bool isEqualDoubleNumbers(double first_num, double second_num);

int getFileSize(FILE *file);

unsigned long long factorial(int number);

//----------------------------------------------------------------------------------------------------------------

#endif //MYGENERALFUNCTIONS_HPP