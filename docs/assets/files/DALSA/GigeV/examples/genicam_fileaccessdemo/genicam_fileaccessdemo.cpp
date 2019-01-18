#include "stdio.h"
#include "cordef.h"
#include "GenApi/GenApi.h"		//!< GenApi lib definitions.
#include "gevapi.h"				//!< GEV lib definitions.
#include <sched.h>
#include <ctype.h>

using namespace std;
using namespace GenICam;
using namespace GenApi;

#define MAX_NETIF					8
#define MAX_CAMERAS_PER_NETIF	32
#define MAX_CAMERAS		(MAX_NETIF * MAX_CAMERAS_PER_NETIF)


#define MAX_CAMERA_FILENAME_LENGTH 512
#define MAX_NUM_CAMERA_FILES	128

typedef struct 
{
	char 		file_id[MAX_CAMERA_FILENAME_LENGTH];
	uint64_t file_index;
	int  		read_access;
	int  		write_access;
	int  		available;		// File location is available (may or may not have data present)
	int  		present;			// Readable file is present
} GENICAM_FILE_INFO, *PGENICAM_FILE_INFO;


GENICAM_FILE_INFO _cam_file_info[MAX_NUM_CAMERA_FILES] = {0};


int gev_command(GEV_CAMERA_HANDLE handle, const char *command)
{
	int status = 1;
	int done = 0;
	int type = 0;
	
	// Issue the command
	int cmd = 1;
	status = GevSetFeatureValue( handle, (char *)command, sizeof(UINT32), &cmd);

	if (status == 0)
	{
		// Wait for command to complete.
		done = 0;
		do
		{
			Sleep(20); // Sleep 20ms for polling.
			status = GevGetFeatureValue( handle, (char *)command, &type, sizeof(UINT32), &done);
		} while( !done && (status == 0));
	}
	return status;
}

int setup_available_files_list( GEV_CAMERA_HANDLE handle, int *num_files, int *max_filename_length )
{
	int status = -1;
	int index = 0;
	int len = 0;
	int max_len = 0;
	GEV_STATUS s = 0;
	
	//=====================================================================
	// Get the GenICam FeatureNodeMap object to access the camera features.
	GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
				
	if (Camera)
	{
		// Get the File Access features we will need.
		// (file access definitions are part of SFNC - so if the device follows SFNC then it should be OK).
		CEnumerationPtr ptrFileSelector = Camera->_GetNode("FileSelector") ;
		CRegisterPtr ptrFileAccessBuffer = Camera->_GetNode("FileAccessBuffer") ;

		
		try {
			NodeList_t entries;
			ptrFileSelector->GetEntries(entries);
			for( NodeList_t::iterator itEntry=entries.begin(); (itEntry != entries.end()) && (index < MAX_NUM_CAMERA_FILES); itEntry++ )
			{
				CEnumEntryPtr entryPtr(*itEntry);
				if (entryPtr.IsValid() && (GenApi::NI != (*itEntry)->GetAccessMode()) )
				{
					int type;
					const char *entryName = static_cast<const char *>(entryPtr->GetSymbolic());
					strncpy( _cam_file_info[index].file_id, entryName, (MAX_CAMERA_FILENAME_LENGTH-1) );
					len = strlen( _cam_file_info[index].file_id);
					max_len = (len > max_len) ? len : max_len;
					_cam_file_info[index].file_index = static_cast<int>(entryPtr->GetValue());

					// Set up to access this file.
					s = GevSetFeatureValueAsString(handle, "FileSelector", _cam_file_info[index].file_id);

					_cam_file_info[index].available = 1; 						

					if ( GenApi::NI != (*itEntry)->GetAccessMode() )
					{
					}
					// Check if this file can be opened for read 
					s = GevSetFeatureValueAsString(handle, "FileOperationSelector", "Open");
					s = GevSetFeatureValueAsString(handle, "FileOpenMode", "Read");
					if (s == 0)
					{
						char result[64] = {0};
						
						// Success (file is read-able, assume it is present).
						_cam_file_info[index].read_access = 1; 	
						
						gev_command(handle, "FileOperationExecute");
						
						// Check the success.
						status = GevGetFeatureValueAsString(handle, "FileOperationStatus", &type, sizeof(result), result);
						if (!strncasecmp(result, "Success", sizeof(result)))
						{
							//int v;
							//GevGetFeatureValue(handle, "FileSize", &type, sizeof(v), &v); 
							//printf(" File %d is present for reading - size = %d\n", index, v);
							_cam_file_info[index].present = 1; 						
							s = GevSetFeatureValueAsString(handle, "FileOperationSelector", "Close");
							gev_command(handle, "FileOperationExecute");								
						}
					}

					// Check if this file can be opened for write 
					s = GevSetFeatureValueAsString(handle, "FileOperationSelector", "Open");
					s = GevSetFeatureValueAsString(handle, "FileOpenMode", "Write");
					if (s == 0)
					{
						// Success (file location is avaiable and writable-able).
						_cam_file_info[index].write_access = 1; 						
					}

					index++;
				}
				status = 0;
				if (num_files != NULL)
				{
					*num_files = index;
				}
			}
		}
		// Catch all possible exceptions from a node access.
		CATCH_GENAPI_ERROR(status);
	}
	if (max_filename_length != NULL)
	{
		*max_filename_length = max_len;
	}
	return status;
}

