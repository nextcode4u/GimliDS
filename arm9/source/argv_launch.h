// SPDX-License-Identifier: MIT
#ifndef ARGV_LAUNCH_H
#define ARGV_LAUNCH_H
class C64;
void argv_launch_capture(int argc, char **argv);
void argv_launch_remember_cwd(void);
void argv_launch_prepare(int argc, char **argv);
void argv_launch_poll(C64 *machine);
#endif
