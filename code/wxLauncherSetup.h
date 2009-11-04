/* Used to setup memory leak detection on Windows with VisualStudio. */

#ifdef _DEBUG

#define MSCRTMEMORY 1

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// catch uses of new and delete
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW

#endif
