/****************************************************************************** 
Copyright (c) 2008-2015, Teledyne DALSA Inc.
All rights reserved.

File : gev_xml_utils.c
	XML Utiltiy functions for GEV C library.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:
	-Redistributions of source code must retain the above copyright 
	notice, this list of conditions and the following disclaimer. 
	-Redistributions in binary form must reproduce the above 
	copyright notice, this list of conditions and the following 
	disclaimer in the documentation and/or other materials provided 
	with the distribution. 
	-Neither the name of Teledyne DALSA nor the names of its contributors 
	may be used to endorse or promote products derived 
	from this software without specific prior written permission. 

===============================================================================
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/
/*! \file gev_xml_utils.c
\brief XML Utility Functions.

*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define __USE_GNU
#include <string.h>
#include <sys/wait.h>


#include "gevapi.h"
#include "gevapi_internal.h"

#include "GenICamVersion.h"


#define XML_URL_LENGTH 0x200

#if COR_LINUX
	#define STRNICMP_FUNC	strncasecmp
#endif
#if COR_WIN32
	#define STRNICMP_FUNC	_strnicmp
#endif

//=============================================================================
// Static functions to help with recursive directory manipulation.
//
static int _make_dir( char *path, mode_t mode )
{
	struct stat st;
	int status = -1;
	
	if (path != NULL)
	{
		status = 0;
		if ( stat(path, &st) != 0)
		{
			// Dir dies not exist.
			if (mkdir(path, mode) != 0)
			{
				status = -1;
			}
		}
		else if ( !S_ISDIR(st.st_mode))
		{
			errno = ENOTDIR;
			status = -1;
		}
	}
	return status;
}

static int _make_path( char *path, mode_t mode )
{
	char *pp;
	char *sp;
	int status = -1;
	int len = 0;
	char *copy = NULL;

	if (path != NULL)
	{
		// Duplicate the path string.
		len = strlen(path);
		copy = (char *) malloc(len+1);

		if ( copy != NULL )
		{
			status = 0;
			strncpy(copy, path, len);
			copy[len] = '\0';
			pp = copy;
			while(status == 0 && (sp = strchr(pp, '/')) != 0)
			{
				if (sp != pp)
				{
					// No root or double delimiter (slash) in path
					*sp = '\0';
					status = _make_dir( copy, mode );
					*sp = '/';
				}
				pp = sp + 1;
			}

			if (status == 0)
			{
				status = _make_dir( path, mode);
			}
			free(copy);
		}
	}	
	return status;
}



//=============================================================================
//
//! Extract the XML URL information. (Internal function).
//
/*!
	This function takes the XML URL string and decodes it to obtain the
	XML file name, the address at which it is stored, and the length of
	the XML data.	
	The URL string must be in the form 
		"Local:<filename>.<extension>,<address>,<length>"
	where both the address and length values are hexadecimal.

	(The <extension> can be either xml or zip.)

	\param [in]	urlBuffer	Pointer to a string containing the URL.
	\param [in] xmlFileName	Pointer to a string for the decoded XML file name.
	\param [in] addr_ptr		Pointer to an integer to hold the decoded on-board 
									camera address for the XML data. If NULL, the 
									address information is not returned.
	\param [in] len_ptr		Pointer to an integer to hold the decoded length of 
									the on-board camera XML data. If NULL, the length 
									information is not returned.
	\return Status return.
	\note Note: There is not yet support for URLs that are not in the camera since
			the standard mandates that they should be in the camera.
*/	

