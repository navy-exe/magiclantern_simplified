/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../fast_probe_7d2.h"
static int calls;
static int correct(const struct fast_7d2_segment *list,unsigned int count)
{
    assert(count>0 && count<=8); calls++;
    for(unsigned int i=0;i<count;i++)
    {
        if(i){assert((const unsigned char*)list[i].src-(const unsigned char*)list[i-1].src==3472);assert((unsigned char*)list[i].dst-(unsigned char*)list[i-1].dst==(int)list[i].bytes);}
        memcpy(list[i].dst,list[i].src,list[i].bytes);
    }
    return 0;
}
static int failed(const struct fast_7d2_segment *list,unsigned int count){(void)list;(void)count;return -3;}
static int bad_guard(const struct fast_7d2_segment *list,unsigned int count){correct(list,count);((unsigned char*)list[0].dst)[-1]=0;return 0;}
static int bad_data(const struct fast_7d2_segment *list,unsigned int count){correct(list,count);((unsigned char*)list[0].dst)[0]^=1;return 0;}
int main(void)
{
    unsigned char *src=malloc(8*3472),*dst=malloc(8*3206+128);assert(src&&dst);
    unsigned int widths[]={1,64,3200,3206};
    for(unsigned int rows=1;rows<=8;rows++)for(unsigned int w=0;w<4;w++)
        assert(fast_7d2_probe(dst,8*3206+128,src,8*3472,rows,widths[w],correct)==0);
    assert(fast_7d2_probe(dst,8*3206+128,src,8*3472,8,3206,failed)==-3);
    assert(fast_7d2_probe(dst,8*3206+128,src,8*3472,8,3206,bad_guard)==-1001);
    assert(fast_7d2_probe(dst,8*3206+128,src,8*3472,8,3206,bad_data)==-1002);
    int before=calls;
    assert(fast_7d2_probe(dst,8*3206+128,src,8*3472,9,3206,correct)==-1000);
    assert(fast_7d2_probe(dst,8*3206+127,src,8*3472,8,3206,correct)==-1000);
    assert(fast_7d2_probe(dst,8*3206+128,src,8*3472-1,8,3206,correct)==-1000);
    assert(fast_7d2_probe(dst,8*3206+128,src,8*3472,8,3207,correct)==-1000);
    assert(fast_7d2_probe(NULL,8*3206+128,src,8*3472,8,3206,correct)==-1000);
    assert(fast_7d2_probe(src,8*3206+128,src,8*3472,8,3206,correct)==-1000);
    assert(fast_7d2_probe(src+128,8*3206+128,src,8*3472,8,3206,correct)==-1000);
    assert(fast_7d2_overlap(100,10,109,10));
    assert(!fast_7d2_overlap(100,10,110,10));
    assert(fast_7d2_overlap(109,10,100,10));
    assert(calls==before);
    puts("PASS: 32 descriptor geometries, exact data/guards, transfer error propagation, corruption detection, rejected capacities/counts.");
    free(src);free(dst);
    size_t source_bytes=1068u*3472, destination_bytes=1048u*3206+128;
    src=malloc(source_bytes);dst=malloc(destination_bytes);assert(src&&dst);
    for(size_t i=0;i<source_bytes;i++)src[i]=(unsigned char)(i*37+i/257+81);
    int heights[]={8,688,800,1032,1048};
    for(unsigned int h=0;h<5;h++)
    {
        size_t bytes=(size_t)heights[h]*3206;
        memset(dst,0xa5,destination_bytes);calls=0;
        assert(fast_7d2_frame(dst+64,bytes,src,source_bytes,1068,20,heights[h],correct)==0);
        assert(calls==heights[h]/8);
        for(int row=0;row<heights[h];row++)assert(!memcmp(dst+64+(size_t)row*3206,src+(size_t)(row+20)*3472+252,3206));
        for(int i=0;i<64;i++)assert(dst[i]==0xa5&&dst[64+bytes+i]==0xa5);
    }
    calls=0;
    assert(fast_7d2_frame(dst+64,688u*3206,src,source_bytes,1068,200,688,failed)==-3);
    assert(fast_7d2_frame(dst+64,688u*3206-1,src,source_bytes,1068,200,688,correct)==-1000);
    assert(fast_7d2_frame(dst+64,688u*3206,src,source_bytes-1,1068,200,688,correct)==-1000);
    assert(fast_7d2_frame(dst+64,destination_bytes-64,src,source_bytes,1068,22,1048,correct)==-1000);
    assert(fast_7d2_frame(dst+64,destination_bytes-64,src,source_bytes,1068,21,688,correct)==-1000);
    assert(fast_7d2_frame(dst+64,destination_bytes-64,src,source_bytes,1068,20,687,correct)==-1000);
    assert(calls==0);
    for(size_t i=0;i<source_bytes;i++)assert(src[i]==(unsigned char)(i*37+i/257+81));
    puts("PASS: full-frame descriptor chains at 8/688/800/1032/1048 rows, exact packed bytes, guards, call counts, source preservation, error propagation and rejected capacities/geometry.");
    free(src);free(dst);return 0;
}
