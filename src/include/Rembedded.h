/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2006  The R Development Core Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */


#ifndef REMBEDDED_H_
#define REMBEDDED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <R_ext/Boolean.h>

extern int Rf_initEmbeddedR(int argc, char *argv[]);
extern void Rf_endEmbeddedR(int fatal);

/* From here on down can be helpful in writing tailored startup and
   termination code */

#ifndef LibExtern
# define LibExtern extern
#endif

int Rf_initialize_R(int ac, char **av);
void setup_Rmainloop(void);
extern void R_ReplDLLinit(void);
extern int R_ReplDLLdo1();

void R_setStartTime(void);
extern void R_RunExitFinalizers();
extern void CleanEd();
extern void Rf_KillAllDevices();
LibExtern int R_DirtyImage;
extern void R_CleanTempDir();
LibExtern char *R_TempDir;    
extern void R_SaveGlobalEnv(void);


#ifdef unix
void fpu_setup(Rboolean start);
#endif

#ifdef Win32
extern char *getDLLVersion(), *getRUser(), *get_R_HOME();
extern void setup_term_ui(void);
LibExtern int UserBreak;
extern Rboolean AllDevicesKilled;
extern void editorcleanall();
extern int GA_initapp(int, char **);
extern void GA_appcleanup();
extern void readconsolecfg();
#endif

#ifdef __cplusplus
}
#endif

#endif /* REMBEDDED_H_ */
