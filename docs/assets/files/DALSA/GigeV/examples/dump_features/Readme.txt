This directory contains four example programs demonstrating feature 
access.

dumpfeatures (in featuretree.cpp) 
   This program dumps the feature hierarchy by category to stdout. From
   there it can be piped to a file for storage.
   
savefeatures (in savefeatures.cpp)
   This program reads features from the camera and formats them in 
   { feature_name value_string } pairs per output line. The output can
   be to a file or to stdout.
   Note: Since the output is intended to be used to restore features to
   a camera, only those that are RW and "streamable" are saved.
   
loadfeatures (in loadfeatures.cpp)
   This program reads { feature_name value_string } pairs from a file
   and outputs them to a camera. The camera is put into 
   "feature streaming" mode first to prevent feature dependencies from
   affecting the accesses. 
   Note: Only "streamable" features can be restored in this manner.
   
c_loadfeatures (in feature_loader.c)
   This program reads { feature_name value_string } pairs from a file
   and outputs them to a camera. The camera is put into 
   "feature streaming" mode first to prevent feature dependencies from
   affecting the accesses. 
   Note: Only "streamable" features can be restored in this manner.
	This program uses only the C API to perform the loading.
  