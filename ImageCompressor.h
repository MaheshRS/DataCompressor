//
//  ImageCompressor.h
//  Constraints
//
//  Created by Mahesh Shanbhag on 11/8/13.
//  Copyright (c) 2013 Mahesh Shanbhag. All rights reserved.
//

#ifndef Constraints_ImageCompressor_h
#define Constraints_ImageCompressor_h

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>
#include <fcntl.h>

#define CHUNK 16384

int def (FILE *source, FILE *destination, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char IN[CHUNK];
    unsigned char OUT[CHUNK];
    
    // allocate the streams
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    ret = deflateInit(&strm, level);
    
    if(ret != Z_OK)
    {
        return ret;
        printf("%s","Failed to allocate");
    }
    
    do
    {
        strm.avail_in = fread(IN, 1, CHUNK, source);
        
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = IN;
        
        do
        {
            strm.avail_out = CHUNK;
            strm.next_out = OUT;
            
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            
            have = CHUNK - strm.avail_out;
            if (fwrite(OUT, 1, have, destination) != have || ferror(destination)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        }while (strm.avail_out == 0);
        
        assert(strm.avail_in == 0);
        
    }while (flush != Z_FINISH);
    
    (void)deflateEnd(&strm);
    return Z_OK;
}

int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char IN[CHUNK];
    unsigned char OUT[CHUNK];
    
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;
    
    do {
        strm.avail_in = fread(IN, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        
        strm.next_in = IN;
        
        /* run inflate() on input until output buffer not full */
        do {
            
            strm.avail_out = CHUNK;
            strm.next_out = OUT;
            
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
            }
            
            have = CHUNK - strm.avail_out;
            if (fwrite(OUT, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
            
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);
    
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
#endif
