// SPDX-License-Identifier: MIT
#include "emulator_stubs.h"
#include "argv_launch.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
DrivePrefs TheDrivePrefs;
Config myConfig;
char Drive8File[256], Drive9File[256], CartFilename[256];
int bDelayLoadPRG, bDelayLoadCRT;
u8 cart_in, bLastFileTypeLoaded;
static int resets, configs, menus, prefs_changed, pauses, resumes;
static char command[128];
void C64::RemoveCart() { CartFilename[0] = 0; cart_in = 0; }
void C64::NewPrefs(DrivePrefs*) { ++prefs_changed; }
void C64::PatchKernal(bool) {}
void C64::Reset() { ++resets; }
void C64::Pause() { ++pauses; }
void C64::Resume() { ++resumes; }
void LoadGameConfig() { ++configs; myConfig.trueDrive = true; }
void ShowKeyboard() { ++menus; }
void kbd_buf_reset() { command[0] = 0; }
void kbd_buf_feed(const char *text) { strcpy(command, text); }
static void reset() {
    resets = configs = menus = prefs_changed = pauses = resumes = 0;
    bDelayLoadPRG = bDelayLoadCRT = 0;
    Drive8File[0] = Drive9File[0] = CartFilename[0] = command[0] = 0;
}
int main() {
    char root[] = "/tmp/gimli-argv-XXXXXX";
    assert(mkdtemp(root));
    char previous[1024]; assert(getcwd(previous, sizeof(previous)));
    C64 machine;
    const char *names[] = {"My Game.PRG", "My Cart.CRT", "My Disk.D64"};
    for (int kind = 0; kind < 3; ++kind) {
        reset(); assert(chdir(root) == 0);
        FILE *file = fopen(names[kind], "wb"); assert(file); fputs("Synthetic fixture", file); fclose(file);
        char app[] = "GimliDS.nds", argument[64]; strcpy(argument, names[kind]);
        char *args[] = {app, argument};
        argv_launch_capture(2, args);
        argv_launch_remember_cwd();
        assert(chdir("/") == 0); // The normal browser has changed directory.
        if (kind == 0) argument[0] = 0; // Simulate loader-owned memory being reused.
        argv_launch_prepare(2, args);
        for (int frame = 0; frame < 59; ++frame) argv_launch_poll(&machine);
        assert(configs == 0);
        argv_launch_poll(&machine);
        assert(configs == 1 && prefs_changed == 1 && menus == 1 && pauses == 1 && resumes == 1);
        assert(kind == 0 ? !argument[0] : !strcmp(argument, names[kind]));
        if (kind == 0) assert(bDelayLoadPRG == 10 && !strcmp(CartFilename, names[kind]) && resets == 1);
        if (kind == 1) assert(bDelayLoadCRT == 5 && cart_in == 1 && resets == 1);
        if (kind == 2) assert(!strcmp(Drive8File, names[kind]) && !strcmp(command, "\rLOAD\"*\",8,1\rRUN\r") && resets == 0);
        for (int frame = 0; frame < 90; ++frame) argv_launch_poll(&machine);
        assert(configs == 1); // Never relaunch after returning to the menus.
        unlink(names[kind]);
    }
    reset(); argv_launch_capture(0, nullptr); argv_launch_prepare(0, nullptr);
    for (int frame = 0; frame < 90; ++frame) argv_launch_poll(&machine);
    assert(configs == 0);
    assert(chdir(previous) == 0); rmdir(root);
}