static GEV_STATUS _Extract_XML_Url_Info( char *urlBuffer, char *xmlFileName, UINT32 *addr_ptr, UINT32 *len_ptr)
{
	int status = GEVLIB_OK;

	if ((urlBuffer != NULL)	&& (xmlFileName != NULL) && (addr_ptr != NULL) && (len_ptr != NULL))
	{
		char *token = NULL;
		char *context;
		char *url = NULL;
		int i;	
		unsigned int address = 0;
		unsigned int length = 0;
		char *delim = "\\:,;";

		// Parse the input URL looking for a format of the form:
		//  "Local:FileName.Extension;address;length"
   	url = urlBuffer;
		token = strtok_r( url, delim, &context);
		if (token != NULL)
		{
			for (i = 0; i < strlen(token); i++)
			{
				int tc = (int)token[i];
				token[i] = (char)tolower(tc);
			}
			if (strcmp(token, "local") == 0)
			{
				// Url potentially contains a proper file name.
				token = strtok_r( NULL, delim, &context);
				if ( token != NULL)
				{
					strcpy(xmlFileName, token);

					// Find the address
					token = strtok_r( NULL, delim, &context);
					if (token != NULL)
					{
						// Pull the address out of here (as a hex or an integer)
						if ( (strstr(token, "0x") != NULL) || (strstr(token, "0X") != NULL))
						{
							if (sscanf(token, "%x", &address) == 0)
							{
								status =  GEVLIB_ERROR_PARAMETER_INVALID;  // Some sort of invalid formatting error.
							}
							*addr_ptr = address;
						}
						else
						{
							if (sscanf(token, "%x", &address) == 0)
							{
								status =  GEVLIB_ERROR_PARAMETER_INVALID; // Some sort of invalid formatting error.
							}
							*addr_ptr = address;
						}

						// Find the length (NULL terminated remainder)...
						token = strtok_r( NULL, delim, &context);		
						if (token != NULL)
						{
							// Pull the length out of here (as a hex or an integer).
							if ( (strstr(token, "0x") != NULL) || (strstr(token, "0X") != NULL))
							{
								if (sscanf(token, "%x", &length) == 0)
								{
									status =  GEVLIB_ERROR_PARAMETER_INVALID; // Some sort of invalid formatting error.
								}
								*len_ptr = length;
							}
							else
							{
								if (sscanf(token, "%x", &length) == 0)
								{
									status =  GEVLIB_ERROR_PARAMETER_INVALID; // Some sort of invalid formatting error.
								}
								*len_ptr = length;
							}
						}
					}
				}
			}
		}
	}
	return status;
}



//=============================================================================
//
//! Retrieve info about the GenICam XML file for the camera.
//
/*!
	This function retrieves the name of the XML file to be used for the camera
	from the camera itself. It also retreives the camera address to find the
	XML data and the length of the XML data.

	\param [in]	handle		Handle to the camera
	\param [in] file_name	Pointer to a string to receive the XML file name.
	\param [in] size			Number of bytes available to store the file name.
	\param [in] address		Pointer to an integer to hold the on-board camera 
									address for the XML data. If NULL, the address 
									information is not returned.
	\param [in] length		Pointer to an integer to hold length of the on-board 
									camera XML data. If NULL, the length information is 
									not returned.
	\return Status return.
	\note None
*/	

GEV_STATUS Gev_RetrieveXMLInfo( GEV_CAMERA_HANDLE handle, char *file_name, int size, int *address, int *length )
{
   GEV_STATUS status = GEVLIB_OK;
	UINT32 addr = 0;
	UINT32 len = 0;
	UINT32 firstUrl = 0;
	UINT32 secondUrl = 0;
	UINT32 urlLength = 0;
   char urlBuffer[ XML_URL_LENGTH ];
   char xmlFileName[ XML_URL_LENGTH ];

	// Obtain the URL addresses for the XML file.
	status = _GetXMLFileUrlLocations( handle, &firstUrl, &secondUrl, &urlLength);
	
	if (status == GEVLIB_OK)
	{
		// Read the first XML url from the camera.
		status = Gev_ReadMem(handle, firstUrl, urlBuffer,  urlLength);
		
		if (status == GEVLIB_OK)
		{
			// Decode the first url. It is required to be of the form >> "Local:FileName.Extension;address;length"
			status = _Extract_XML_Url_Info( urlBuffer, xmlFileName, &addr, &len);
			LogPrint( GEV_LOG_INFO, __FILE__, __LINE__, "XML Filename = %s, at address 0x%x, size is 0x%x bytes\n", xmlFileName, addr, len);

			if (status == GEVLIB_OK)
			{
				if (address != NULL)
				{
					*address = addr;
				}
				if (length != NULL)
				{
					*length = len;
				}
				if (file_name != NULL)
				{
					if (size >= strlen(xmlFileName))
					{
						strcpy(file_name, xmlFileName);
					}
					else
					{
						status = GEVLIB_ERROR_ARG_INVALID;
						LogPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "XML file name too long for string argument"); 
					}
				}
			}
			else
			{
				LogPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "Error decoding first URL for XML file"); 
			}
		}
		// See if we succeeded in getting the first one. If not, try the second one.
		if (status != GEVLIB_OK)
		{
			// Read the second XML url from the camera.
			status = Gev_ReadMem(handle, secondUrl, urlBuffer,  urlLength);
			
			if (status == GEVLIB_OK)
			{
				// Decode the first url. It is required to be of the form >> "Local:FileName.Extension;address;length"
				status = _Extract_XML_Url_Info( urlBuffer, xmlFileName, &addr, &len);
				LogPrint( GEV_LOG_INFO, __FILE__, __LINE__, "XML Filename = %s, at address 0x%x, size is 0x%x bytes\n", xmlFileName, addr, len);

				if (status == GEVLIB_OK)
				{
					if (address != NULL)
					{
						*address = addr;
					}
					if (length != NULL)
					{
						*length = len;
					}
					if (file_name != NULL)
					{
						if (size >= strlen(xmlFileName))
						{
							strcpy(file_name, xmlFileName);
						}
						else
						{
							status = GEVLIB_ERROR_ARG_INVALID;
							LogPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "XML file name too long for string argument"); 
						}
					}
				}
				else
				{
					LogPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "Error decoding second URL for XML file"); 
				}
			}
		}
	}
	return status;
}

