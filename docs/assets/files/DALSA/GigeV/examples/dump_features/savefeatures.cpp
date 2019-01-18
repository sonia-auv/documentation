//
// savefeatures : Save streamable features from a camera to a file.
//
//	They are saved as "FeatureName" "FeatureValue_as_String" pairs
//	separated by "newlines".
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
	printf("Usage: savefeatures                    : Output features from camera 0 to stdout.\n");
	printf("       savefeatures - cam_index        : Output features from camera 'cam_index' to stdout. (Note the hyphen indicating stdout)\n");
	printf("       savefeatures filename           : Save features from camera 0 to 'filename'.\n");
	printf("       savefeatures filename cam_index : Save features from camera 'cam_index' to 'filename'.\n");
}


static void OutputFeatureValuePair( const char *feature_name, const char *value_string, FILE *fp )
{
	if ( (feature_name != NULL)  && (value_string != NULL) )
	{
		// Feature : Value pair output (in one place in to ease changing formats or output method - if desired).
		fprintf(fp, "%s %s\n", feature_name, value_string);
	}
}


static void OutputFeatureValues( const CNodePtr &ptrFeature, FILE *fp )
{
	
   CCategoryPtr ptrCategory(ptrFeature);
   if( ptrCategory.IsValid() )
   {
       GenApi::FeatureList_t Features;
       ptrCategory->GetFeatures(Features);
       for( GenApi::FeatureList_t::iterator itFeature=Features.begin(); itFeature!=Features.end(); itFeature++ )
       {	
          OutputFeatureValues( (*itFeature), fp );
       }
   }
   else
   {
		// Store only "streamable" features (since only they can be restored).
		if ( ptrFeature->IsStreamable() )
		{
			// Create a selector set (in case this feature is selected)
			bool selectorSettingWasOutput = false;
			CSelectorSet selectorSet(ptrFeature);
			
			// Loop through all the selectors that select this feature.
			// Use the magical CSelectorSet class that handles the 
			//   "set of selectors that select this feature" and indexes
			// through all possible combinations so we can save all of them.
			selectorSet.SetFirst();
			do
			{
				CValuePtr valNode(ptrFeature);  
				if ( valNode.IsValid() && (GenApi::RW == valNode->GetAccessMode()) && (ptrFeature->IsFeature()) )
				{
					// Its a valid streamable feature.
					// Get its selectors (if it has any)
					FeatureList_t selectorList;
					selectorSet.GetSelectorList( selectorList, true );

					for ( FeatureList_t ::iterator itSelector=selectorList.begin(); itSelector!=selectorList.end(); itSelector++ )	
					{
						// Output selector : selectorValue as a feature : value pair.
						selectorSettingWasOutput = true;
						CNodePtr selectedNode( *itSelector);
						CValuePtr selectedValue( *itSelector);
						OutputFeatureValuePair(static_cast<const char *>(selectedNode->GetName()), static_cast<const char *>(selectedValue->ToString()), fp);
					}
						
					// Output feature : value pair for this selector combination 
					// It just outputs the feature : value pair if there are no selectors. 
					OutputFeatureValuePair(static_cast<const char *>(ptrFeature->GetName()), static_cast<const char *>(valNode->ToString()), fp);					
				}
				
			} while( selectorSet.SetNext());
			// Reset to original selector/selected value (if any was used)
			selectorSet.Restore();
			
			// Save the original settings for any selector that was handled (looped over) above.
			if (selectorSettingWasOutput)
			{
				FeatureList_t selectingFeatures;
				selectorSet.GetSelectorList( selectingFeatures, true);
				for ( FeatureList_t ::iterator itSelector = selectingFeatures.begin(); itSelector != selectingFeatures.end(); ++itSelector)
				{
					CNodePtr selectedNode( *itSelector);
					CValuePtr selectedValue( *itSelector);
					OutputFeatureValuePair(static_cast<const char *>(selectedNode->GetName()), static_cast<const char *>(selectedValue->ToString()), fp);
				} 
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
	char filename[MAX_PATH] = {0};
	FILE *fp = NULL;
	
	// Greetings
	printf ("\nGigE Vision Library GenICam Feature Save Example (%s)\n", __DATE__);

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
		if (argc > 3)
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
		
		// Set up the output stream.
		if (argc == 1)
		{
			// Output to stdout.
			fp = stdout; 
		} 
		else if ( argc > 1 )
		{
			// Extract the filename.
			strncpy(filename, argv[1], sizeof(filename));
			
			// See if it is '-' for "stdout".
			if ( (1 == strlen(filename)) && (filename[0] == '-'))
			{
				fp = stdout;
			}
			else
			{
				// Open the file.
				fp = fopen(filename, "w");
				if (fp == NULL)
				{
					printf("Error opening file %s : errno = %d\n", filename, errno);
					exit(-1);
				}
			}			
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
				// Create CNodeMap object from Camera XML File data (many possible ways to do it)....
				#if 0
				{
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
								
				// Traverse the node map and dump all the { feature value } pairs.
				if ( status == 0 )
				{
					// Find the standard "Root" node and dump the features.
					GenApi::CNodePtr pRoot = Camera._GetNode("Root");
					OutputFeatureValues( pRoot, fp);
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
				
				// Done - close the camera.
				GevCloseCamera(&handle);
				fclose(fp);
			}
			else
			{
				printf("Error : 0x%0x : opening camera\n", status);
			}
		}
	}

	// Close down the API.
	GevApiUninitialize();

	// Close socket API
	_CloseSocketAPI ();	// must close API even on error


	//printf("Hit any key to exit\n");
	//kbhit();

	return 0;
}

