#include <gtk/gtk.h>
#include <glade/glade.h>
#include <pthread.h>
#include <stdlib.h>

#include "gevapi.h"

typedef struct
{
	int 			thread_exit;
	int 			thread_running;
	pthread_t	thread;
	GtkListStore	*table;
	GtkWidget 		*window;
	GtkWidget		*tree;
	_EVENT		updateEvent;
} THREAD_CONTEXT, *PTHREAD_CONTEXT;

THREAD_CONTEXT thread_context = {0};

// Keep track of all the label widgets created (by ROW) so we can clean them up.

void clean_exit()
{
	// Close out all the GigE threads (etc...)
	if (thread_context.thread_running)
	{
		void *retval;
		thread_context.thread_exit = TRUE;
		_SetEvent(&thread_context.updateEvent);
		pthread_join(thread_context.thread, &retval);
		_DestroyEvent(thread_context.updateEvent);
	}

	// Exit the program.
	gtk_main_quit();
}

void window1_activate_default_cb( GtkWidget *widget, gpointer user_data)
{
	// Start a thread here to perform periodic "discovery" of GigE devices and to populate status table.
	fprintf(stderr,"Activate CB\n");
	fflush(stderr);
}

void on_window1_destroy( GtkWidget *widget, gpointer user_data)
{
	clean_exit();
}

void exit_button_clicked_cb( GtkWidget *widget, gpointer user_data)
{
	clean_exit();
}

GtkWidget *make_label( char *data, int max_size, int default_size)
{
	GtkWidget *label;
	char *label_string;

	// Make the label test
	label_string = g_markup_printf_escaped( "<small>%s</small>", data);

	// Make the label (set its options)
	label = gtk_label_new(NULL);
	gtk_label_set_width_chars(GTK_LABEL(label), default_size);
	gtk_label_set_max_width_chars(GTK_LABEL(label), max_size);
	gtk_label_set_selectable(GTK_LABEL(label), FALSE);
	gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
	gtk_label_set_single_line_mode(GTK_LABEL(label), TRUE);
	
	// Set the actual marked up text;
	gtk_label_set_markup( GTK_LABEL(label), label_string);
	g_free(label_string);

	return label;
}

gchar *column_labels[10] = { "Manufacturer", "Model", "Serial number", "MAC address", "Status", "Camera IP Address", "NIC IP Address", "MaxPktSize", "F/W Ver", "User name"};

gchar *debug[11] = { "DALSA", "GenieHM the big one", "S12345678", "00:11:22:33:44:55", "Available", "111.222.123.201", "111.222.123.200",  "1500", "32000", "Roger_0"};

void	add_label_row( GtkWidget *table, int row_number, char **data)
{
	int i;
	GtkAttachOptions xoptions;
	GtkAttachOptions yoptions;

	for (i = 0; i < 12; i++)
	{
			GtkWidget *label = make_label(data[i], 32, -1);

			gtk_table_attach_defaults(GTK_TABLE(table), label, i, i+1, row_number, row_number+1);
	}
}

void	write_label_row( GtkWidget *table, int row_number, char **data)
{

}


void blank_label_row( GtkWidget *table, int row_number)
{

}


#define MANUF_INDEX				0
#define MODEL_INDEX				1
#define SERIAL_NUMBER_INDEX	2
#define MAC_ADDRESS_INDEX		3
#define STATUS_INDEX				4
#define CAM_IP_INDEX				5
#define NIC_IP_INDEX				6
#define PKT_SIZE_INDEX			7
#define VERSION_INDEX			8
#define USER_NAME_INDEX			9
char g_ListStrings[10][64] = {0};


