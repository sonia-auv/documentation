ROS Interface
=============

This file describe how the provider vision software is interfacing with ROS.
You will find here every information you need about the services and messages.

Services
--------

### `vision_server/vision_server_execute_cmd`

**ROS Service command :**

`rosservice call vision_server/vision_server_execute_cmd node_name filterchain_name media_name cmd`

**Parameters :**

* node_name : string, nom de l’exécution, correspond au topic qui sera créé et où seront publiées les données
* filterchain_name : string, nom de la filterchain à appliquer
* media_name : string, nom du média à partir duquel charger les images (voir plus bas pour la liste des médias)
* cmd : uint8, valeur de la commande (1 : START, 2 : STOP) 

**Return Value :**

### `vision_server/vision_server_get_information_list`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_get_media_param`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_set_media_param`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_copy_filterchain`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

Manages the ROS service vision_server_copy_filterchain.

**ROS Service command :**

`rosservice call vision_server/vision_server_copy_filterchain filterchain_to_copy filterchain_new_name`

**Parameters :**

* `filter_chain_name_to_copy` : The name of the filterchain to copy.
* `filter_chain_new_name` : The name of the new filterchain (the copy).


**Return Value :**


### `vision_server/vision_server_manage_filterchain`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_get_filterchain_filter_param`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_set_filterchain_filter_param`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_get_filterchain_filter_all_param`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_get_filterchain_filter`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_manage_filterchain_filter`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_save_filterchain`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_set_filterchain_filter_order`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_get_filterchain_from_execution`

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**

### `vision_server/vision_server_get_media_from_execution`

**ROS Service command :**

`rosservice call vision_server/vision_server_get_media_from_execution exec_name`

**Parameters :**

* `exec_name` : Name of the running execution.

**Return Value :**

### `vision_server/vision_server_set_filterchain_filter_observer`

Sets the observer to the filter given as parameter.

As the processing of a filterchain is like a pipe, it is possible to
observe the render of a specific filter and all the filters before.
This method set this "cursor".
The filter has to be used by a filterchain used by a running execution.

**ROS Service command :**

``

**Parameters :**

* 

**Return Value :**


Topics
------
