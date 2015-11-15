Software Architecture Overview
==============================

In order to offer S.O.N.I.A. Software more modularity and maintainability, we decided to construct our architecture with layers. All layers should be totaly independent of the one above in order to minimize the impact of a modification on a packet API.

![AUV7 Design](../assets/img/software_auv7_design.jpeg)

As you can see on the design scheme, you will find three different layers wich are:

- The Providers
- The Stacks
- The Controllers

There is also two categories of packages that are used by all the packages:

- The Libraries
- The GUI Softwares

As the libraries are used by all the software, this is a critical component of our system.

Providers
---------

Stacks
------

Controllers
-----------

Libraries
---------

GUI
---