#define MAX_CAMERAS	64
void table_handler( THREAD_CONTEXT *context)
{
	int i = 0;
	context->thread_running = TRUE;
	int numCamera = 0;
	int prev_numCamera = 0;
	int first_time = 1;
	int base_w = 0;
	int base_h = 0;
	int tree_w = 0;
	int tree_h = 0;
	int max_packet_size = 0;
	int timeout = 20000;
	GEV_STATUS status = 0;
	GtkTreeIter	iter;			// Iterator as entries are added after first row.
	GtkTreeIter	base_iter;	// Iterator for first row in table (after headings).
	GtkListStore *table;
	GEV_DEVICE_INTERFACE  *pCamera = NULL;


	if (context == NULL)
	{
		pthread_exit(0);
	}
	table = context->table;

	// Need an initial blank row (to show there are no cameras).
	gtk_list_store_append(table, &iter);
	
	// Allocate storage for the camera data returned from the API.
	pCamera = (GEV_DEVICE_INTERFACE *) malloc(MAX_CAMERAS * sizeof(GEV_DEVICE_INTERFACE));
	if (pCamera != NULL)
	{
		while (!context->thread_exit)
		{
			memset( pCamera, 0, (MAX_CAMERAS * sizeof(GEV_DEVICE_INTERFACE)));
			status = GevGetCameraList( pCamera, MAX_CAMERAS, &numCamera);

			if (numCamera != 0)
			{
				if (numCamera > MAX_CAMERAS)
				{
					numCamera = MAX_CAMERAS;
				}

				// Cameras found - update list display for "numCamera" entries.		
				for (i = 0; i < numCamera; i++)
				{

					max_packet_size = 0;
					// Get the Manufacturer
					memset(g_ListStrings[MANUF_INDEX], 0, 64);
					sprintf(g_ListStrings[MANUF_INDEX], "%s", pCamera[i].manufacturer);
					if (!strlen(g_ListStrings[MANUF_INDEX]))
					{
						sprintf(g_ListStrings[MANUF_INDEX], "?");
					}

					// Get the model
					memset(g_ListStrings[MODEL_INDEX], 0, 64);
					sprintf(g_ListStrings[MODEL_INDEX], "%s", pCamera[i].model);
					if (!strlen(g_ListStrings[MODEL_INDEX]))
					{
						sprintf(g_ListStrings[MODEL_INDEX], "?");
					}

					// Get the serial number
					memset(g_ListStrings[SERIAL_NUMBER_INDEX], 0, 64);
					sprintf(g_ListStrings[SERIAL_NUMBER_INDEX], "%s", pCamera[i].serial);
					if (!strlen(g_ListStrings[SERIAL_NUMBER_INDEX]))
					{
						sprintf(g_ListStrings[SERIAL_NUMBER_INDEX], "?");
					}

					// Get the MAC address
					memset(g_ListStrings[MAC_ADDRESS_INDEX], 0, 64);
					sprintf(g_ListStrings[MAC_ADDRESS_INDEX], "%02X:%02X:%02X:%02X:%02X:%02X", 
									(pCamera[i].macHigh & 0x00ff00)>>8,   (pCamera[i].macHigh & 0x00ff),
									(pCamera[i].macLow & 0xff000000)>>24, (pCamera[i].macLow & 0x00ff0000)>>16,
									(pCamera[i].macLow & 0x0000ff00)>>8,  (pCamera[i].macLow & 0x000000ff));

					// Get the status (???)-> this needs an actual connection to the device (not good)
					memset(g_ListStrings[STATUS_INDEX], 0, 64);
					//if (!first_time)
					if (1)
					{
						GEV_CAMERA_HANDLE handle = NULL;
						GEV_STATUS status = GevOpenCamera( &pCamera[i], GevMonitorMode, &handle);
						if (status == GEV_STATUS_SUCCESS	)
						{
							GEV_CAMERA_OPTIONS options = {0};
							sprintf(g_ListStrings[STATUS_INDEX], "Available"); 
							
							// Get the max packet size here !!!!
							GevGetCameraInterfaceOptions( handle, &options);
							max_packet_size = (options.streamPktSize & 0x000FFFF);

							// Close the camera handle.
							GevCloseCamera(&handle);
						}
						else
						{
							if (status == GEV_STATUS_ACCESS_DENIED)
							{
								sprintf(g_ListStrings[STATUS_INDEX], "Connected"); 
							}
							else
							{
								sprintf(g_ListStrings[STATUS_INDEX], "Unavailable");
							}
						}
					}
					else
					{	
						sprintf(g_ListStrings[STATUS_INDEX], "Checking..."); 
					}

					// Get the Camera IP address
					memset(g_ListStrings[CAM_IP_INDEX], 0, 64);
					if (pCamera[i].ipAddr == 0)
					{
						sprintf(g_ListStrings[CAM_IP_INDEX], "---.---.---.---");
					}
					else
					{
						sprintf(g_ListStrings[CAM_IP_INDEX], "%3d.%3d.%3d.%3d", 
									(pCamera[i].ipAddr & 0xff000000)>>24, (pCamera[i].ipAddr & 0x00ff0000)>>16,
									(pCamera[i].ipAddr & 0x0000ff00)>>8,  (pCamera[i].ipAddr & 0x000000ff)); 
					}

					// Get the NIC IP address
					memset(g_ListStrings[NIC_IP_INDEX], 0, 64);
					if (pCamera[i].host.ipAddr == 0)
					{
						sprintf(g_ListStrings[NIC_IP_INDEX], "---.---.---.---");
					}
					else
					{
						sprintf(g_ListStrings[NIC_IP_INDEX], "%3d.%3d.%3d.%3d", 
									(pCamera[i].host.ipAddr & 0xff000000)>>24, (pCamera[i].host.ipAddr & 0x00ff0000)>>16,
									(pCamera[i].host.ipAddr & 0x0000ff00)>>8,  (pCamera[i].host.ipAddr & 0x000000ff)); 
					}

					// Get the Maximum Packet size -> this needs an actual connection to the device (not good)
					memset(g_ListStrings[PKT_SIZE_INDEX], 0, 64);
					if (max_packet_size == 0)
					{
						sprintf(g_ListStrings[PKT_SIZE_INDEX], "1500 (def)");
					}
					else
					{
						sprintf(g_ListStrings[PKT_SIZE_INDEX], "%d", max_packet_size);
					}
					
					// Get the Firmware version info
					memset(g_ListStrings[VERSION_INDEX], 0, 64);
					sprintf(g_ListStrings[VERSION_INDEX], "%s", pCamera[i].version);
					if (!strlen(g_ListStrings[VERSION_INDEX]))
					{
						sprintf(g_ListStrings[VERSION_INDEX], "?");
					}

					// Get the User defined name
					memset(g_ListStrings[USER_NAME_INDEX], 0, 64);
					sprintf(g_ListStrings[USER_NAME_INDEX], "%s", pCamera[i].username);
					if (!strlen(g_ListStrings[USER_NAME_INDEX]))
					{
						sprintf(g_ListStrings[USER_NAME_INDEX], "<Empty>");
					}

					// Set the string list for this camera.
					if (i > 0)
					{
						// First one uses the initial blank entry.
						// This one might need a new entry for storgae.
						if ( !gtk_tree_model_iter_next( GTK_TREE_MODEL(table), &iter) )
						{
							gtk_list_store_append(table, &iter);
						}
					}
					gtk_list_store_set( table, &iter, 0, g_ListStrings[0], 1, g_ListStrings[1], 2, g_ListStrings[2], 3, g_ListStrings[3], \
												 4, g_ListStrings[4], 5, g_ListStrings[5], 6, g_ListStrings[6], 7, g_ListStrings[7], \
												 8, g_ListStrings[8], 9, g_ListStrings[9],   -1);
				}				
			}

			if (first_time)
			{
				// Get the base Window size (adjust it going forward as cameras are added / removed)
				gtk_window_get_size(GTK_WINDOW(context->window), &base_w, &base_h);
			}

			// See if the number of cameras has changed.
			if (prev_numCamera != numCamera)
			{
				// Clear display between "numCamera" entry and "prev_numCamera" entry.
				if (prev_numCamera > numCamera)
				{
					// Delete the list entries between the end of the list and the end of the prevous list.
					for (i = numCamera; i < prev_numCamera; i++)
					{
						if ( gtk_tree_model_iter_next( GTK_TREE_MODEL(table), &iter) )
						{
							gtk_list_store_remove( table, &iter);
							gtk_tree_model_get_iter_first( GTK_TREE_MODEL(table), &iter);
						}
						else
						{
							break;
						}
					}
					// Check if there are now no cameras at all (the first blank entry needs to be set up).
					if ( numCamera == 0)
					{
						gtk_tree_model_get_iter_first( GTK_TREE_MODEL(table), &iter);
						gtk_list_store_remove( table, &iter);
						gtk_list_store_append(table, &iter);
					}
				}
			}
			
			// Resize the window height if necessary to view all entries in the table/list object - 
			// Only if necessary though.
			{
				int h, w;
				gtk_window_get_size(GTK_WINDOW(context->window), &w, &h);
				if ( h < (base_h + 12*numCamera))
				{
					gtk_window_resize(GTK_WINDOW(context->window), w, base_h+12*numCamera);	// #New #Tweaked.
				}			
			}
			
			// Wait here for the prescribed time/timeout (or for when signalled)
			if (first_time)
			{
				first_time = 0;
				timeout = (numCamera > 1) ? 200 : 20000;
			}
			else
			{
				if (timeout == 200) timeout = 20000; 
			}
			_WaitForEvent(&context->updateEvent, timeout);
			_ClearEvent(&context->updateEvent);
			first_time = 0;
			prev_numCamera = numCamera;	
			gtk_tree_model_get_iter_first( GTK_TREE_MODEL(table), &iter);
		}
		free(pCamera);
	}
	else
	{
		fprintf(stderr, "Error - no memory\n");
	}

	// Exit the thread.
	pthread_exit(0);
}