// This function reads data from an open file on a device into a buffer.
// The data is received in the "data" buffer (already allocated)
// The size of the file (without padding) is in the "dataSize" variable.
int gev_read_data_from_camera( GEV_CAMERA_HANDLE handle, void *data, int dataSize)
{
	GEV_STATUS status = -1;
	int type;
	int byteOffset, bytesRead, bytesToRead, maxBytes;
	char result[64] = {0};
	
	// Maximize the buffer size to use.
	GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
				
	if (Camera)
	{
		CIntegerPtr accessLength = Camera->_GetNode("FileAccessLength");
		maxBytes = accessLength->GetMax();
	}
	
	if ((data != NULL) && (dataSize != 0) )
	{
		unsigned char *pBuf = (unsigned char *)data;
		
		// OK to proceed.
		//=======================================================
		// Read data from the camera (in a loop)
		byteOffset = 0;
		while ( (int)byteOffset < dataSize )
		{
			// Set up the read offset in the camera
			status = GevSetFeatureValueAsString(handle, "FileOperationSelector", "Read");
			
			bytesToRead = min( maxBytes, (dataSize - byteOffset) );
			status = GevSetFeatureValue( handle, "FileAccessLength",  sizeof(UINT32), &bytesToRead);
			status = GevSetFeatureValue( handle, "FileAccessOffset", sizeof(UINT32), &byteOffset);

			
			// Tell the camera to transfer the intended read data.
			status = gev_command( handle, "FileOperationExecute");

			// Check the success.
			status = GevGetFeatureValueAsString(handle, "FileOperationStatus", &type, sizeof(result), result);
			if (strncasecmp(result, "Success", sizeof(result)))
			{
				break;
			}
		
			// Get the number of bytes read.
			status = GevGetFeatureValue(handle, "FileOperationResult", &type, sizeof(bytesRead), &bytesRead);
			if (status != 0)
			{
				break;
			}
			
			// Read the data into the buffer.
			status = GevGetFeatureValue( handle, "FileAccessBuffer", &type, bytesRead, &pBuf[byteOffset]);
			if (status != 0)
			{
				break;
			}
			
			byteOffset += bytesRead;			
		}
		
		if ( byteOffset < dataSize )
		{
			return -1;
		}
	}
	return status;	
}

