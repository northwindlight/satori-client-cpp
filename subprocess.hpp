#pragma once

#include "subprocess/basic_types.hpp"
#include "subprocess/pipe.hpp"
#include "subprocess/ProcessBuilder.hpp"
#include "subprocess/shell_utils.hpp"
#include "subprocess/environ.hpp"

#ifdef SUBPROCESS_AMALGAMATE_SOURCES
/* To regen:
    (for file in *.cpp; do; echo '#include "'subprocess/$file'"'; done;) | sort
*/
#include "subprocess/CowData.cpp"
#include "subprocess/environ.cpp"
#include "subprocess/pipe.cpp"
#include "subprocess/PipeVar.cpp"
#include "subprocess/ProcessBuilder.cpp"
#include "subprocess/ProcessBuilder_posix.cpp"
#include "subprocess/ProcessBuilder_windows.cpp"
#include "subprocess/shell_utils.cpp"
#include "subprocess/utf8_to_utf16.cpp"

#endif