# SPDX-License-Identifier: GPL-2.0-or-later
"""Compile the actual FPS reporting function with mocked camera inputs.

Run with Python 3 and a C compiler on PATH (or set CC). No camera files needed.
"""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[4]
source = (ROOT / 'src/fps-engio.c').read_text()
start = source.index('int fps_get_current_x1000()\n')
end = source.index('static void calc_rolling_shutter', start)
function = source[start:end]
prefix = r"""
#include <assert.h>
#include <stdio.h>
static int lv, movie, video_mode_resolution, video_mode_crop, lv_dispsize, override, video_mode_fps, timer_reads;
static int is_movie_mode(void){return movie;}
static int get_fps_override(void){return override;}
static int get_fps_register_b(void){timer_reads++;return 99;}
#define TIMER_TO_FPS_x1000(t) ((t)==100?22373:0)
"""
tests = r"""
static void reset(void){lv=movie=1;video_mode_resolution=video_mode_crop=override=timer_reads=0;lv_dispsize=1;video_mode_fps=24;}
int main(void){
 reset();lv=0;assert(fps_get_current_x1000()==0&&timer_reads==0);
 #ifdef CONFIG_7D2
 int modes[]={24,25,30,50,60};int rates[]={23976,25000,29970,50000,59940};
 for(int i=0;i<5;i++){reset();video_mode_fps=modes[i];assert(fps_get_current_x1000()==rates[i]&&timer_reads==0);}
 reset();movie=0;assert(fps_get_current_x1000()==22373&&timer_reads==1);
 reset();lv_dispsize=5;assert(fps_get_current_x1000()==22373&&timer_reads==1);
 reset();video_mode_resolution=1;assert(fps_get_current_x1000()==22373&&timer_reads==1);
 reset();video_mode_crop=1;assert(fps_get_current_x1000()==22373&&timer_reads==1);
 reset();override=1;assert(fps_get_current_x1000()==22373&&timer_reads==1);
 reset();video_mode_fps=120;assert(fps_get_current_x1000()==22373&&timer_reads==1);
 puts("PASS: actual 7D2 FPS function, five Canon rates, LV off, photo/zoom/crop/720p/override/unknown fallbacks.");
 #else
 reset();assert(fps_get_current_x1000()==22373&&timer_reads==1);
 puts("PASS: non-7D2 build retains timer-based FPS reporting.");
 #endif
}
"""

with tempfile.TemporaryDirectory(prefix='ml-fps-test-') as directory:
    work = Path(directory)
    src = work / 'fps.c'
    src.write_text(prefix + function + tests)
    for camera in (True, False):
        exe = work / ('fps-7d2.exe' if camera else 'fps-other.exe')
        command = shlex.split(os.environ.get('CC', 'cc'))
        command += ['-std=c99', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function']
        if camera:
            command += ['-DCONFIG_7D2']
        subprocess.run(command + [str(src), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
