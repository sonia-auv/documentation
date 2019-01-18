//
// feature_loader : Load (restore) streamable features from a file to a camera.
//		(Using the C API only).
//

#include "stdio.h"
#include "cordef.h"
#include "gevapi.h"				//!< GEV lib definitions.


#define MAX_NETIF					8
#define MAX_CAMERAS_PER_NETIF	32
#define MAX_CAMERAS		(MAX_NETIF * MAX_CAMERAS_PER_NETIF)

void Usage()
{
	printf("Usage: c_loadfeatures filename           : Load features from 'filename' to camera 0.\n");
	printf("       c_loadfeatures filename cam_index : Load features from 'filename' to camera 'cam_index'.\n");
}

int main(int argc, char* argv[])
{
	GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = {0};
	UINT16 status;
	int numCamera = 0;
	int camIndex = 0;
	int error_count = 0;
	int feature_count = 0;
	char filename[MAX_PATH] = {0};
	FILE *fp = NULL;
	
	// Greetings
	printf ("\nGigE Vision Library GenICam Feature Load Example (%s)\n", __DATE__);

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

	//====================================================================================
	// Interact with the command line.
	// Select the first camera found (unless the command line has a parameter = the camera index)
	if (numCamera != 0)
	{
		if ((argc == 1) || (argc > 3))
		{
			Usage();
			exit(-1);
		}
		
		// Check for help
		if ((argc == 2) && ( (argv[1][0] == '?') || (argv[1][0] == '-') ) )
		{
			Usage();
			exit(-1);
		}

		// Extract the filename.
		strncpy(filename, argv[1], sizeof(filename));
			
		// Open the file.
		fp = fopen(filename, "r");
		if (fp == NULL)
		{
			printf("Error opening file %s : errno = %d\n", filename, errno);
			exit(-1);
		}	
		
		// Extract the (optional) camera index
		if (argc > 2)
		{
			int len = sscanf(argv[2], "%d", &camIndex);
			if (len == 0)
			{
				printf("Invalid camera index entered - defaulting to 0\n");
				camIndex = 0;
			}
			if (camIndex >= (int)numCamera)
			{
				printf("Camera index out of range - only %d camera(s) are present - defaulting to 0\n", numCamera);
				camIndex = 0;
			}
		}


		if (camIndex != -1)
		{
			//====================================================================
			// Connect to Camera
			//
			//
			GEV_CAMERA_HANDLE handle = NULL;

			// Open the camera.
			status = GevOpenCamera( &pCamera[camIndex], GevExclusiveMode, &handle);
			if (status == 0)
			{
				//=================================================================
				// Initiliaze access to GenICam features via Camera XML File 

				status = GevInitGenICamXMLFeatures( handle, TRUE);				
				if (status == GEVLIB_OK)
				{
					// Get the name of XML file name back (example only - in case you need it somewhere).
					char xmlFileName[MAX_PATH] = {0};
					status = GevGetGenICamXML_FileName( handle, (int)sizeof(xmlFileName), xmlFileName);
					if (status == GEVLIB_OK)
					{
						printf("XML stored as %s\n", xmlFileName);
					}
				}
				
				if (status == GEVLIB_OK)
				{
					// Put the camera in "streaming feature mode".
					status = GevSetFeatureValueAsString( handle, "Std::DeviceFeaturePersistenceStart", " ");
					if ( status == 0 )
					{					
						// Verify that we are in "streaming feature mode"/
						{
							int done = FALSE;
							int timeout = 5;
							int type = 0;

							while(!done && (timeout-- > 0))
							{
								Sleep(10);
								status = GevGetFeatureValue( handle, "Std::DeviceFeaturePersistenceStart", &type, sizeof(done), &done);
								if (status != 0)
								{
									done = TRUE;
								}
							}
						}
									
						// Read the file as { feature : value } pairs and write them to the camera.
						if ( status == 0 )
						{
							char feature_name[MAX_GEVSTRING_LENGTH+1] = {0};
							char value_str[MAX_GEVSTRING_LENGTH+1] = {0};
							
							while ( 2 == fscanf(fp, "%s %s", feature_name, value_str) )
							{
								status = GevSetFeatureValueAsString( handle, feature_name, value_str );
								if (status != 0)
								{
									error_count++;
									printf("Error restoring feature %s : with value %s\n", feature_name, value_str); 
								}
								else
								{
									feature_count++;
								}
							}
						}
						else
						{
							printf("Error - unable to initialize feature streaming! (DeviceFeaturePersistenceStart command failed)\n"); 
						}

						// End the "streaming feature mode".
						status = GevSetFeatureValueAsString( handle, "Std::DeviceFeaturePersistenceEnd", " ");
						if ( status == 0 )
						{					
							// Verify that we are in "streaming feature mode"/
							{
								int done = FALSE;
								int timeout = 5;
								int type = 0;

								while(!done && (timeout-- > 0))
								{
									Sleep(10);
									status = GevGetFeatureValue( handle, "Std::DeviceFeaturePersistenceEnd", &type, sizeof(done), &done);
									if (status != 0)
									{
										done = TRUE;
									}
								}
							}
						}
					}
					
					// Note : The features should be validated here by reading the feature tree.
					// 	The C functions in the API do not yet have this capability.
					//     (See the C++ version of this example).
				
					if (error_count == 0)
					{
						printf("%d Features loaded successfully!\n", feature_count);
					}
					else
					{
						printf("%d Features loaded successfully : %d Features had errors\n", feature_count, error_count);
					}
				}
				
				// Done - close the camera.
				GevCloseCamera(&handle);
			}
			else
			{
				printf("Error : 0x%0x : opening camera\n", status);
			}
		}
	}

	// Close the file.
	fclose(fp);

	// Close down the API.
	GevApiUninitialize();

	// Close socket API
	_CloseSocketAPI ();	// must close API even on error

	return 0;
}

