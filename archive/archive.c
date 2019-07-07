#include <sys/types.h>
#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <assert.h>
#include <zlib.h>

#include <string.h>

#define CHUNK 32768 // 256K


/* 
  Compress from file source to file dest until EOF on source.
  def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
  allocated for processing, Z_STREAM_ERROR if an invalid compression
  level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
  version of the library linked do not match, or Z_ERRNO if there is
  an error reading or writing the files.
*/

int compressFile(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // allocate deflate state 
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    // compress until end of file
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);

        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }

        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        // run deflate() on input until output buffer not full, finish
        // compression if all of source has been read in
        do {

            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = deflate(&strm, flush);    // no bad return value
            assert(ret != Z_STREAM_ERROR);  // state not clobbered

            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }

        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

    /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/*
  Decompress from file source to file dest until stream ends or EOF.
  inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
  allocated for processing, Z_DATA_ERROR if the deflate data is
  invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
  the version of the library linked do not match, or Z_ERRNO if there
  is an error reading or writing the files. 
*/
int decompressFile(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    // decompress until deflate stream ends or end of file
    do {

        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        // run inflate() on input until output buffer not full
        do {

            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  // state not clobbered
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     // and fall through
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }

            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }

        } while (strm.avail_out == 0);

    // done when inflate() says it's done
    } while (ret != Z_STREAM_END);

    // clean up and return
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

void write_archive(const char *outname, const char **filename)
{
    struct archive *a;
    struct archive_entry *entry;
    struct stat st;
    char buff[8192];
    int len;
    int fd;

    // initialize archive structure
    a = archive_write_new();
    //archive_write_add_filter_gzip(a);  use zlib for compression, dont use this
    archive_write_set_format_pax_restricted(a);
    archive_write_open_filename(a, outname);

    // initialize archive entry 
    entry = archive_entry_new();

    while (*filename) {
        stat(*filename, &st);
        archive_entry_set_pathname(entry, *filename);
        //archive_entry_set_size(entry, st.st_size); // Note 3
        archive_entry_copy_stat(entry, &st);
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_write_header(a, entry);

        fd = open(*filename, O_RDONLY);
        len = read(fd, buff, sizeof(buff));

        while ( len > 0 ) {
            archive_write_data(a, buff, len);
            len = read(fd, buff, sizeof(buff));
        }

        filename++;
        close(fd);
        archive_entry_clear(entry);
    }

    // cleanup
    archive_entry_free(entry);
    archive_write_close(a);
    archive_write_free(a);
}



int copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
    off_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
          return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
          return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
          fprintf(stderr, "%s\n", archive_error_string(aw));
          return (r);
        }
    }
}

void extract(const char *filename)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;

    // Select which attributes we want to restore.
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    // initialize archive
    a = archive_read_new();
    archive_read_support_format_all(a);
    //archive_read_support_compression_all(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    if ((r = archive_read_open_filename(a, filename, 10240))){
        exit(1);
    }

    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(a));
        if (r < ARCHIVE_WARN){
            exit(1);
        }

        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(ext));
            if (r < ARCHIVE_WARN){
                exit(1);
            }
        }

        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
        if (r < ARCHIVE_WARN){
            exit(1);
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    //exit(0);
}


int package(const char *archive, const char **filename){
    int ret;

    write_archive("temp", filename);
    
    FILE* source = fopen("temp", "rb");
    FILE* dest = fopen(archive, "wb");

    // 9 is the highest compression value
    // we want the file as small as possible for
    // transmission
    ret = compressFile(source, dest, 9);

    fclose(source);
    fclose(dest);

    if (ret != Z_OK)
        zerr(ret);
    return ret;
}

int depackage(const char *packageName){
    int ret;
    
    FILE* source = fopen(packageName, "rb");
    FILE* dest = fopen("temp", "wb");

    ret = decompressFile(source, dest);

    fclose(source);
    fclose(dest);
    if (ret != Z_OK)
        zerr(ret);

    extract("temp");

    return ret;
}


/****************************************
*
*  Usage: -c compression
*         -e extraction
*
*  for comepression first argument after
*  flag is out file, next arguments are
*  files to compress
*
*  extraction, only takes one argument
*  after flag and thats compressed file
*
****************************************/
int main(int argc, const char **argv)
{
  const char *outname;
  const char *flag;
  
  argv++;
  flag = *argv++;
  outname = *argv++;

  if (strcmp ("-c", flag) == 0){
    package(outname, argv);
  }
  else if (strcmp ("-e", flag) == 0){
    depackage(outname);
  }
  
  return 0;
}