// This function writes data to an open file on a device from a buffer.
// The data is in the "data" buffer.
// The size of the file (without padding) is in the "dataSize" variable.
int gev_write_data_to_camera( GEV_CAMERA_HANDLE handle, void *data, int dataSize)
{
	GEV_STATUS status = -1;
	int byteOffset, bytesWritten, bytesToWrite, maxBytes;
	char result[64] = {0};
	int type = 0;
	
	// Maximize the buffer size to use.
	GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
				
	if (Camera)
	{
		CIntegerPtr accessLength = Camera->_GetNode("FileAccessLength");
		maxBytes = accessLength->GetMax();
	}
	
	if ((data != NULL) && (dataSize != 0) )
	{
		unsigned char *pBuf = (unsigned char *)data;
		
		// OK to proceed.
		//=======================================================
		// Write data to the camera (in a loop)
		byteOffset = 0;
		while ( byteOffset < dataSize )
		{
			// Set up the write offset in the camera
			status = GevSetFeatureValueAsString(handle, "FileOperationSelector", "Write");
			
			bytesToWrite = min( maxBytes, (dataSize - byteOffset) );
			status = GevSetFeatureValue( handle, "FileAccessLength",  sizeof(UINT32), &bytesToWrite);
			status = GevSetFeatureValue( handle, "FileAccessOffset", sizeof(UINT32), &byteOffset);

			// Write the data into the camera.
			status = GevSetFeatureValue( handle, "FileAccessBuffer", bytesToWrite, &pBuf[byteOffset]);
			if (status != 0)
			{
				break;
			}

			// Tell the camera to transfer the intended read data.
			status = gev_command( handle, "FileOperationExecute");

			// Check the success.
			status = GevGetFeatureValueAsString(handle, "FileOperationStatus", &type, sizeof(result), result);
			if (strncasecmp(result, "Success", sizeof(result)))
			{
				break;
			}
		
			// Get the number of bytes written.
			status = GevGetFeatureValue(handle, "FileOperationResult", &type, sizeof(bytesWritten), &bytesWritten);
			if (status != 0)
			{
				break;
			}
			
			byteOffset += bytesWritten;			
		}
		
		if ( byteOffset < dataSize )
		{
			return -1;
		}
	}
	return status;	
}


// This function writes a file on disk to the device.
// The file is identified by the file index.
// The file name is passed in.
//
// Note : The file is loaded into a memory array that is padded to be the
// next largest multiple of 4 bytes. Only the valid data btyes will be
// stored in the file on the device.
int gev_write_file_to_camera( GEV_CAMERA_HANDLE handle, char *fileName, int fileIndex)
{
	GEV_STATUS status = -1;
	FILE *fp = NULL;
	void *file_data = NULL;
	int file_size = 0;

	// Open the file and get is size;
	fp = fopen( fileName, "r");
	if (fp != NULL)
	{
		int pad = 0;
		
		fseek(fp, 0L, SEEK_END);
		file_size = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		
		// Allocate memory for the file contents.
		if ((file_size % 4) != 0)
		{
			pad = 4 - (file_size % 4);  // Multiple of 4 bytes required.
		}
		file_data = malloc(file_size + pad);
		memset(file_data, 0, (file_size + pad));
		if (file_data != NULL)
		{
			// Load the file into memory.
			int len = fread( file_data, 1, file_size, fp);

			// Close the file.
			fclose(fp);
			
			// Check if all the data came in.
			if (len != file_size)
			{
				// Error - not all the data was read into memory.
				printf("len = %d, file_size = %d\n", len, file_size); //????
				return status;
			}

			//========================================================
			// Open the camera file for write access;
			status = GevSetFeatureValue(handle, "FileSelector", sizeof(fileIndex), &fileIndex);
			status = GevSetFeatureValueAsString(handle, "FileOperationSelector", "Open");
			status = GevSetFeatureValueAsString(handle, "FileOpenMode", "Write");
			status = GevSetFeatureValue(handle, "FileSelector", sizeof(fileIndex), &fileIndex);

			status = gev_command( handle, "FileOperationExecute");
			if (status == 0)
			{
				// Write the file to the camera.
				status = gev_write_data_to_camera( handle, file_data, file_size);

				//========================================================
				// Close the camera file to complete the write access;			
				GevSetFeatureValueAsString(handle, "FileOperationSelector", "Close");
				gev_command( handle, "FileOperationExecute");
			}
		}
	}
	return status;	
}


