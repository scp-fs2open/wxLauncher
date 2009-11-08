#ifndef VERSION_H
#define VERSION_H

#define MAJOR_VERSION 0
#define MINOR_VERSION 5

#define FULL_VERSION (MAJOR_VERSION*1000000 + MINOR_VERSION*1000)

extern wchar_t *HGVersion;
extern wchar_t *HGDate;

#endif