void start_tablehandler( GtkWidget *tree, GtkWidget *window, GtkListStore *table)
{
	pthread_attr_t	attr;
	pthread_attr_init( &attr);
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE);

	_CreateEvent(&thread_context.updateEvent);

	thread_context.thread_running = FALSE;
	thread_context.thread_exit = FALSE;
	thread_context.tree = tree;
	thread_context.window = window;
	thread_context.table = table;
	pthread_create( &thread_context.thread, &attr, (void *)table_handler, &thread_context);

}

#define N_COLUMNS 10
GType column_types[N_COLUMNS] = { G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, \
											 G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, \
											 G_TYPE_STRING, G_TYPE_STRING };

int main(int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *tree;
	GtkListStore *table;
	GtkTreeIter	iter;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
   GladeXML *xml;
	int i;

   gtk_init(&argc, &argv);
   
   xml = glade_xml_new("/usr/dalsa/GigeV/tools/GigeDeviceStatus/GigeDeviceStatus.glade", NULL, NULL);
   glade_xml_signal_autoconnect(xml);

	// Get the tree widget from the xml UI def.
	tree = glade_xml_get_widget(xml, "treeview1");
	window = glade_xml_get_widget(xml, "window1");

	// Create a list store for the tree to use as a model
	table = gtk_list_store_newv( N_COLUMNS, column_types);  
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(table));

	// Set up the columns
	for (i = 0; i < N_COLUMNS; i++)
	{
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(column_labels[i], renderer, "text", i, NULL);		
		gtk_tree_view_append_column( GTK_TREE_VIEW(tree), column);
	}

	// Set the intial size of the table (list) object.
	{
		if (1)
		{
			// Get the tree widget size as a base 
			// (Note : gtk_window_resize may not be available in the future).
			GtkRequisition requisition;
			gtk_widget_size_request(tree, &requisition);

			// Re-size the main window.
			// Note : add a little bit to account for the Window Manager decorating the window borders.
			 gtk_window_resize(GTK_WINDOW(window), requisition.width+24, requisition.height+30);
		}
		else
		{
			// Just set the initial window size. 
			int w, h;
			w = 960;	// 128 characters with a font size of 8.
			h = 56;
			gtk_widget_set_size_request(window, w, h);
		}
		 
	}
	
	// Start a thread to fill in the table elements.
	start_tablehandler(tree, window, table);	
	gtk_widget_show_all(GTK_WIDGET(tree));

   gtk_main();

   return 0;
}



