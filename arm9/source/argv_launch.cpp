// SPDX-License-Identifier: MIT
// Isolated startup adapter. Existing delayed cartridge/PRG handlers do the loading.
#include "sysdeps.h"
#include "argv_launch.h"
#include "argv_path.h"
#include "C64.h"
#include "1541d64.h"
#include "diskmenu.h"
#include "mainmenu.h"
#include <unistd.h>

static char captured_argument[1024];
static bool argument_captured;
static bool storage_ready = true;
static char mount_directory[256];
static char launch_directory[256];
static char launch_filename[MAX_FILENAME_LEN];
static int launch_delay;
extern int bDelayLoadPRG;
extern int bDelayLoadCRT;
extern u8 cart_in;
extern u8 bLastFileTypeLoaded;
extern void LoadGameConfig(void);
extern void ShowKeyboard(void);
extern void kbd_buf_reset(void);
extern void kbd_buf_feed(const char *text);

void argv_launch_capture(int argc, char **argv)
{
    argument_captured = true;
    captured_argument[0] = 0;
    if (argc > 1 && argv && argv[1] && strlen(argv[1]) < sizeof(captured_argument))
        strcpy(captured_argument, argv[1]);
}

void argv_launch_remember_cwd(void)
{
    storage_ready = argv_path_select_root(captured_argument);
    if (!storage_ready || !getcwd(mount_directory, sizeof(mount_directory)))
        mount_directory[0] = 0;
}

void argv_launch_prepare(int argc, char **argv)
{
    launch_delay = 0;
    const char *argument = argument_captured ? captured_argument
        : (argc > 1 && argv && argv[1] ? argv[1] : NULL);
    if (!storage_ready || !argument || !*argument || !mount_directory[0]
        || !argv_path_prepare(argument, mount_directory, launch_directory, sizeof(launch_directory),
                              launch_filename, sizeof(launch_filename)))
        return;
    if (!argv_path_extension(launch_filename, ".crt")
        && !argv_path_extension(launch_filename, ".prg")
        && !argv_path_extension(launch_filename, ".d64"))
        return;
    char previous[256], canonical[256];
    if (!getcwd(previous, sizeof(previous)) || chdir(launch_directory) != 0)
        return;
    if (!getcwd(canonical, sizeof(canonical)))
    {
        chdir(previous);
        return;
    }
    // Give the C64's initial reset time to reach its normal keyboard/BASIC state.
    launch_delay = 60;
}

void argv_launch_poll(C64 *machine)
{
    if (!launch_delay || --launch_delay)
        return;
    if (chdir(launch_directory) != 0)
        return;
    machine->Pause();
    bool disk = argv_path_extension(launch_filename, ".d64");
    if (disk)
    {
        machine->RemoveCart();
        strcpy(Drive8File, launch_filename);
        Drive9File[0] = 0;
    }
    else
    {
        strcpy(CartFilename, launch_filename);
        Drive8File[0] = Drive9File[0] = 0;
    }
    LoadGameConfig();
    bLastFileTypeLoaded = disk ? 0 : 1;
    DrivePrefs prefs(TheDrivePrefs);
    strcpy(prefs.DrivePath[0], Drive8File);
    strcpy(prefs.DrivePath[1], Drive9File);
    prefs.TrueDrive = myConfig.trueDrive;
    machine->NewPrefs(&prefs);
    TheDrivePrefs = prefs;
    machine->PatchKernal(TheDrivePrefs.TrueDrive);
    kbd_buf_reset();
    if (disk)
    {
        // Same mount-and-run command as the disk menu; avoid a second cold reset.
        kbd_buf_feed("\rLOAD\"*\",8,1\rRUN\r");
    }
    else
    {
        machine->Reset();
        if (argv_path_extension(launch_filename, ".crt"))
        {
            cart_in = 1;
            bDelayLoadCRT = 5;
        }
        else
        {
            cart_in = 0;
            bDelayLoadPRG = 10;
        }
    }
    ShowKeyboard();
    machine->Resume();
}
