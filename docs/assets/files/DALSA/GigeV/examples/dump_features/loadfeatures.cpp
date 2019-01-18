//
// loadfeatures : Load (restore) streamable features from a file to a camera.
//

#include "stdio.h"
#include "cordef.h"
#include "GenApi/GenApi.h"		//!< GenApi lib definitions.
#include "gevapi.h"				//!< GEV lib definitions.
//#include "SapX11Util.h"
//#include "X_Display_utils.h"
//#include <sched.h>

using namespace std;
using namespace GenICam;
using namespace GenApi;

#define MAX_NETIF					8
#define MAX_CAMERAS_PER_NETIF	32
#define MAX_CAMERAS		(MAX_NETIF * MAX_CAMERAS_PER_NETIF)

void Usage()
{
	printf("Usage: loadfeatures filename           : Load features from 'filename' to camera 0.\n");
	printf("       loadfeatures filename cam_index : Load features from 'filename' to camera 'cam_index'.\n");
}


static void ValidateFeatureValues( const CNodePtr &ptrFeature )
{
	
   CCategoryPtr ptrCategory(ptrFeature);
   if( ptrCategory.IsValid() )
   {
       GenApi::FeatureList_t Features;
       ptrCategory->GetFeatures(Features);
       for( GenApi::FeatureList_t::iterator itFeature=Features.begin(); itFeature!=Features.end(); itFeature++ )
       {	
          ValidateFeatureValues( (*itFeature) );
       }
   }
   else
   {
		// Issue a "Get" on the feature (with validate set to true).
		CValuePtr valNode(ptrFeature);  
		if ((GenApi::RW == valNode->GetAccessMode()) || (GenApi::RO == valNode->GetAccessMode()) )
		{
			int status = 0;
			try {
				valNode->ToString(true);
			}
			// Catch all possible exceptions from a node access.
			CATCH_GENAPI_ERROR(status);
			if (status != 0)
			{
				printf("load_features : Validation failed for feature %s\n", static_cast<const char *>(ptrFeature->GetName())); 
			}
		}
	}
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
			GenApi::CNodeMapRef Camera;	// The GenICam XML-based feature node map.
			GEV_CAMERA_HANDLE handle = NULL;

			// Open the camera.
			status = GevOpenCamera( &pCamera[camIndex], GevExclusiveMode, &handle);
			if (status == 0)
			{
				//=================================================================
				// Manually create CNodeMap object from Camera XML File data (many possible ways to do it)....
				#if 0
				{
					//===================================================================
					// Get the XML data itself (into an array) and use it to make the CNodeMap object.
					int xmlDataSize = 0;
					int zipData = 0;
               char *pXmlData;
               
               // Query the XML size.
					Gev_RetrieveXMLData( handle, 0, NULL, &xmlDataSize, NULL);
					pXmlData = (char *)malloc( ((xmlDataSize + 3) & (~3)) + 1);
					
					// Get the XML data.
					Gev_RetrieveXMLData( handle, (xmlDataSize + 3) & (~3), pXmlData, &xmlDataSize, &zipData);
					pXmlData[xmlDataSize ] = 0;
					
					// Use it to make the CNodeMap object
					if ( !zipData )
					{
						GenICam::gcstring xmlStr( pXmlData );
						Camera._LoadXMLFromString(xmlStr);
					}
					else
					{
						Camera._LoadXMLFromZIPData((void *)pXmlData, xmlDataSize);
					}
					free(pXmlData);

				}
				#else
				{
					//===================================================================
					// Get the XML file onto disk and use it to make the CNodeMap object.
					char xmlFileName[MAX_PATH] = {0};
					
					status = Gev_RetrieveXMLFile( handle, xmlFileName, sizeof(xmlFileName), FALSE );
					if ( status == GEVLIB_OK)
					{
						printf("XML stored as %s\n", xmlFileName);
						Camera._LoadXMLFromFile( xmlFileName );
					}
				}
				#endif
				
				// Connect the features in the node map to the camera handle.
				status = GevConnectFeatures( handle, static_cast<void *>(&Camera));
				if ( status != 0 )
				{
					printf("Error %d connecting node map to handle\n", status);
				}

				// Put the camera in "streaming feature mode".
				CCommandPtr start = Camera._GetNode("Std::DeviceFeaturePersistenceStart");
				if ( start )
				{
					try {
							int done = FALSE;
							int timeout = 5;
							start->Execute();
							while(!done && (timeout-- > 0))
							{
								Sleep(10);
								done = start->IsDone();
							}
					}
					// Catch all possible exceptions from a node access.
					CATCH_GENAPI_ERROR(status);
				}
								
				// Read the file as { feature value } pairs and write them to the camera.
				if ( status == 0 )
				{
					char feature_name[MAX_GEVSTRING_LENGTH+1] = {0};
					char value_str[MAX_GEVSTRING_LENGTH+1] = {0};
					
					while ( 2 == fscanf(fp, "%s %s", feature_name, value_str) )
					{
						status = 0;
						// Find node and write the feature string (without validation).
						GenApi::CNodePtr pNode = Camera._GetNode(feature_name);
						if (pNode)
						{
							CValuePtr valNode(pNode);  
							try {
								valNode->FromString(value_str, false);
							}
							// Catch all possible exceptions from a node access.
							CATCH_GENAPI_ERROR(status);
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
						else
						{
							error_count++;
							printf("Error restoring feature %s : with value %s\n", feature_name, value_str);
						}
					}
				}

				// End the "streaming feature mode".
				CCommandPtr end = Camera._GetNode("Std::DeviceFeaturePersistenceEnd");
				if ( end  )
				{
					try {
							int done = FALSE;
							int timeout = 5;
							end->Execute();
							while(!done && (timeout-- > 0))
							{
								Sleep(10);
								done = end->IsDone();
							}
					}
					// Catch all possible exceptions from a node access.
					CATCH_GENAPI_ERROR(status);
				}
				
				// Validate.
				if (status == 0)
				{
					// Iterate through all of the features calling "Get" with validation enabled.
					// Find the standard "Root" node and dump the features.
					GenApi::CNodePtr pRoot = Camera._GetNode("Root");
					ValidateFeatureValues( pRoot );
				}
				

				if (error_count == 0)
				{
					printf("%d Features loaded successfully !\n", feature_count);
				}
				else
				{
					printf("%d Features loaded successfully : %d Features had errors\n", feature_count, error_count);
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

