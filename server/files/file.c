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

	if (dir)
	{
		return (FILE_DIR_EXISTS);
	}
	else if (error == ENOENT) 
	{
		if (mkdir(dir_name, S_IRWXU | S_IRGRP | S_IXGRP) != 0) 
		{
			return FILE_DIR_CREATE_FAILED;
		}
		else
		{
			return FILE_DIR_EXISTS;
		}
	} 
	return(FILE_DIR_ERROR_OPEN);
}

// only need to use if the name is not null-terminated. 
// use fopen if the name is null terminated to open the file. 
FILE *file_open(char *name, int length, char *mode)
{   
	if (length > FILE_NAME_LENGTH)
	{
		return(NULL);
	}

	char filename[length + 1];
	filename[length] = NULL_ZERO;
	memcpy(filename, name, length);

	FILE *file  = fopen(filename, mode);
	return(file);
}

int file_delete(char *filename)
{
	return (unlink(filename));
}

int file_download(FILE *file, network_s *network, file_write_s *info)
{
	int read_required = 0;
	do
	{
		read_required = info->size - info->index;

		if (read_required > FILE_BUFFER_LENGTH)
		{
			read_required = FILE_BUFFER_LENGTH;
		}

		if (read_required == 0)
		{
			break;
		} 

		network_data_s data = network_read_stream(network, read_required);
	
		if (data.error_code) 
		{
			network_data_free(data);
			return(FILE_NETWORK_ERROR);
		}
	
		int bytes_written   = fwrite(data.data_address, 1
			, data.data_length, file);
	
		if (bytes_written != data.data_length)
		{
			return(FILE_WRITE_ERROR);
		}

		network_data_free(data);
		info->index += data.data_length;
	} 
	while(info->index < info->size);

	fflush(file);
	return(FILE_SUCCESS);    
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
	if (file == NULL)
	{
		return(reader);
	}

	reader.file	= file;
	reader.maxlength	= FILE_BUFFER_LENGTH;
	reader.buffer	= m_malloc(FILE_BUFFER_LENGTH, MEMORY_FILE_LINE);
	reader.stats	= file_read_stat(file);
	return(reader);
}

void file_close_reader(file_reader_s *reader)
{
	if (reader->buffer)
	{
		free(reader->buffer);
	}
}

// file_reader_fill will fill the buffer with file data 
// data will be written at (buffer + fill_at) for fill_size bytes
int file_reader_fill(file_reader_s *reader, long fill_at, long fill_size)
{
	if (reader->is_eof)
	{
		reader->readlength  = -1;
		return (FILE_ERROR);
	}

	int read    = fread(reader->buffer + fill_at, 1, fill_size, reader->file);

	reader->is_eof = feof(reader->file);
	reader->readlength  = read;

	if (read != fill_size &&  reader->is_eof == 0)
	{
		return(FILE_READ_ERROR);
	}
	return(FILE_SUCCESS);
}

string_s file_path_concat(string_s dir1, string_s dir2, string_s file_name)
{
	ulong length	= dir1.length + dir2.length + file_name.length + 1;
	string_s path	= {0, .length = length};
	path.address	= m_calloc(length, MEMORY_FILE_LINE);
	strncpy(path.address, dir1.address, dir1.length);
	strncpy(path.address + dir1.length, dir2.address, dir2.length);
	strncpy(path.address + dir1.length + dir2.length
		, file_name.address, file_name.length);
	return(path);
}

int file_rename(string_s dest, string_s src)
{
	int result	= rename(src.address, dest.address);
	if (result == -1)
	{
		perror("rename");
	}
	return (result == 0 ? SUCCESS : FAILED);
}

int file_append(char *dest, char *src, ulong size)
{
	ulong maxread	= FILE_BUFFER_LENGTH;
	char *buffer	= m_malloc(maxread, MEMORY_FILE_LINE);
	ulong written	= 0;

	FILE *src_file	= fopen(src, "rw+");
	FILE *dest_file	= fopen(dest, FILE_MODE_APPENDONLY);

	if (src_file	== NULL || dest_file == NULL)
	{
		return (FILE_ERROR_OPEN);
	}

	while (written < size)
	{
		ulong read_required	= (size - written) > maxread 
			? maxread 
			: (size - written);

		ulong read_len	= fread(buffer, 1, read_required, src_file);
		
		if (read_len == -1 || read_len != read_required)
		{
			perror("fread");
			return (FILE_READ_ERROR);
		}

		ulong write_len	= fwrite(buffer, 1, read_len, dest_file);

		if (write_len == -1 || write_len != read_len)
		{
			perror("fwrite");
			return (FILE_WRITE_ERROR);
		}

		written += write_len;
	}
	return(FILE_SUCCESS);
}