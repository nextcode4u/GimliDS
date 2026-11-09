#pragma once
#include <cstdint>
#include <cstring>
typedef uint8_t u8;
#define MAX_FILENAME_LEN 256
struct DrivePrefs { char DrivePath[2][256] = {}; bool TrueDrive = false; };
struct Config { bool trueDrive = false; };
extern DrivePrefs TheDrivePrefs;
extern Config myConfig;
extern char Drive8File[256], Drive9File[256], CartFilename[256];
class C64 { public: void RemoveCart(); void NewPrefs(DrivePrefs*); void PatchKernal(bool); void Reset(); void Pause(); void Resume(); };
