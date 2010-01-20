#include "version.h"
#include "generated/configure_launcher.h"

const wxUint8 MAJOR_VERSION = VERSION_MAJOR;
const wxUint8 MINOR_VERSION = VERSION_MINOR;
const wxUint8 PATCH_VERSION = VERSION_PATCH;
const wxUint64 FULL_VERSION = VERSION_MAJOR*1000000 + VERSION_MINOR*1000 + VERSION_PATCH;