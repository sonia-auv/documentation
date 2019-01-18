//
// featuretree: Output the tree of all features accessible from the "Root" node.
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

const char *intfTypeStrings[] = 
{
	"IValue", "IBase", "IInteger", "IBoolean", "ICommand", "IFloat", "IString",
	"IRegister", "ICategory", "IEnumeration", "IEnumEntry", "IPort"
};

static void DumpFeatureHierarchy( const CNodePtr &ptrFeature, int Indent )
{
   int i = 0;
   for (i = 0; i < Indent; i++)
   {
      printf("\t");
	}
    
   CCategoryPtr ptrCategory(ptrFeature);
   if( ptrCategory.IsValid() )
   {
       //cout << IndentStr << "Category '" << ptrFeature->GetName() << "'\n";
       printf("Category : %s\n", static_cast<const char *>(ptrFeature->GetName()));
       GenApi::FeatureList_t Features;
       ptrCategory->GetFeatures(Features);
       for( GenApi::FeatureList_t::iterator itFeature=Features.begin(); itFeature!=Features.end(); itFeature++ )
       {
            DumpFeatureHierarchy( (*itFeature), Indent + 2 );
       }
   }
   else
   {
		// Get the type of the feature as information.
		int index = static_cast<int>(ptrFeature->GetPrincipalInterfaceType());
		if ( (index >= (int)intfIValue) && (index <= (int)intfIPort))
		{		
			printf("%s : <%s>\n", static_cast<const char *>(ptrFeature->GetName()), intfTypeStrings[index]);
		}
		else
		{
			printf("%s : <Unknown Type>\n", static_cast<const char *>(ptrFeature->GetName()));
		}
	}
}


int main(int argc, char* argv[])
{
	GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = {0};
	UINT16 status;
	int numCamera = 0;
	int camIndex = 0;
	
	// Greetings
	printf ("\nGigE Vision Library GenICam Feature Tree Dumper (%s)\n", __DATE__);

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
		// Extract the camera index (if present)
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

				// Traverse the node map and dump all the features.
				if ( status == 0 )
				{
					// Find the standard "Root" node and dump the features.
					GenApi::CNodePtr pRoot = Camera._GetNode("Root");
					printf("Dumping feature tree : \n");
					DumpFeatureHierarchy( pRoot, 1);
				}
				GevCloseCamera(&handle);
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

