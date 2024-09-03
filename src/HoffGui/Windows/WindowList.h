// List of all ImGui windows in the application.

// No include guard (e.g. #pragma once) to allow this file to be included multiple times in the same compilation unit.

// Example usage:
// 
//     #define WINDOW_LIST_MACRO(T) static T##Window Get##T##();
//     #include "WindowList.h"
//

#ifndef WINDOW_LIST_MACRO
#error WINDOW_LIST_MACRO is not defined
#endif

WINDOW_LIST_MACRO(ModWindow)
WINDOW_LIST_MACRO(OutputWindow)

#undef WINDOW_LIST_MACRO