// This function reads a file on the device to a file on disk.
// The file is identified by the file index.
// The file name is passed in.
//
// Note : The file size is obtained from the camera and a memory array 
// that is padded to be the next largest multiple of 4 bytes is used to 
// receive the data. Only the valid data bytes are stored to disk.
int gev_read_file_from_camera( GEV_CAMERA_HANDLE handle, char *fileName, int fileIndex)
{
	GEV_STATUS status = -1;
	FILE *fp = NULL;
	void *file_data = NULL;
	int file_size = 0;
	int type = 0;
	
	//========================================================
	// Open the camera file for read access;
	status = GevSetFeatureValue(handle, "FileSelector", sizeof(fileIndex), &fileIndex);
	status = GevSetFeatureValueAsString(handle, "FileOperationSelector", "Open");
	status = GevSetFeatureValueAsString(handle, "FileOpenMode", "Read");

	status = gev_command( handle, "FileOperationExecute");
	if (status == 0)
	{
		printf("Camera file opened\n");

		// Get the file size from the camera.
		status = GevGetFeatureValue(handle, "FileSize", &type, sizeof(file_size), &file_size);
		
		if ( (status == 0) && (file_size > 0) )
		{
			// Open the disk file (early - in case we don't have permission).
			status = -1;
			fp = fopen( fileName, "w+");
			if (fp != NULL)
			{
				// Allocate memory for the file data contents.
				int pad = 0;
				
				if ((file_size % 4) != 0)
				{
					pad = 4 - (file_size % 4);  // Multiple of 4 bytes required.
				}
				file_data = malloc(file_size + pad);
				if (file_data != NULL)
				{
					// Read the file from the camera into memory.							
					status = gev_read_data_from_camera( handle, file_data, file_size);			
					if (status == 0)
					{
						// Write the data to the file.
						fwrite(file_data, 1, file_size, fp);
					}
					else
					{
						printf("read_data returns error\n"); 
						fwrite(file_data, 1, file_size, fp);
					}
				}
				// Close the file,
				fclose(fp);
			}
		}
		else
		{
			if (file_size == 0)
			{
				status = -1;
			}
		}
		
		//========================================================
		// Close the camera file to complete the read access;			
		GevSetFeatureValueAsString(handle, "FileOperationSelector", "Close");
		gev_command( handle, "FileOperationExecute");

	}
	return status;
}

int get_a_filename( char *file_name, int *file_index)
{
	int i = 0;
	
	if ((file_name != NULL) && (file_index != NULL))
	{
		char response[512] = {0};
		int done = FALSE;
		while(!done)
		{
			printf("\n\t Enter File Id (#) to access (non-digit to quit): ");
			response[0] = '\0';
			scanf("%s", response);

			if ( isdigit(response[0]) )
			{ 
				*file_index = atoi(response);
	
				// Find the entry
				i = 0;
				while( _cam_file_info[i].file_id[0] != '\0' )
				{
					if ( (int)_cam_file_info[i].file_index == *file_index )
					{
						// Found the file - get an associated filename.
						printf("Enter File Name : ");
						scanf("%s", file_name);
						return 0;
					}
					else
					{
						i++;
					}	
				} 
				printf("File entry at Index %d not available\n", *file_index);
			}
			else
			{
				done = TRUE;
			}
		}
	}
	return -1;
}

char GetKey()
{
   char key = getchar();
   while ((key == '\r') || (key == '\n'))
   {
      key = getchar();
   }
   return key;
}

void PrintMenu()
{
   printf("File Control : [L]=List Files on Camera\n");
   printf("File Control : [R]=Read File from Camera, [W]=Write File to Camera\n");
   printf("MISC         : [Q]or[ESC]=end\n");
}

