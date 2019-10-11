#include <archive.h>
#include <archive_entry.h>
#include <iostream>
#include <fstream>
#include "archiver.h"

#define DEBUG 0

Archiver::Archiver()
{
	this->name = "";
}

Archiver::Archiver(const string& name)
{
	this->name = name;
}

Archiver::Archiver(const string& name, const vector<string>& filenames)
{
	this->name = name;
	this->filenames = filenames;
}

void Archiver::setup(const string& name)
{
	this->name = name;
}

void Archiver::setup(const string& name, const vector<string>& filenames)
{
	this->name = name;
	this->filenames = filenames;
}

int Archiver::archive()
{
	struct archive *a;
    struct archive_entry *entry;
    struct stat st;
    vector<char> buff(8192);

    if (this->name.empty() || this->filenames.empty())
    	return 0;

    // initialize archive structure
    a = archive_write_new();
    //archive_write_add_filter_gzip(a);  use zlib for compression, dont use this
    archive_write_set_format_pax_restricted(a);
    archive_write_open_filename(a, this->name.c_str());

    // initialize archive entry 
    entry = archive_entry_new();

    for (auto& filename : this->filenames)
    {
    	stat(filename.c_str(), &st);
        archive_entry_set_pathname(entry, filename.c_str());
        //archive_entry_set_size(entry, st.st_size); // Note 3
        archive_entry_copy_stat(entry, &st);
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_write_header(a, entry);

        ifstream ifs{filename, ifstream::binary};
        while (!ifs.eof() && !ifs.fail())
        {
        	ifs.read (&buff[0], buff.size());
        	archive_write_data(a, &buff[0], ifs.gcount());
        }
        archive_entry_clear(entry);
    }

    // cleanup
    archive_entry_free(entry);
    archive_write_close(a);
    archive_write_free(a);
    return 0;
}

int Archiver::extract()
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

    if ((r = archive_read_open_filename(a, this->name.c_str(), 10240))){
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
            r = copyArchive(a, ext);
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
	return 0;
}

int Archiver::copyArchive(struct archive *ar, struct archive *aw)
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



#if DEBUG == 1
int main()
{

	return 0;
}
#endif