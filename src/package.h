

/**********************************************************
*
*  returns 0 on success
*
*  archive:  path of the archived and compressed data
*  filename: a pointer to an array of filrenames to
*            be archived and compressed
*
**********************************************************/
int package(const char *archive, char const* const*filename);


/**********************************************************
*
*  returns 0 on success
*
*  packageName:  the name of the package to be decompressed
*                and unarchived
*
**********************************************************/
int depackage(const char *packageName);