//=============================================================================
//
//! Retrieve the GenICam XML data from the camera.
//
/*!
	This function retrieves the XML data to be used for the camera from the
	camera itself. The data is returned in a buffer.
	If the input buffer pointer is NULL, the function will return the required
	size of the XML data buffer.

	\param [in]	handle		Handle to the camera
	\param [in] size			Size of the the xml_data buffer.
	\param [in] xml_data		Pointer to storage to hold the XML data from 
									the camera. 
	\param [in] num_read		Pointer to hold the number of bytes read from the 
									camera. (If xml_data is NULL, the required buffer
									size is returned here).
	\param [in] data_is_compressed
									Pointer to hold a flag indocating if the returned 
									XML data is compressed (1 for true) or not 
									(0 for false). (Compression uses ZIP) 
	\return Status return.
	\note Care should be taken with the returned data. The data may be compressed
		if the onboard file is a ".zip" file instead of being a ".xml" file.
*/	
GEV_STATUS Gev_RetrieveXMLData( GEV_CAMERA_HANDLE handle, int size, char *xml_data, int *num_read, int *data_is_compressed )
{
   GEV_STATUS status = GEVLIB_ERROR_NULL_PTR;
	int addr = 0; 
	int len = 0;
   char xmlFileName[ XML_URL_LENGTH ];

	if (num_read != NULL)
	{
		// Get the info about the XML data.
		status = Gev_RetrieveXMLInfo( handle, xmlFileName, sizeof(xmlFileName), &addr, &len);
		if (status == GEVLIB_OK)
		{
			if (data_is_compressed != NULL)
			{
				if ( strcasestr(xmlFileName, ".zip") != NULL)
				{
					*data_is_compressed = 1;  // True - zip data.
				}
				else
				{
					*data_is_compressed = 0;  // False - standard data.
				} 
			}
			if (xml_data == NULL)
			{
				// Return the required size to the caller.
				*num_read = len;
			}
			else
			{
				int pad = 4 - (len & 3);
				if (pad == 4) pad = 0;
				
				if (size >= (len+pad))
				{

					// Check if buffer pointer and size are
					// Read the data from the camera.
					status = Gev_ReadMem( handle, addr, xml_data, (len+pad));
					if (status == GEVLIB_OK)
					{
						*num_read = len;
					}
				}
				else
				{
					status = GEVLIB_ERROR_PARAMETER_INVALID;
				}
			}
		}
	}
	return status;
}
//=============================================================================
//
//! Retrieve the GenICam XML file for the camera.
//
/*!
	This function retrieves the name of the XML file to be used for the camera.
	If the XML file has not yet been downloaded from the camera, it is downloaded
	and stored in the subdirectory 'xml/<manufacturer>' of the installation 
	directory pointed to by the GIGEV_XML_DOWNLOAD environment variable.

	If the GIGEV_XML_DOWNLOAD environment variable is not set, the XML file is
	stored in the 'xml/<manufacturer>' subdirectory of the program executing.

	Generally, once the XML file is already on the local disk, it is not 
	downloaded again. If the "force_download" flag is set, the XML file is 
	downloaded, regardless of whether it is on the disk or not.

	\param [in]	handle			Handle to the camera
	\param [in] file_name		Pointer to a string to receive the XML file path name.
	\param [in] size				Number of bytes available to store the file path name.
	\param [in] force_download	If TRUE, the XML file is always downloaded from the camera
										overwriting the file on disk.
	\return Status return.
	\note None
*/	

