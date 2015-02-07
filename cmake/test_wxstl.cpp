#include <wx/wx.h>

#ifndef wxUSE_STL
#error wxUSE_STL not defined
#elif wxUSE_STL == 0
#error wxUSE_STL is not enabled
#endif

int main(int, char**)
{
	return 0;
}
