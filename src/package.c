/**********************************************************
**
**   Author: Lorant Polya
**
**   This file archives a folder and
**   compresses it using compress.h lib.
**   After the file is compressed it is 
**   ready to be transported.
**   This file is also resposible for
**   decompressing the transported file
**   and unarchiving it.
**
**   https://github.com/libarchive/libarchive/wiki/Examples
**
**********************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compress.h"

#define DEBUG 0

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
#if DEBUG == 1
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
    else{
        printf("Usage: ./a.out [-c|-e] [package_name] [files_to_package] ...\n");
    }

    return 0;
}
#endif