GEV_STATUS Gev_RetrieveXMLFile( GEV_CAMERA_HANDLE handle, char *file_name, int size, BOOL force_download )
{
   GEV_STATUS status = GEVLIB_OK;
   int nbRead;
	UINT32 addr = 0;
	UINT32 length = 0;
   char xmlFileName[ XML_URL_LENGTH ];
   char xmlFilePath[ XML_URL_LENGTH ];
   char xmlFullFilePath[ MAX_PATH ];
	GEV_CAMERA_INFO *info = NULL;
	BOOL	download_from_camera = FALSE;


	// Get the camera info (need the manufacturers name).
	info = GevGetCameraInfo( handle);
	if (info == NULL)
	{
		status = GEVLIB_ERROR_INVALID_HANDLE;
		return status;
	}

	// Read the XML info from the camera.
	status = Gev_RetrieveXMLInfo( handle, xmlFileName, sizeof(xmlFileName), (int *)&addr, (int *)&length);
   if( status == GEVLIB_OK )
   {
		// Do a quick sanity check for the XML info.
		if ( (length == 0) || (addr == 0))
		{
			// Length and/or address is 0 - bad data in camera.
			status = GEVLIB_ERROR_RESOURCE_INVALID;
      	LogPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "Camera's First URL for XML file is not valid"); 
			return status;
 		}

		// Get the standard path for storing the XML file.
		if (GetEnvironmentVariable( GIGEV_XML_DOWNLOAD, xmlFilePath, sizeof(xmlFilePath)) != 0)
		{
			// Add the sub-path "/xml/DALSA" to that found from the environment variable.
			// Check the end of the path from the env variable.
			int len = strlen( xmlFilePath);
			if (xmlFilePath[len-1] != '/')
			{
				strcat(xmlFilePath, "/");
			}
			strcat(xmlFilePath, "xml/");
			strcat(xmlFilePath, info->manufacturer);
		}
		else
		{
			// Environment variable was not found.
			// We will use "./xml/DALSA" as the directory (relative to the current directory).
			strcpy(xmlFilePath, "xml/");
			strcat(xmlFilePath, info->manufacturer);
		}

		// Try to open the file indicated by the camera. (a ".zip" or a ".xml")
		strcpy( xmlFullFilePath, xmlFilePath );
		strcat( xmlFullFilePath, "/" );
		strcat( xmlFullFilePath, xmlFileName );
		if ( CheckFileExists( xmlFullFilePath, FALSE ) )
		{
			// File exists, are we able to overwrite it if required?
			if ( force_download )
			{
				if ( CheckFileExists( xmlFullFilePath, TRUE ) )
				{
					download_from_camera = TRUE;
				}
				else
				{
					// Cannot overwrite - a permission issue!!! 
					// Use the file that is already there.
					status = GEVLIB_ERROR_INSUFFICIENT_PRIVILEGE;
				}			
			}	
		}
		else
		{
			// File does not exist - we must download it.
			download_from_camera = TRUE;
		}	

		// See if we need to force a download from the camera
		if ( download_from_camera )
		{
			char *ext = NULL;
			char *xmlData = NULL;
			int pad = 4 - (length & 3);
			if (pad == 4) pad = 0;

			// XML file not present
			// Allocate a buffer to hold the data.
			xmlData = (char *)malloc(length+pad);
			if (xmlData != NULL)
			{
				int remainder = length + pad; // Multiple of 4 bytes.

				// Read the XML file from the camera.
				status = Gev_RetrieveXMLData( handle, remainder, xmlData, &nbRead, NULL );
				if (status == GEVLIB_OK)
				{
					FILE *fp = NULL;
					// Write XML data to a file on disk.
					// Create the path for the file (ignor the error in case it is already there).
					status = _make_path( xmlFilePath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH  );					
	
					//Open
					fp = fopen(xmlFullFilePath, "w");
					if (fp != NULL)
					{
						//Write
						fwrite(xmlData, 1, length, fp);

						//Close
						fclose(fp);
					}

					// See if the file was a zip file.
					if ( (ext = strstr(xmlFullFilePath, ".zip")) != NULL)
					{
						// Unzip the file in its current directory and pipe it to create the
						// output file with same name (just different extension).
						char command[MAX_PATH] = {0};
						char xmlFullOutputPath[ MAX_PATH ];
						strncpy(xmlFullOutputPath, xmlFullFilePath, MAX_PATH);
						
						// Set up the name of the XML file to contain the unzipped data (via pipe)..
						// (It must have the same name as the zip file - just a different extension)
						strcpy(ext, ".xml");		
						
						//sprintf( command, "unzip -d \"%s\" \"%s\" > \"%s\"", xmlFilePath, xmlFullOutputPath, xmlFullFilePath);
						sprintf( command, "unzip -p \"%s\" > \"%s\"", xmlFullOutputPath, xmlFullFilePath );
						status = system(command);

						// Check if the unzip command worked
						if (WEXITSTATUS(status) != 0)
						{
							LogPrint( GEV_LOG_ERROR, __FILE__, __LINE__,"Error unzipping file %s : OS status = %d\n", xmlFullFilePath, WEXITSTATUS(status));
						}
						else
						{
							// Delete the zip file.
							sprintf( command, "rm \"%s\"", xmlFullOutputPath );
							status = system(command);
						}

					}
				}
				free(xmlData);
			}
			else
			{
				status = GEVLIB_ERROR_INSUFFICIENT_MEMORY;
      		LogPrint( GEV_LOG_ERROR, __FILE__, __LINE__, "Error reading XML from camera (no memory)"); 
				return status;
			}
		}

		// Copy the found file name back to the caller.
		strncpy( file_name, xmlFullFilePath, size );
	}

	return status;
}


