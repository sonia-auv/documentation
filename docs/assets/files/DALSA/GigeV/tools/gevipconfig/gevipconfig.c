/* ===========================================================

GigE-V Framework for Linux : Camera IP Configuration tool.

===========================================================*/
#include "stdio.h"
#include "cordef.h"
#include "gevapi.h"				//!< GEV lib definitions.


// IP Config modes.
#define IPCONFIG_MODE_LLA			0x04
#define IPCONFIG_MODE_DHCP			0x02
#define IPCONFIG_MODE_PERSISTENT	0x01

GEV_STATUS ForceCameraIPAndMode( BOOL persistentIPMode, UINT32 macHi, UINT32 macLo, UINT32 IPAddress, UINT32 subnet_mask);
GEV_STATUS GetCameraIPSettings( GEV_CAMERA_HANDLE handle, PUINT32 pIPAddress, PUINT32 pSubnet_mask, BOOL *pPersistentIpMode);

static void usage()
{
		printf("Usage: gevipconfig [-p] MAC_Address IP_Address Subnet_Mask\n");
		printf("                    -p (optional) = sets address/subnet to persistent mode\n");
		printf("                    MAC_Address   = aa:bb:cc:dd:ee:ff (a-f are HEX digits)\n");
		printf("                    IP_Address    = A.B.C.D  (A-D are decimal digits)\n");  
		printf("                    Subnet_Mask   = A.B.x.y  (Mask for class B or C subnet)\n");
}


int main(int argc, char* argv[])
{
	GEV_STATUS status = -1;
	int parse_index = 0;
	int persistentIPMode = FALSE;
	unsigned int macLo = 0;
	unsigned int macHi = 0;
	unsigned int ipAddr = 0;
	unsigned int subnet_mask = 0;
	int len = 0;
	int i1, i2, i3, i4, i5, i6;

	if ((argc == 1) || (argc > 5))
	{
		usage();
		exit(-1);
	}

	// Get the optional parameter.
	if ( 0 == strncmp(argv[1], "-p", 2) )
	{
		persistentIPMode = TRUE;
		parse_index = 1;
	}
	if ( (argc - parse_index) != 4)
	{
		usage();
		exit(-1);
	}

	// Parse the command line.
	{
		len = sscanf(argv[1+parse_index], "%x:%x:%x:%x:%x:%x", &i1, &i2, &i3, &i4, &i5, &i6);
		if (len != 6)
		{
			printf("\t*** Malformed MAC address string *** \n");
			exit(-1);
		}
		macLo = ((i3 & 0xFF) << 24) | ((i4 & 0xFF) << 16) | ((i5 & 0xFF) << 8) | (i6 & 0xFF);
		macHi = ((i1 & 0xFF) << 8) | (i2 & 0xFF);
	}

	{
		len = sscanf(argv[2+parse_index], "%d.%d.%d.%d", &i1, &i2, &i3, &i4);
		if (len != 4)
		{
			printf("\t*** Malformed IP address string *** \n");
			exit(-1);
		}
		ipAddr = ((i1 & 0xFF) << 24) | ((i2 & 0xFF) << 16) | ((i3 & 0xFF) << 8) | (i4 & 0xFF);
	}
	
	{
		len = sscanf(argv[3+parse_index], "%d.%d.%d.%d", &i1, &i2, &i3, &i4);
		if (len != 4)
		{
			printf("\t*** Malformed IP subnet string *** \n");
			exit(-1);
		}
		subnet_mask = ((i1 & 0xFF) << 24) | ((i2 & 0xFF) << 16) | ((i3 & 0xFF) << 8) | (i4 & 0xFF);
	}
	//  Outputs for debugging - 
	//			printf(" ForceIP : Camera at %02x:%02x:%02x:%02x:%02x:%02x set to ", ((macHi&0x0000ff00)>>8), (macHi&0x000000ff), ((macLo&0xff000000)>>24),((macLo&0x00ff0000)>>16),((macLo&0x0000ff00)>>8), (macLo&0x000000ff));
	//			if ( persistentIPMode ) printf("Persistent ");
	//			printf("IP address %d.%d.%d.%d ", ((ipAddr & 0xff000000)>>24),((ipAddr & 0x00ff0000)>>16),((ipAddr & 0x0000ff00)>>8),(ipAddr & 0x000000ff));
	//			printf("(Mask= %d.%d.%d.%d)\n", ((subnet_mask & 0xff000000)>>24),((subnet_mask & 0x00ff0000)>>16),((subnet_mask & 0x0000ff00)>>8),(subnet_mask & 0x000000ff));

	// Initialize the API.
	GevApiInitialize();
	
	// Change the command acknowledge timeout if needed.
	{
		GEVLIB_CONFIG_OPTIONS options = {0};

		GevGetLibraryConfigOptions( &options);
		if (options.command_timeout_ms < 500)
		{
			options.command_timeout_ms = 500;
			GevSetLibraryConfigOptions( &options);
		}
	}
	
	// Program the IP address (and mode) 
	status = ForceCameraIPAndMode( persistentIPMode, macHi, macLo, ipAddr, subnet_mask);
	if (status == 0)
	{
		GEV_CAMERA_HANDLE handle;
		GevDeviceCount();
	
		// Open the camera by address (readonly mode)
		status = GevOpenCameraByAddress( ipAddr, GevMonitorMode, &handle);
		if (status == 0)
		{
			// Read the settings back and report them
			status = GetCameraIPSettings( handle, &ipAddr, &subnet_mask, &persistentIPMode);
			if (status == 0)
			{
				printf(" ForceIP : Camera at %02x:%02x:%02x:%02x:%02x:%02x set to ", ((macHi&0x0000ff00)>>8), (macHi&0x000000ff), ((macLo&0xff000000)>>24),((macLo&0x00ff0000)>>16),((macLo&0x0000ff00)>>8), (macLo&0x000000ff));
				if ( persistentIPMode ) printf("Persistent ");
				printf("IP address %d.%d.%d.%d ", ((ipAddr & 0xff000000)>>24),((ipAddr & 0x00ff0000)>>16),((ipAddr & 0x0000ff00)>>8),(ipAddr & 0x000000ff));
				printf("(Mask= %d.%d.%d.%d)\n", ((subnet_mask & 0xff000000)>>24),((subnet_mask & 0x00ff0000)>>16),((subnet_mask & 0x0000ff00)>>8),(subnet_mask & 0x000000ff));
			}
			else
			{
				printf(" ForceIP successful but could not read resulting configuration back from the camera!\n");
			}
			GevCloseCamera(&handle);
		}
		else
		{
			printf(" ForceIP successful but camera not accessible !\n");
		}
	}
	else
	{
		int decstatus = status;
		if ((status & 0x0000F000) == 0x0000F000) decstatus |= 0xFFFF0000;
		if ( (status > 0x8000) && (status < 0x8FFF))
		{
			// Error from protocol.
			printf(" ForceIP failed - status = 0x%x (possibly no response from camera)\n", status); 
		}
		else
		{
			// Error from API
			printf(" ForceIP failed - status = %d\n", decstatus); 
		}
	}
	GevApiUninitialize();
	return 0;
}


