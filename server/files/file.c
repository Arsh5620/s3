#include "file.h"
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../memdbg/memory.h"

//file dir make if not exists
int file_dir_mkine(char *dir_name)
{
	DIR *dir = opendir(dir_name);
	int error = errno;
	closedir(dir);

	if(dir) {
		return(FILE_DIR_EXISTS);
	}
	else if (error == ENOENT) {
		if(mkdir(dir_name, S_IRWXU | S_IRGRP | S_IXGRP) != 0) {
			return FILE_DIR_CREATE_FAILED;
		}
		else return(FILE_DIR_EXISTS);
	} 
	return(FILE_DIR_ERR_OPEN);
}

// only need to use if the name is not null-terminated. 
// use fopen if the name is null terminated to open the file. 
FILE *file_open(char *name, int length, char *mode)
{   
	if(length > FILE_NAME_MAXLENGTH)
		return(NULL);

	char filename[length + 1];
	filename[length] = 0;

	memcpy(filename, name, length);

	FILE *file  = fopen(filename, mode);
	return(file);
}

int file_delete(char *filename)
{
	return(unlink(filename));
}

int file_download(FILE *file
					, netconn_info_s *network
					, file_write_s *info)
{
	int read_required = 0;
	do{
		read_required = info->size - info->current;
		if(read_required > FILE_UPLOAD_BUFFER)
			read_required = FILE_UPLOAD_BUFFER;
	
		if(read_required == 0) break;
		netconn_data_s data_read = 
			network_data_readstream(network, read_required);
	
		void *data_address  = network_data_address(&data_read);
		if(data_read.is_error) {
			network_data_free(data_read);
			return(FILE_UPLOAD_ERR);
		}
	
		int bytes_written   = 
			fwrite(data_address, 1, data_read.data_length, file);
		fflush(file);
	
		if(bytes_written != data_read.data_length){
			return(FILE_UPLOAD_ERR);
		}
		network_data_free(data_read);
		info->current += data_read.data_length;
	} while(info->current < info->size);
	return(FILE_UPLOAD_COMPLETE);    
}

struct stat file_read_stat(FILE *file)
{
	struct stat file_stat = {0};
	fstat(file->_fileno, &file_stat);
	return(file_stat);
}

// file_init_reader sets up memory for an open file to be 
// read as a stream so that the entire file does not need to 
// be loaded into the memory. 
file_reader_s file_init_reader(FILE *file)
{
	file_reader_s reader = {0};
	if(file == NULL)
		return(reader);

	reader.file = file;
	reader.maxlength    = FILE_READER_BUFFERLENGTH;

	reader.buffer   = 
		m_malloc(FILE_READER_BUFFERLENGTH, MEMORY_FILE_LINE);

	if(reader.buffer == NULL)
		return(reader);

	reader.stats   = file_read_stat(file);
	return(reader);
}

void file_close_reader(file_reader_s *reader)
{
	if(reader->buffer)
		free(reader->buffer);

	if(reader->file)
		fclose(reader->file);
}

// file_reader_fill will fill the buffer with file data 
// data will be written at (buffer + fill_at) for fill_size bytes
int file_reader_fill(file_reader_s *reader, long fill_at, long fill_size)
{
	if(reader->is_eof){
		reader->readlength  = -1;
		return(FILE_READER_UNSUCCESSFUL);
	}

	int read    =  
		fread(reader->buffer + fill_at, 1, fill_size, reader->file);

	reader->is_eof = feof(reader->file);
	reader->readlength  = read;
	if(read != fill_size &&  reader->is_eof == 0)
		return(FILE_READER_UNSUCCESSFUL);

	return(FILE_READER_SUCCESS);
}