int main(int argc, char* argv[])
{
	GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = {0};
	UINT16 status;
	int numCamera = 0;
	int camIndex = 0;
	char c;
	int done = FALSE;
	char filename[MAX_PATH] = {0};
	int  file_index = 0;

	// Greetings
	printf ("\nGigE Vision Library GenICam FileAccess Example Program (%s)\n", __DATE__);
	printf ("Copyright (c) 2016, Teledyne DALSA.\nAll rights reserved.\n\n");

	//===================================================================================
	// Set default options for the library.
	{
		GEVLIB_CONFIG_OPTIONS options = {0};

		GevGetLibraryConfigOptions( &options);
		//options.logLevel = GEV_LOG_LEVEL_OFF;
		//options.logLevel = GEV_LOG_LEVEL_TRACE;
		options.logLevel = GEV_LOG_LEVEL_NORMAL;
		GevSetLibraryConfigOptions( &options);
	}

	//====================================================================================
	// DISCOVER Cameras
	//
	// Get all the IP addresses of attached network cards.
	status = GevGetCameraList( pCamera, MAX_CAMERAS, &numCamera);

	printf ("%d camera(s) on the network\n", numCamera);

	// Select the first camera found (unless the command line has a parameter = the camera index)
	if (numCamera != 0)
	{
		if (argc > 1)
		{
			sscanf(argv[1], "%d", &camIndex);
			if (camIndex >= (int)numCamera)
			{
				printf("Camera index out of range - only %d camera(s) are present\n", numCamera);
				camIndex = -1;
			}
		}

		if (camIndex != -1)
		{
			GEV_CAMERA_HANDLE handle = NULL;
			
			//====================================================================
			// Open the camera.
			status = GevOpenCamera( &pCamera[camIndex], GevExclusiveMode, &handle);
			if (status == 0)
			{
				//=================================================================
				// Initialize access to GenICam features via Camera XML File 

				status = GevInitGenICamXMLFeatures( handle, TRUE);				
				if (status != GEVLIB_OK)
				{
					// Error - need access to XML features.
					printf("Error %d initializing access to XML based features\n", status);
					return -1;
				}
			}
			
			//=====================================================================
			// Get the GenICam FeatureNodeMap object and access the camera features.
			GenApi::CNodeMapRef *Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
				
			if (Camera)
			{
				int num_files = 0;
				int max_filename_length = 0;
				int i = 0;
				if ( setup_available_files_list( handle, &num_files, &max_filename_length) == 0)
				{
					//=================================================================
					// Call the main command loop or the example.
					PrintMenu();
					while(!done)
					{
						c = GetKey();

						if (c == '?')
						{
							PrintMenu();
						}

						if ((c == 'l') || (c == 'L'))
						{
							// List the files on the camera.
							printf("Available files [Index : Name : Access]\n");
							printf("---------------------------------------\n");
							for (i = 0; i < num_files; i++)
							{
								printf("%d\t : %-*s : ", (int)_cam_file_info[i].file_index, max_filename_length, _cam_file_info[i].file_id);
								if (_cam_file_info[i].available)
								{
									if ( _cam_file_info[i].read_access && _cam_file_info[i].write_access )
									{
										if ( _cam_file_info[i].present ) 
										{
											printf("R/W Access - file present\n");
										}
										else
										{
											printf("R/W Access - no file present\n");
										}
									}
									else if (_cam_file_info[i].read_access)
									{
										if ( _cam_file_info[i].present ) 
										{
											printf("Read Access - file present\n");
										}
										else
										{
											printf("Read Access - no file present\n");
										}
									}
									else
									{
										printf("Write Access\n");
									}
								}
								else
								{
									printf("\n");
								}
							}						
							printf("---------------------------------------\n");
							
						}
						if ((c == 'r') || (c == 'R'))
						{
							int available = 0;
							// Show available files
							for (i = 0; i < num_files; i++)
							{
								if ( _cam_file_info[i].read_access && _cam_file_info[i].present)
								{
									available++;
									printf("File index %4d aka %s is available for reading\n", (int)_cam_file_info[i].file_index, _cam_file_info[i].file_id);
								}
							}
							if ( !available )
							{
								printf("No files available for reading\n");
							}
							else
							{							
								// Get index of file to read and name of host file to write to.
								if (get_a_filename( filename, &file_index) == 0 )
								{
									// Read the file from the camera.
									if ( gev_read_file_from_camera( handle, filename, file_index) == 0 )
									{
										printf("Success\n");
									}
									else
									{
										printf("Failed\n");
									}
								}
								else
								{
									printf("Aborted\n");
								}
							}
						}
							
						if ((c == 'w') || (c == 'W'))
						{
							int available = 0;
							// Show available files
							for (i = 0; i < num_files; i++)
							{
								if ( _cam_file_info[i].write_access && _cam_file_info[i].available)
								{
									available++;
									printf("File index %4d aka %s is available for writing\n", (int)_cam_file_info[i].file_index, _cam_file_info[i].file_id);
								}
							}
							if ( !available )
							{
								printf("No files available for writing\n");
							}
							else
							{
								// Get index of file to write and name of host file to read from.
								if (get_a_filename( filename, &file_index) == 0 )
								{
									// Write the file to the camera.
									if ( gev_write_file_to_camera( handle, filename, file_index) == 0 )
									{
										printf("Success\n");
									}
									else
									{
										printf("Failed\n");
									}
								}
								else
								{
									printf("Aborted\n");
								}
							}
						}
							
						if ((c == 0x1b) || (c == 'q') || (c == 'Q'))
						{
							done = TRUE;
						}
					}
				}
				
			}

			// Close the camera.
			GevCloseCamera(&handle);
		}
		else
		{
			printf("Error : 0x%0x : opening camera\n", status);
		}
	}

	// Close down the API.
	GevApiUninitialize();

	return 0;
}