GEV_STATUS ForceCameraIPAndMode( BOOL persistentIPMode, UINT32 macHi, UINT32 macLo, UINT32 IPAddress, UINT32 subnet_mask)
{
	GEV_STATUS status = -1;
	
	// Camera must be closed (an open camera will disappear from its current settings).
	// Force the IP for the identified device.
	status = GevForceCameraIPAddress( macHi, macLo, IPAddress, subnet_mask);
	
	if (status == 0)
	{
		int num = 0;
		GEV_CAMERA_HANDLE handle;
		
		// Forced a new IP address and subnet mask. Get a handle to the camera and save the settings.
		// First - refresh the camera list with the new camera.
		num = GevDeviceCount();
		
		// Open the camera by address;
		status = GevOpenCameraByAddress( IPAddress, GevExclusiveMode, &handle);
		if (status == 0)
		{
			// Have access to the camera. Save the settings so they are not lost at powerup.
			UINT32 config = 0;

			// Get the current IP config setting (status/mode).
			status = GevReadRegisterByName( handle, "GevCurrentIPConfiguration", 0, sizeof(UINT32), &config);
			if (status == 0)
			{
				// Save the new settings.
				if (persistentIPMode)
				{
					// Now, write the persistent IP address and persistent subnet mask.
					status = GevWriteRegisterByName( handle, "GevPersistentIPAddress", 0, sizeof(UINT32), &IPAddress);
					status = GevWriteRegisterByName( handle, "GevPersistentSubnetMask", 0, sizeof(UINT32), &subnet_mask);

					// Mask the bits properly.
					config &= ~IPCONFIG_MODE_DHCP;
					config |= IPCONFIG_MODE_PERSISTENT;

					// Write the IP config register back.
					status = GevWriteRegisterByName( handle, "GevCurrentIPConfiguration", 0, sizeof(UINT32), &config);
					
				}
				else
				{
					// Automatic mode (LLA / DHCP) - disable the persistent IP setting.
					// Mask the bits properly.
					config &= ~IPCONFIG_MODE_PERSISTENT;
					config |= IPCONFIG_MODE_DHCP;

					// Write the IP config register back.
					status = GevWriteRegisterByName( handle, "GevCurrentIPConfiguration", 0, sizeof(UINT32), &config);
				}
			}
			GevCloseCamera(&handle);
		}
	}
	return status;
}

GEV_STATUS GetCameraIPSettings( GEV_CAMERA_HANDLE handle, PUINT32 pIPAddress, PUINT32 pSubnet_mask, BOOL *pPersistentIpMode)
{
	GEV_STATUS status = -1;
	UINT32 value = 0;

	// Get the current status
	status = GevReadRegisterByName( handle, "GevCurrentIPConfiguration", 0, sizeof(UINT32), &value);
	if (status == 0)
	{
		if ( (value & IPCONFIG_MODE_PERSISTENT) != 0 )
		{
			// Persistent mode.
			if (pPersistentIpMode != 0)
			{
				*pPersistentIpMode = TRUE;
			}
		}
		else
		{
			// Auto mode (DHCP/LLA).
			// Persistent mode.
			if (pPersistentIpMode != 0)
			{
				*pPersistentIpMode = FALSE;
			}
			
		}
		// Read the current IP address and subnet mask.				
		if ( pIPAddress != NULL)
		{
			status = GevReadRegisterByName( handle, "GevCurrentIPAddress", 0, sizeof(UINT32), pIPAddress);
		}
		if ( pSubnet_mask != NULL)
		{
			status = GevReadRegisterByName( handle, "GevCurrentSubnetMask", 0, sizeof(UINT32), pSubnet_mask);
		}
	}
	return status;
}

