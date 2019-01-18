/****************************************************************************** 
Copyright (c) 2009-2011, Teledyne DALSA Inc.
All rights reserved.

File : gevapi.c
	Public API for GigEVision C library.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:
	-Redistributions of source code must retain the above copyright 
	notice, this list of conditions and the following disclaimer. 
	-Redistributions in binary form must reproduce the above 
	copyright notice, this list of conditions and the following 
	disclaimer in the documentation and/or other materials provided 
	with the distribution. 
	-Neither the name of DALSA nor the names of its 
	contributors may be used to endorse or promote products derived 
	from this software without specific prior written permission. 
===============================================================================
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/
#include "gevapi.h"				//!< GEV API definitions.

// Default DALSA_GENICAM_GIGE_REGS structure. (Legacy compatibility)
static DALSA_GENICAM_GIGE_REGS m_Default_Regs = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",         NOREF_ADDR, RO, FALSE, stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceScanType",                NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceMaxThroughput",           NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceRegistersStreamingStart", NOREF_ADDR, WO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersStreamingEnd",   NOREF_ADDR, WO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersCheck",          NOREF_ADDR, WO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersValid",          NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "SensorWidth",                   NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",                  NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                      NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "HeightMax",                     NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""}, 
	{ "Width",                         NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  4, 4,8192,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                        NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,1,16383,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                       NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  0, 0,8188,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetY",                       NOREF_ADDR, RW, TRUE , integerReg, 4,  4,  0,  0,  0, 0,16383,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "LinePitch",                     NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningHorizontal",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningVertical",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "DecimationHorizontal",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "DecimationVertical",            NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ReverseX",                      NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "ReverseY",                      NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "PixelColorFilter",              NOREF_ADDR, RO, FALSE, fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""}, 	 
	{ "PixelCoding",                   NOREF_ADDR, RO, FALSE, fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},
	{ "PixelSize",                     NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},     
	{ "PixelFormat",                   NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "PixelDynamicRangeMin",          NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "PixelDynamicRangeMax",          NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "TestImageSelector",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionMode",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              NOREF_ADDR, WO, FALSE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               NOREF_ADDR, WO, FALSE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",              NOREF_ADDR, WO, FALSE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionArm",                NOREF_ADDR, RW, FALSE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},
	{ "AcquisitionFrameCount",         NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameRateMax",       NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateMin",       NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateRaw",       NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "AcquisitionFrameRateAbs",       NOREF_ADDR, RW, FALSE, floatReg,   4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionLineRateRaw",        NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},  
	{ "AcquisitionLineRateAbs",        NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},  	 
	{ "AcquisitionStatusSelector",     NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},  
	{ "AcquisitionStatus",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "AcquisitionStatusSelector", ""},  	 
	{ "TriggerSelector",               NOREF_ADDR, WO, FALSE, fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},
	{ "TriggerMode",                   NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerSoftware",               NOREF_ADDR, WO, FALSE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "TriggerSelector", ""},	
	{ "TriggerSource",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerActivation",             NOREF_ADDR, RW, FALSE, integerReg, 8,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},  
	{ "TriggerOverlap",                NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},  
	{ "TriggerDelayAbs",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerDelayRaw",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerDivider",                NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerMultiplier",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "ExposureMode",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ExposureAlignment",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureDelay",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, // NOTE : same as TriggerDelayRaw (Genie only, not in STD)	
	{ "ExposureTimeRaw",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeAbs",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ExposureAuto",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ExposureTimeMin",               NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  3,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0, 3300, 0, 0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
 
	

	/* I/O stuff (some are Genie Specific registers) - more like FEATUREs than actual registers !! */
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
	{ "LineSelector",                  NOREF_ADDR, RW, FALSE, intVal,     4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",             ""},					
	{ "LineMode",                      NOREF_ADDR, RO, FALSE, intVal,     4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},		
	{ "LineInverter",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Not in GENIE!!! Polarity setting for selected input or output
	{ "LineStatus",                    NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// 0 or 1 (min 0 max 1).			
	{ "LineStatusAll",                 NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",             ""},	// Not in GENIE!!!
	{ "LineSource",                    NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Standard version for "OutputLineLineEventSource"				
	{ "OutputLineEventSource",         NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Genie version of "LineSource" 
	{ "LineFormat",                    NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Not in GENIE
	{ "UserOutputValue",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// Standard Version of "OutputLineValue"
	{ "OutputLineValue",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},  // Genie Version of "UserOutputValue"
	{ "UserOutputSelector",            NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "UserOutputValueAll",            NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "UserOutputValueAllMask",        NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "InputLinePolarity",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},				
	{ "InputLineDebouncingPeriod",     NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OutputLinePulsePolarity",       NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OutputLineMode",                NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OutputLinePulseDelay",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},		 
	{ "OutputLinePulseDuration",       NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "CounterSelector",               NOREF_ADDR, RO, FALSE, fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},	
	{ "CounterEventSource",            NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterLineSource",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},		
	{ "CounterReset",                  NOREF_ADDR, WO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "CounterSelector", ""},
	{ "CounterValue",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterValueAtReset",           NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterDuration",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterStatus",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterTriggerSource",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterTriggerActivation",      NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "TimerSelector",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",              ""},	
	{ "TimerDurationAbs",              NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
	{ "TimerDurationRaw",              NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
	{ "TimerDelayAbs",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
	{ "TimerDelayRaw",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
	{ "TimerValueAbs",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
	{ "TimerValueRaw",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
	{ "TimerStatus",                   NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
	{ "TimerTriggerSource",            NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
	{ "TimerTriggerActivation",        NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TimerSelector", ""},	
 
	{ "EventSelector",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",              ""}, 
	{ "EventNotification",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "EventSelector", ""}, 

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
	{ "GainSelector",                  NOREF_ADDR, WO, FALSE, fixedVal,   4,  4,  0,  0,  0,   0,  0,          0,          0, NULL,    "",        ""},	
	{ "GainRaw",                       NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "GainAbs",                       NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	
	{ "GainAuto",                      NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	
	{ "GainAutoBalance",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "BlackLevelSelector",            NOREF_ADDR, WO, FALSE, fixedVal,   4,  4,  0,  0,  0,   0,  0,          0,          0, NULL,    "",        ""},	 
	{ "BlackLevelRaw",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 
	{ "BlackLevelAbs",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	
	{ "BlackLevelAuto",                NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	
	{ "BlackLevelAutoBalance",         NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "WhiteClipSelector",             NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "WhiteClipRaw",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "WhiteClipSelector", ""},	
	{ "WhiteClipAbs",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "WhiteClipSelector", ""},	
	{ "BalanceRatioSelector",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "BalanceRatioAbs",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BalanceRatioSelector", ""},	
	{ "BalanceWhiteAuto",              NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "Gamma",                         NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "LUTSelector",                   NOREF_ADDR, WO, FALSE, fixedVal,   4,  4,  0,  0,  0,   0,  0,        0x1,        0x1, NULL,    "",        ""},
	{ "LUTEnable",                     NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "LUTIndex",                      NOREF_ADDR, RW, FALSE, intVal,     4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "LUTValue",                      NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", "LUTIndex"},
	{ "LUTValueAll",                   NOREF_ADDR, RW, FALSE, dataArea,   4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},	

	{ "UserSetDefaultSelector",        NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   NOREF_ADDR, WO, FALSE, integerReg, 4,  4,  0,  0,  1,   0,  0,        0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetSave",                   NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,   0,  0,        0x1,        0x1, NULL,    "UserSetSelector"}, 

	//================================================================
	// Gev transport layer registers. - Most are Fixed by GigE Vision Standard.
	//
	{ "PayloadSize",                   NOREF_ADDR, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

/*=========================================== Add these in later - the code already uses them since they are fixed.
	GEV_REGISTER GevVersionMajor; 
	GEV_REGISTER GevVersionMinor; 
	GEV_REGISTER GevDeviceModeIsBigEndian; 
	GEV_REGISTER GevDeviceModeCharacterSet; 
	GEV_REGISTER GevInterfaceSelector; 
	GEV_REGISTER GevMACAddress; 
========================================================================================*/
/*   |"FeatureName"                            |Addres     |Acc|Avail |  Type |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*   |                                         |           |   |      |       |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GevSupportedIPConfigurationLLA",          0x00000010, RO, TRUE,  bitReg,  4,  4,  0,  0,  0, 29, 29,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevSupportedIPConfigurationDHCP",         0x00000010, RO, TRUE,  bitReg,  4,  4,  0,  0,  0, 30, 30,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevSupportedIPConfigurationPersistentIP", 0x00000010, RO, TRUE,  bitReg,  4,  4,  0,  0,  0, 31, 31,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/*===============================================================================
	GEV_REGISTER GevCurrentIPConfiguration; 
===============================================================================*/
   { "GevCurrentIPConfigurationLLA",            NOREF_ADDR, RO, FALSE,  bitReg,  4,  4,  0,  0,  0, 29, 29,  0xffffffff, 0xffffffff, NULL,    "",        ""},		
	{ "GevCurrentIPConfigurationDHCP",           NOREF_ADDR, RO, FALSE,  bitReg,  4,  4,  0,  0,  0, 30, 30,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "GevCurrentIPConfigurationPersistentIP",   NOREF_ADDR, RO, FALSE,  bitReg,  4,  4,  0,  0,  0, 31, 31,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/*  |"FeatureName"                |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*  |                             |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GevCurrentIPConfiguration",   0x00000014, RW, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevCurrentIPAddress",         0x00000024, RO, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevCurrentSubnetMask",        0x00000034, RO, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevCurrentDefaultGateway",    0x00000044, RO, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevPersistentIPAddress",      0x0000064C, RW, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevPersistentSubnetMask",     0x0000065C, RW, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevPersistentDefaultGateway", 0x0000066C, RW, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevFirstURL",                 0x00000200, RO, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevSecondURL",                0x00000400, RO, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevNumberOfInterfaces",       0x00000400, RO, TRUE, integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/*=========================================== Add these in later - the code already uses them since they are fixed.
	GEV_REGISTER GevMessageChannelCount; 
	GEV_REGISTER GevStreamChannelCount;

	GEV_REGISTER GevSupportedOptionalCommandsUserDefinedName; 
	GEV_REGISTER GevSupportedOptionalCommandsSerialNumber; 
	GEV_REGISTER GevSupportedOptionalCommandsEVENTDATA; 
	GEV_REGISTER GevSupportedOptionalCommandsEVENT; 
	GEV_REGISTER GevSupportedOptionalCommandsPACKETRESEND; 
	GEV_REGISTER GevSupportedOptionalCommandsWRITEMEM; 
	GEV_REGISTER GevSupportedOptionalCommandsConcatenation; 

	GEV_REGISTER GevHeartbeatTimeout; 
	GEV_REGISTER GevTimestampTickFrequency; 
	{ "GevTimestampControlLatch",               0x00000944, WO, TRUE,  fixedVal,   4,  4,  0,  0,  1,  0,  0,    0x2,           0x2,    NULL,    "",        ""},
	{ "GevTimestampControlReset",               0x00000944, WO, TRUE,  fixedVal,   4,  4,  0,  0,  1,  0,  0,    0x1,           0x1,    NULL,    "",        ""},

	GEV_REGISTER GevTimestampValue;

	// These are managed by the internal API and should NEVER be written to by an application.
	GEV_REGISTER GevCCP;					
	GEV_REGISTER GevMCPHostPort;		
	GEV_REGISTER GevMCDA;				
	GEV_REGISTER GevMCTT;				
	GEV_REGISTER GevMCRC;				
	GEV_REGISTER GevStreamChannelSelector; 
	GEV_REGISTER GevSCPInterfaceIndex;
	GEV_REGISTER GevSCPHostPort;			
	GEV_REGISTER GevSCPFireTestPacket;	
	GEV_REGISTER GevSCPDoNotFragment;	
	GEV_REGISTER GevSCPSBigEndian;		
	GEV_REGISTER GevSCPSPacketSize;
	GEV_REGISTER GevSCPD; 
	GEV_REGISTER GevSCDA;
	
=========================*/

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GevLinkSpeed",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "GevIPConfigurationStatus",      NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie

	//============================================
	//	Chunk data support (not in Genie)
	//
	{ "ChunkModeActive",               NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},					// (**) Not in genie
	{ "ChunkSelector",                 NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},					// (**) Not in genie
	{ "ChunkEnable",                   NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "ChunkSelector", ""},	// (**) Not in genie
	{ "ChunkOffsetX",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkOffsetY",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkWidth",                    NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkHeight",                   NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkPixelFormat",              NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkDynamicRangeMax",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkDynamicRangeMin",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkTimestamp",                NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkLineStatusAll",            NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkCounterSelector",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkCounter",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "ChunkCounterSelector", ""}, // (**) Not in genie
	{ "ChunkTimerSelector",            NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",        ""},	// (**) Not in genie
	{ "ChunkTimer",                    NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "ChunkTimerSelector", ""},	 // (**) Not in genie


	//============================================
	// File Access support (not in Genie)
	//
	{ "FileSelector",                  NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "",             ""},	      // (**) Not in genie
	{ "FileOperationSelector",         NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", ""},	// (**) Not in genie
	{ "FileOperationExecute",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", "FileOperationSelector"},	// (**) Not in genie
	{ "FileOpenModeSelector",          NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", "FileOperationSelector"},	// (**) Not in genie
	{ "FileAccessOffset",              NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", "FileOperationSelector"},	// (**) Not in genie
	{ "FileAccessLength",              NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", "FileOperationSelector"},	// (**) Not in genie
	{ "FileAccessBuffer",              NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", "FileOperationSelector"},	// (**) Not in genie
	{ "FileOperationStatus",           NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", "FileOperationSelector"},	// (**) Not in genie
	{ "FileOperationResult",           NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", "FileOperationSelector"},	// (**) Not in genie
	{ "FileSize",                      NOREF_ADDR, RW, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff,  NULL,   "FileSelector", ""},	// (**) Not in genie

};

//=============================================================================
//
// Camera-Specific list of register that differ from the default.
//
//
static GEV_REGISTER m_GenieHM_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",         0x10000074, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceScanType",                0x10000010, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceMaxThroughput",           0x10000018, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceRegistersStreamingStart", 0x10000058, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersStreamingEnd",   0x1000005C, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersCheck",          0x10000060, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersValid",          0x10000060, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "SensorWidth",                   0x10000008, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",                  0x1000000C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                      0x1000001C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "HeightMax",                     0x10000020, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""}, 
	{ "Width",                         0x1000002C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                        0x10000030, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                       0x10000024, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetY",                       0x10000028, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "LinePitch",                     0x10000038, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningHorizontal",             0x300010AC, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningVertical",               0x300010B0, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ReverseX",                      0x300000A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "PixelColorFilter",              NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""}, 	 
	{ "PixelCoding",                   NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},
	{ "PixelFormat",                   0x10000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "PixelDynamicRangeMin",          0x10000098, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "PixelDynamicRangeMax",          0x1000009C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "TestImageSelector",             0x30000048, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionMode",               0x10000040, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              0x100000A8, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               0x100000AC, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",              0x100000B0, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionArm",                0x300010A8, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},
	{ "AcquisitionFrameCount",         0x10000044, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameRateMax",       0x30001020, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateMin",       0x3000104C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateRaw",       0x30000008, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TriggerSelector",               NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},
	{ "TriggerMode",                   0x30000038, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerSoftware",               0x3000000C, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  1,         0x1,        0x1, NULL,    "TriggerSelector", ""},	
	{ "TriggerSource",                 0x3000003C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerActivation",             0x30000040, RW, TRUE,  integerReg, 8,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},  // indexed formula	- depends on trigger source. Tied to GPIx reg. 
	{ "TriggerDelayRaw",               0x30000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "ExposureMode",                  0x30000020, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ExposureAlignment",             0x30000030, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureDelay",                 0x30000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // NOTE : same as TriggerDelayRaw (Genie only, not in STD)	
	{ "ExposureTimeRaw",               0x30000024, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeMin",               0x30001040, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               0x30001024, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 

	/* I/O stuff (some are Genie Specific registers) - more like FEATUREs than actual registers !! */
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector       | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string         | string */
	{ "LineSelector",                  NOREF_ADDR, RW, TRUE,  intVal,     4,   4, 0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",             ""},	// 0,1,2,3 for HM					
	{ "LineMode",                      NOREF_ADDR, RO, TRUE,  intVal,     4,   4, 0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Input(0) for line 0,1 : Output(1) for line 2,3 for HM	
	{ "LineStatus",                    0x30000058, RO, TRUE,  integerReg, 8,   4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// 0 or 1 (min 0 max 1).			
	{ "LineSource",                    0x30000070, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Standard version for "OutputLineLineEventSource"				
	{ "OutputLineEventSource",         0x30000070, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Genie version of "LineSource" 
	{ "UserOutputValue",               0x30000068, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// Standard Version of "OutputLineValue"
	{ "OutputLineValue",               0x30000068, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},  	// Genie Version of "UserOutputValue"

	{ "InputLinePolarity",             0x30000040, RW, TRUE,  integerReg,  8,  4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// GENIE ONLY !!!!			
	{ "InputLineDebouncingPeriod",     0x30000050, RW, TRUE,  integerReg,  8,  4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// GENIE ONLY !!!!
	{ "OutputLinePulsePolarity",       0x30000074, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!
	{ "OutputLineMode",                0x3000006C, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!
	{ "OutputLinePulseDelay",          0x30000078, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!	 
	{ "OutputLinePulseDuration",       0x3000007C, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!

	{ "CounterSelector",               NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",                ""},	
	{ "CounterEventSource",            0x30001090, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterLineSource",             0x30001094, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	// (*) in Genie but not in standard	
	{ "CounterReset",                  0x00000944, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "CounterSelector", ""},

	{ "EventSelector",                 0x100000A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",              ""}, 
	{ "EventNotification",             0x100000A4, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "EventSelector", ""}, 

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector         | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string           | string */
	{ "GainSelector",                  NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",             ""},	
	{ "GainRaw",                       0x30000014, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "BlackLevelSelector",            NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",             ""},	 
	{ "BlackLevelRaw",                 0x3000001C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector",   ""},	 

	{ "LUTSelector",                   NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,         0x1,        0x1, NULL,    "",              ""},
	{ "LUTEnable",                     0x30000060, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector",   ""},
	{ "LUTIndex",                      NOREF_ADDR, RW, TRUE,  intVal,     4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector",   ""},
	{ "LUTValue",                      0x31000000, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", "LUTIndex"},
	{ "LUTValueAll",                   0x31000000, RW, TRUE,  dataArea,   2048,   4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,  "LUTSelector", ""},	

	{ "UserSetDefaultSelector",        0x10000070, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",               0x10000064, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   0x10000068, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetSave",                   0x1000006C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 

	{ "PayloadSize",                   0x1000003C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
};

static GEV_REGISTER m_GenieColor_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",         0x10000074, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceScanType",                0x10000010, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceMaxThroughput",           0x10000018, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceRegistersStreamingStart", 0x10000058, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersStreamingEnd",   0x1000005C, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersCheck",          0x10000060, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersValid",          0x10000060, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "SensorWidth",                   0x10000008, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",                  0x1000000C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                      0x1000001C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "HeightMax",                     0x10000020, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""}, 
	{ "Width",                         0x1000002C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                        0x10000030, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                       0x10000024, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetY",                       0x10000028, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "LinePitch",                     0x10000038, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "PixelFormat",                   0x10000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "PixelDynamicRangeMin",          0x10000098, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "PixelDynamicRangeMax",          0x1000009C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "TestImageSelector",             0x30000048, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionMode",               0x10000040, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              0x100000A8, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               0x100000AC, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",              0x100000B0, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionFrameCount",         0x10000044, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameRateMax",       0x31001034, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateMin",       0x3100103C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateRaw",       0x30000008, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TriggerSelector",               NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},
	{ "TriggerMode",                   0x30000038, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerSoftware",               0x3000000C, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "TriggerSelector", ""},	
	{ "TriggerSource",                 0x3000003C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerActivation",             0x30000040, RW, TRUE,  integerReg, 8,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},  // indexed formula	- depends on trigger source. Tied to GPIx reg. 
	{ "TriggerDelayRaw",               0x30000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "ExposureMode",                  0x30000020, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ExposureAlignment",             0x30000030, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureDelay",                 0x30000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // NOTE : same as TriggerDelayRaw (Genie only, not in STD)	
	{ "ExposureTimeRaw",               0x30000024, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeMin",               0x31001040, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               0x31001038, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 

	/* I/O stuff (some are Genie Specific registers) - more like FEATUREs than actual registers !! */
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
	{ "LineSelector",                  NOREF_ADDR, RW, TRUE,  intVal,     4,   4, 0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// 0,1,2,3 for HM					
	{ "LineMode",                      NOREF_ADDR, RO, TRUE,  intVal,     4,   4, 0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Input(0) for line 0,1 : Output(1) for line 2,3 for HM	
	{ "LineStatus",                    0x30000058, RO, TRUE,  integerReg, 8,   4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// 0 or 1 (min 0 max 1).			
	{ "LineSource",                    0x30000070, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Standard version for "OutputLineLineEventSource"				
	{ "OutputLineEventSource",         0x30000070, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Genie version of "LineSource" 
	{ "UserOutputValue",               0x30000068, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// Standard Version of "OutputLineValue"
	{ "OutputLineValue",               0x30000068, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},  	// Genie Version of "UserOutputValue"

	{ "InputLinePolarity",             0x30000040, RW, TRUE,  integerReg,  8,  4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// GENIE ONLY !!!!			
	{ "InputLineDebouncingPeriod",     0x30000050, RW, TRUE,  integerReg,  8,  4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// GENIE ONLY !!!!
	{ "OutputLinePulsePolarity",       0x30000074, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!
	{ "OutputLineMode",                0x3000006C, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!
	{ "OutputLinePulseDelay",          0x30000078, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!	 
	{ "OutputLinePulseDuration",       0x3000007C, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!

	{ "CounterSelector",               NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},	
	{ "CounterEventSource",            0x3100101C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterLineSource",             0x31001020, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	// (*) in Genie but not in standard	
	{ "CounterReset",                  0x00000944, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "CounterSelector", ""},

	{ "EventSelector",                 0x100000A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "EventNotification",             0x100000A4, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "EventSelector"}, 

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
	{ "GainSelector",                  NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",             ""},	
	{ "GainRaw", /*separate RGB*/      0x31000000, RW, TRUE,  integerReg, 4,  4,  1,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "BlackLevelSelector",            NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},	 
	{ "BlackLevelRaw",                 0x3000001C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 
	{ "LUTSelector",                   NOREF_ADDR, RW, TRUE,  intVal,     4,  4,  0,  0,  0,  0,  0,         0x1,        0x1, NULL,    "",            ""},
	{ "LUTEnable",                     0x30000060, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "LUTIndex",                      NOREF_ADDR, RW, TRUE,  intVal,     4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "LUTValue",                      0x30000098, RW, TRUE,  integerReg, 4,  1,  0, 512, 0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", "LUTIndex"},
	{ "LUTValueAll",                   0x30000098, RW, TRUE,  dataArea,   768, 1, 0, 512, 0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},	// (**) Not in Genie

	{ "UserSetDefaultSelector",        0x10000070, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",               0x10000064, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   0x10000068, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetSave",                   0x1000006C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 

	{ "PayloadSize",                   0x1000003C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
};

static GEV_REGISTER m_GenieMono_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceFirmwareVersion",         0x10000074, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceScanType",                0x10000010, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceMaxThroughput",           0x10000018, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceRegistersStreamingStart", 0x10000058, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersStreamingEnd",   0x1000005C, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersCheck",          0x10000094, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0,          0x1,        NULL,    "",        ""},
	{ "DeviceRegistersValid",          0x10000060, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "SensorWidth",                   0x10000008, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",                  0x1000000C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                      0x1000001C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "HeightMax",                     0x10000020, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""}, 
	{ "Width",                         0x1000002C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                        0x10000030, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                       0x10000024, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetY",                       0x10000028, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "LinePitch",                     0x10000038, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningHorizontal",             0x31001030, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningVertical",               0x31001034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ReverseX",                      0x31000000, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "PixelColorFilter",              NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""}, 	 
	{ "PixelCoding",                   NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},
	{ "PixelFormat",                   0x10000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "PixelDynamicRangeMin",          0x10000098, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "PixelDynamicRangeMax",          0x1000009C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "TestImageSelector",             0x30000048, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionMode",               0x10000040, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              0x100000A8, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               0x100000AC, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",              0x100000B0, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionFrameCount",         0x10000044, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameRateMax",       0x31001020, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateMin",       0x31001028, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateRaw",       0x30000008, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TriggerSelector",               NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},
	{ "TriggerMode",                   0x30000038, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerSoftware",               0x3000000C, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "TriggerSelector", ""},	
	{ "TriggerSource",                 0x3000003C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "TriggerActivation",             0x30000040, RW, TRUE,  integerReg, 8,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},  // indexed formula	- depends on trigger source. Tied to GPIx reg. 
	{ "TriggerDelayRaw",               0x30000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	
	{ "ExposureMode",                  0x30000020, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ExposureAlignment",             0x30000030, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureDelay",                 0x30000034, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // NOTE : same as TriggerDelayRaw (Genie only, not in STD)	
	{ "ExposureTimeRaw",               0x30000024, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeMin",               0x3100102C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               0x31001024, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 

	/* I/O stuff (some are Genie Specific registers) - more like FEATUREs than actual registers !! */
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
	{ "LineSelector",                  NOREF_ADDR, RW, TRUE,  intVal,     4,   4, 0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// 0,1,2,3 for HM					
	{ "LineMode",                      NOREF_ADDR, RO, TRUE,  intVal,     4,   4, 0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Input(0) for line 0,1 : Output(1) for line 2,3 for HM	
	{ "LineStatus",                    0x30000058, RO, TRUE,  integerReg, 8,   4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// 0 or 1 (min 0 max 1).			
	{ "LineSource",                    0x30000070, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Standard version for "OutputLineLineEventSource"				
	{ "OutputLineEventSource",         0x30000070, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Genie version of "LineSource" 
	{ "UserOutputValue",               0x30000068, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// Standard Version of "OutputLineValue"
	{ "OutputLineValue",               0x30000068, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},  	// Genie Version of "UserOutputValue"

	{ "InputLinePolarity",             0x30000040, RW, TRUE,  integerReg,  8,  4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// GENIE ONLY !!!!			
	{ "InputLineDebouncingPeriod",     0x30000050, RW, TRUE,  integerReg,  8,  4, 0,  1,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// GENIE ONLY !!!!
	{ "OutputLinePulsePolarity",       0x30000074, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!
	{ "OutputLineMode",                0x3000006C, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!
	{ "OutputLinePulseDelay",          0x30000078, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!	 
	{ "OutputLinePulseDuration",       0x3000007C, RW, TRUE,  integerReg, 48, 24, 2,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "UserOutputSelector", ""},	// GENIE ONLY !!!!

	{ "CounterSelector",               NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},	
	{ "CounterEventSource",            0x3100100C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	
	{ "CounterLineSource",             0x31001010, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "CounterSelector", ""},	// (*) in Genie but not in standard	
	{ "CounterReset",                  0x00000944, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "CounterSelector", ""},

	{ "EventSelector",                 0x100000A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",              ""}, 
	{ "EventNotification",             0x100000A4, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "EventSelector", ""}, 

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
	{ "GainSelector",                  NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},	
	{ "GainRaw",                       0x30000014, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "BlackLevelSelector",            NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""},	 
	{ "BlackLevelRaw",                 0x3000001C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 

	{ "LUTSelector",                   NOREF_ADDR, WO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,          0x1,        0x1, NULL,    "",        ""},
	{ "LUTEnable",                     0x30000060, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "LUTIndex",                      NOREF_ADDR, RW, TRUE,  intVal,     4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "LUTValue",                      0x31000000, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", "LUTIndex"},
	{ "LUTValueAll",                   0x31000000, RW, TRUE,  dataArea,   2048,   4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL, "LUTSelector", ""},	// (**) Not in Genie

	{ "UserSetDefaultSelector",        0x10000070, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",               0x10000064, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   0x10000068, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetSave",                   0x1000006C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 

	{ "PayloadSize",                   0x1000003C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
};


static GEV_REGISTER m_Cetus_Demo_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",         0x10000074, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceScanType",                NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceRegistersValid",          NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  1,  0,  1,  0x1,        0,          NULL,    "",        ""},
	{ "SensorWidth",                   NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0, 1316, 0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",                  NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0, 1318, 0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                      NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0, 1316, 0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "HeightMax",                     NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0, 1318, 0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""}, 
	{ "Width",                         0x51000008, RO, TRUE,  integerReg, 4,  4,  0,  0, 1316, 4,1316,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                        0x5100000C, RO, TRUE,  integerReg, 4,  4,  0,  0, 1318, 1,1318,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                       NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetY",                       NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "BinningHorizontal",             0x51000018, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningVertical",               0x51000018, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "PixelSize",                     NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0, 14,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},     
	//{ "PixelFormat",                   NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0, 0x01100025, 0, 0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PixelFormat",                   0x10000034, RO, TRUE,  fixedVal,   4,  4,  0,  0, 0, 0, 0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "TestImageSelector",             0x5F000000, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  8,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionMode",               /*0x51002000*/0x10000040, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              /*0x51002028*/0x100000A8, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               /*0x51002010*/0x100000AC, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",              /*NOREF_ADDR*/0x100000B0, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionFrameCount",         0x10000044, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "EventSelector",                 0x100000A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",              ""}, 
	{ "EventNotification",             0x100000A4, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "EventSelector", ""}, 
};

// A very simplified view of Spyder3
static GEV_REGISTER m_Spyder3_04k80_SG_11_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceManufacturerInfo",        0x000000A8, RO, TRUE,  stringReg, 48, 48,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceReset",                   0x0000D340, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  1,         0x1,        0x1, NULL,    "",        ""}, 
	{ "UserSetSelector",               0x0000D1AC, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   0x0000D1B0, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetSave",                   0x0000D1B4, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetDefaultSelector",        0x0000D1B8, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceTemperature",             0x0000E808, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceSerialNumber",            0x0000E80C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceVoltage",                 0x0000E810, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "ReadVoltageTemperature",        0x0000E814, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""}, 
	{ "SubModelID",                    0x0000E818, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "DeviceScanType",                0x0000D32C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},

	{ "SensorWidth",                   0x0000A020, RW, TRUE,  integerReg, 4,  4,  0,  0,  4096, 1024,4096, 0xffffffff, 0x00000000, NULL,    "",   ""},	

	{ "PixelCoding",                   NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  11,           0,          0, NULL,    "",        ""},
	{ "PixelColorFilter",              NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""}, 	 
	{ "ExposureMode",                  0x0000E81C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "DeviceMaxThroughput",           NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0, 80, 80, 80,  0xffffffff, 0,          NULL,    "",        ""},


	{ "ExposureTime",                  NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeRaw",               NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeMin",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3300,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "AcquisitionLineRate",           NOREF_ADDR, RW, TRUE, integerReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie
	{ "AcquisitionLineRateRaw",        NOREF_ADDR, RW, TRUE, integerReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie
	{ "ExposureAlignment",             NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "CameraScanType",                NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) not in standard	

	{ "SensorDigitizationTaps",        0x0000E81C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "WidthMax",                      0x0000A020, RW, TRUE,  integerReg, 4,  4,  0,  0,  4096,  1024, 4096,  0xffffffff, 0x00000000, NULL,    "",        ""}, // depends on binning !!!
	{ "HeightMax",                     NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  16383, 16383, 16383, 0xffffffff, 0xffffffff, NULL,   "",        ""},	
	{ "Width",                         0x0000D300, RW, TRUE,  integerReg, 4,  4,  0,  0,  4096,  8,  4096,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 // depends on binning !!!
	{ "Height",                        0x0000D304, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  16383,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetX",                       0x0000D31C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  16376,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "BinningHorizontal",             0x0000E82C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ReverseX",                      0x0000E830, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "PixelFormat",                   0x0000D308, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TestImageSelector",             0x0000D33C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "SensitivityMode",               0x0000E90C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "LUTEnable",                     0x0000E834, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "LUTEnabled",                    0x0000E834, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "AcquisitionMode",               0x0000D310, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              0x0000D314, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               0x0000D318, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionFrameCount",         0x0000D334, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,255,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "AcquisitionLineRateAbs",        0x0000E838, RW, TRUE, integerReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie	 
	{ "ExposureTimeAbs",               0x0000E83C, RW, TRUE, integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// formula - then accesses ExposureTimeRaw!!

	{ "Line0Input",                    0x0000E840, RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// formula - then accesses ExposureTimeRaw!!
	{ "Line1Input",                    0x0000E844, RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// formula - then accesses ExposureTimeRaw!!
	{ "Line2Input",                    0x0000E848, RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// formula - then accesses ExposureTimeRaw!!
	{ "Line3Input",                    0x0000E84C, RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// formula - then accesses ExposureTimeRaw!!
	{ "LineSelector",                  NOREF_ADDR, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "",             ""},					
	{ "LineFormat",                    0x0000E840, RW, TRUE, integerReg, 4,  4,  0,  3,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Not in GENIE
	{ "PayloadSize",                   0x0000D30C, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "ReadoutMode",                   0x0000EF80, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "EventSelector",                 NOREF_ADDR, RW, TRUE, integerReg, 4,  4,  0,  0,  1,  1,  8,  0xffffffff, 0xffffffff, NULL,    "",              ""},
};

static GEV_REGISTER m_Spyder3_01k80_SG_11_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceManufacturerInfo",        0x000000A8, RO, TRUE,  stringReg, 48, 48,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceReset",                   0x0000E800, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  1,         0x1,        0x1, NULL,    "",        ""}, 
	{ "UserSetSelector",               0x0000D1AC, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   0x0000E888, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "RestoreUserSetting",            0x0000E888, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetSave",                   0x0000E858, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "WriteUserSetting",              0x0000E858, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetDefaultSelector",        0x0000D1B8, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceSerialNumber",            0x0000E804, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "DeviceTemperature",             0x0000E840, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "CameraTemperature",             0x0000E840, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceVoltage",                 0x0000E848, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "CameraVoltage",                 0x0000E848, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "DeviceScanType ",               0x0000D32C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},

	{ "SensorWidth",                   0x0000A020, RO, TRUE,  integerReg, 4,  4,  0,  0,  1024, 1024,1024, 0xffffffff, 0x00000000, NULL,    "",   ""},	

	{ "ExposureMode",                  0x0000E83C, RW, TRUE,  integerReg, 4,  4,  0,  0,  2,  2,  8,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "DALSAExposureMode",             0x0000E83C, RW, TRUE,  integerReg, 4,  4,  0,  0,  2,  2,  8,  0xffffffff, 0xffffffff, NULL,    "",        ""},	


	{ "ExposureTime",                  0x0000E810, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeAbs",               0x0000E810, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeRaw",               NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeMin",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3300,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "AcquisitionLineRate",           0x0000E80C, RW, TRUE, integerReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie
	{ "AcquisitionLineRateAbs",        0x0000E80C, RW, TRUE, integerReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie

	{ "SensorDigitizationTaps",        0x0000D324, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "WidthMax",                      0x0000A020, RO, TRUE,  integerReg, 4,  4,  0,  0,  1024,  1024, 1024,  0xffffffff, 0x00000000, NULL,    "",        ""}, // depends on binning !!!
	{ "HeightMax",                     NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  16383, 16383, 16383, 0xffffffff, 0xffffffff, NULL,   "",        ""},	
	{ "Width",                         0x0000A010, RW, TRUE,  integerReg, 4,  4,  0,  0,  1024,  8,  1024,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 // depends on binning !!!
	{ "Height",                        0x0000A018, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  16383,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "BinningHorizontal",             0x0000E808, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "DecimationHorizontal",          NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  1,  1,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "DecimationVertical",            NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  1,  1,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "PixelFormat",                   0x0000D308, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TestImageSelector",             0x0000D33C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "SensitivityMode",               0x0000E82C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "LUTEnable",                     0x0000E834, RW, TRUE,   integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},
	{ "LUTEnabled",                    0x0000E834, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LUTSelector", ""},

	{ "AcquisitionMode",               0x0000D310, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              0x0000D314, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               0x0000D318, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionFrameCount",         0x0000D334, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,255,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "PayloadSize",                   0x0000D30C, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "ReadoutMode",                   0x0000E894, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "EventSelector",                 NOREF_ADDR, RW, TRUE, integerReg, 4,  4,  0,  0,  1,  1,  8,  0xffffffff, 0xffffffff, NULL,    "",              ""},

	{ "GPIOSelector",                  NOREF_ADDR, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  1,  3,  0xffffffff, 0xffffffff, NULL,    "",              ""},
	{ "GPIOInput",                     0x0000E8BC, RW, TRUE, integerReg, 4,  4,  0,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "GPIOSelector", ""},	
	{ "GPIOInputPort0",                0x0000E8BC, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GPIOInputPort1",                0x0000E8C0, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GPIOInputPort2",                0x0000E8C4, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GPIOInputPort3",                0x0000E8C8, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GPIOOutput",                    0x0000E8CC, RW, TRUE, integerReg, 4,  4,  0,  3,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "GPIOSelector", ""},	
	{ "GPIOOutputPort0",               0x0000E8CC, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GPIOOutputPort1",               0x0000E8D0, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GPIOOutputPort2",               0x0000E8D4, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GPIOOutputPort3",               0x0000E8D8, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
};

static GEV_REGISTER m_Spyder3_04k80_SG_14_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceManufacturerInfo",        0x000000A8, RO, TRUE,  stringReg, 48, 48,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceReset",                   0x0000D340, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  1,         0x1,        0x1, NULL,    "",        ""}, 
	{ "UserSetSelector",               0x0000D1AC, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   0x0000D1B0, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetSave",                   0x0000D1B4, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetDefaultSelector",        0x0000D1B8, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceSerialNumber",            0x0000E80C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceFirmwareVersion",         0x0000E820, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "DeviceTemperature",             0x0000E800, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "CameraTemperature",             0x0000E800, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceVoltage",                 0x0000E810, RO, TRUE,    floatReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "CameraVoltage",                 0x0000E810, RO, TRUE,    floatReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "ReadVoltageTemperature",        0x0000E814, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "SubModelID",                    0x0000E818, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "SensorWidth",                   0x0000E918, RO, TRUE,  integerReg, 4,  4,  0,  0,  1024, 1024,4096, 0xffffffff, 0x00000000, NULL,    "",   ""},	
	{ "CameraSensorWidth",             0x0000E918, RO, TRUE,  integerReg, 4,  4,  0,  0,  1024, 1024,4096, 0xffffffff, 0x00000000, NULL,    "",   ""},	
	{ "ExposureMode",                  0x0000E860, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "DALSAExposureMode",             0x0000E860, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "ExposureTime",                  0x0000E878, RW, TRUE,    floatReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeAbs",               0x0000E878, RW, TRUE,    floatReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeRaw",               NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeMin",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3300,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "AcquisitionLineRate",           0x0000E874, RW, TRUE,    floatReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie
	{ "AcquisitionLineRateAbs",        0x0000E874, RW, TRUE,    floatReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie

	{ "SensorTaps",                    0x0000E864, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "SensorDigitizationTaps",        0x0000D324, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "WidthMax",                      NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  1024,  1024, 4096,  0xffffffff, 0x00000000, NULL,    "",        ""}, // depends on binning !!!
	{ "HeightMax",                     NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  16383, 16383, 16383, 0xffffffff, 0xffffffff, NULL,   "",        ""},	
	{ "Width",                         0x0000D300, RW, TRUE,  integerReg, 4,  4,  0,  0, 4096, 8, 4096,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 // depends on binning !!!
	{ "Height",                        0x0000D304, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  16383,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetX",                       0x0000D31C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  16376,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "BinningHorizontal",             0x0000E868, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ReverseX",                      0x0000E86C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "PixelFormat",                   0x0000D308, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TestImageSelector",             0x0000D33C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "SensitivityMode",               0x0000E998, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "LUTReset",                      0x0000E980, WO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "", ""},
	{ "LUTEnable",                     0x0000E98C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "", ""},
	{ "LUTEnabled",                    0x0000E98C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "", ""},

	{ "AcquisitionMode",               0x0000D310, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              0x0000D314, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               0x0000D318, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionFrameCount",         0x0000D334, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,255,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "PayloadSize",                   0x0000D30C, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "ReadoutMode",                   0x0000E940, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "EventSelector",                 NOREF_ADDR, RW, TRUE, integerReg, 4,  4,  0,  0,  1,  1,  8,  0xffffffff, 0xffffffff, NULL,    "",              ""},

	{ "Line0Input",                    0x0000E87C, RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "Line1Input",                    0x0000E880, RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "Line2Input",                    0x0000E884, RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "Line3Input",                    0x0000E888, RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "LineSelector",                  NOREF_ADDR, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "",             ""},					
	{ "LineFormat",                    0x0000E87C, RW, TRUE, integerReg, 4,  4,  0,  3,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	
	{ "OutputSelector",                NOREF_ADDR, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "",             ""},					
	{ "OutputFormat",                  0x0000E924, RW, TRUE, integerReg, 4,  4,  0,  3,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "OutputSelector", ""},	

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "UpdateGainReference",			  0x0000E93C, WO, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "CalibrateBlackLevel",			  0x0000E934, WO, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "CalibrateBlackLevelTarget",     0x0000E938, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  1,  255,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "CalibrateGain",                 0x0000E948, WO, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "CalibrateGainSelector",         0x0000E94C, RW, TRUE, integerReg, 4,  4,  0,  0,  1,  4,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "CalibrateGainTarget",           0x0000E944, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  1024, 4095,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "CalibrateResult",               0x0000E950, RO, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "ReadCalibrateResult",           0x0000E954, WO, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GainSelector",                  NOREF_ADDR, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "GainAbs",						     0x0000E88C, RW, TRUE, integerReg, 4,  4,  0,  2,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	
	{ "DigitalGainSelector",           NOREF_ADDR, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "DigitalGainRaw",				     0x0000E8A4, RW, TRUE, integerReg, 4,  4,  0,  2,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "DigitalGainSelector", ""},	
	{ "DigitalOffsetSelector",         NOREF_ADDR, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "DigitalOffsetRaw",				  0x0000E8B0, RW, TRUE, integerReg, 4,  4,  0,  2,  0,  0,  2048,  0xffffffff, 0xffffffff, NULL,    "DigitalOffsetSelector", ""},	
	{ "BlackLeveSelector",             NOREF_ADDR, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "BlackLevelRaw",				     0x0000E898, RW, TRUE, integerReg, 4,  4,  0,  2,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	
	{ "BackgroundSubtractSelector",    NOREF_ADDR, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "BackgroundSubtractRaw",			  0x0000E8BC, RW, TRUE, integerReg, 4,  4,  0,  2,  0,  0,  4095,  0xffffffff, 0xffffffff, NULL,    "BackgroundSubtractSelector", ""},	


/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "DeviceScanType",                0x0000D32C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "TriggerConfiguration",          0x0000B81C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GrbCh0TrigCfg",                 0x0000B81C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/*	Frame Trigger configuration is a number of bits.									*/	
/*		D17    = Automatic retrigger															*/
/*		D15:D4 = FramesToSkip (0 to 4095)													*/
/*		D3     = FrameTriggerEnable (1 / 0) - use PLC config to trigger grabber.*/
/*		D2:D1  = FramesInPipe (0 to 2) - 0 is continuous								*/
/*    Set to 0x8 for trigger enable, no frames skipped								*/

	{ "LineTriggerMode", 0x0000E82C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Line trigger is 0 for OFF, 1 for ON 	*/ 


/*    |"FeatureName"        |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                     |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "PLC_I0toI7",           0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I0",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I1",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I2",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I3",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I4",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I5",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I6",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I7",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/* PLC Routing block (above) - 8 Groups of 4 bits , (but I don't do bit ranges yet - so do the whole thing at once or do Rd/Mod/Wr	*/
/* A set of definitions is elsewhere */
/* PLC_I1 is the Frame Trigger control */

/*    |"FeatureName" |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "Q0",            0x0000A100, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q1",            0x0000A104, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q2",            0x0000A108, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q3",            0x0000A10C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q4",            0x0000A110, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q5",            0x0000A114, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q6",            0x0000A118, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q7",            0x0000A11C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q8",            0x0000A120, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q9",            0x0000A124, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q10",           0x0000A128, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q11",           0x0000A12C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q12",           0x0000A130, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q13",           0x0000A134, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q14",           0x0000A138, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q15",           0x0000A13C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q16",           0x0000A140, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q17",           0x0000A144, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/* PLC LUT (above) - bit masks represent the expression {Variable0 : Operator0 : Variable1 : Operator1 : Variable2 : Operator2 : Variable3 } */
/*						 - for each of 18 channels that can be programmed 	*/
/* (Valid definitions elsewhere) */
/* Q12 Variable0 (along with PLC_I1) controls edge triggered (fixed length) versus level triggered (variable length) frame acquisition */


/*    |"FeatureName"                    |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                                 |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "FrameValidConfiguration",          0x0000B860, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PixelBusFrameValidFunctionSelect", 0x0000B860, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/* Frame valid source configuration (above) - part of the programmable I/O architecture */
/*	D18:D16  (0 = Internal, 1 = Internal AND PLC_Q12, 2 = Internal OR PLC_Q12, 3 = PLC_Q12, 4 = Internal AND Spare, 5 = Internal OR Spare) */

/*          |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*          |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GrbCh0AcqCfgIncludeMetadataInImage",  0x0000B818, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Metadata in image is 0 for OFF, 1 for ON 	*/ 

	{ "GrabberMetadata",                     0x0000B9A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GrbCh0MetadataInsertionMode",         0x0000B9A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GrbCh0MetadataSourceSelection",       0x0000B9A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GrbCh0MetadataMsbOverride",           0x0000B9A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* A set of bits to control the metadata selection                                 */
/* For "BlockID" aka line # inserted into all lines - set this to 0x81000000       */
/* For "BlockID" aka line # inserted into first line only - set this to 0x01000000 */
/* For metadata insertion control - above */

/*    |"FeatureName"                   |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                                |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GevIPConfigurationStatus",        0x0000A030, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 4,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevCurrentIPConfiguration",       0x00000014, RW, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
   { "GevCurrentIPConfigurationLLA",    0x00000014, RW, TRUE,  bitReg,      4,  4,  0,  0,  0,29,29,  0xffffffff, 0xffffffff, NULL,    "",        ""},		
	{ "GevCurrentIPConfigurationDHCP",   0x00000014, RW, TRUE,  bitReg,      4,  4,  0,  0,  0,30,30,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "GevCurrentIPConfigurationPersistentIP", 0x00000014, RW, TRUE,bitReg,  4,  4,  0,  0,  0,31,31,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Current IP Configuration : 0=None, 1=PersistentIP, 2=DHCP, 3=LLA, 4=ForceIP */

/*    |"FeatureName"                   |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                                |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GevTimestampCounterSelector",     0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 14, 14,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Timestamp counter selector - mask is 0x00020000 (0 for GevTimestamp, 0x00020000 for Counter1) */
	{ "GevTimestampSetSource",           0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 23, 25,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Source signal for "Timestamp Set" cmd - mask is 0x000001C0 (0-7 for Q3,Q7,Q8,Q9,Q10,Q11,Q16,Q17 */
	{ "GevTimestampSetActivation",       0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 27, 28,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Enable/disable "Timestamp Set" from PLC signal - mask is 0x00000018 (0 for disabled, 0x00000010 for enabled, 0x00000008 to do a "Set" */
	{ "GevTimestampResetSource",         0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 16, 18,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Source signal for "Timestamp Reset" cmd - mask is 0x0000E000 (0-7 for Q3,Q7,Q8,Q9,Q10,Q11,Q16,Q17 */
	{ "GevTimestampResetActivation",     0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 20, 21,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Enable/disable "Timestamp Reset" from PLC signal - mask is 0x00000C00 (0 for disabled, 0x00000080 for enabled, 0x00000040 to do a "Reset" */

	{ "GevTimestampValueAtSet",          0x0000B8E0, RW, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

	{ "GevTimestampTickFrequencyHigh",   0x0000093C, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevTimestampTickFrequencyLow",    0x00000940, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

	{ "GevTimestampValueHigh",           0x00000948, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevTimestampValueLow",            0x0000094C, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevTimestampControl",             0x00000944, WO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Write a 2 to TimestampControl to latch the counter into the Value regs. Write a 1 to reset it to 0 */

/*    |"FeatureName"              |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                           |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "SensensitivityMode",         0x0000E998, RW,  TRUE, integerReg, 4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "SensorScanDirection",        0x0000E994, RW,  TRUE, integerReg, 4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "ReadSensorScanDirection",    0x0000E85C, WO,  TRUE, integerReg, 4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "SensorScanExternalDirection",0x0000E858, RO,  TRUE, integerReg, 4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},


/*	Calibration related features.....                                                                                                       */
/*    |"FeatureName"     |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                  |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "PixelSetReset",     0x0000E958,  WO, TRUE, integerReg, 4,  4,  0,  4,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PixelSetSelector",  NOREF_ADDR,  RW, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  4,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PixelSetLoad",      0x0000E8C8,  WO, TRUE, integerReg, 4,  4,  0,  4,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "PixelSetSelector", ""},
   { "PixelSetPRNUSave",  0x0000E8DC,  WO, TRUE, integerReg, 4,  4,  1,  4,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "PixelSetSelector", ""},
	{ "PixelSetFPNSave",   0x0000E8EC,  WO, TRUE, integerReg, 4,  4,  1,  4,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "PixelSetSelector", ""},

	{ "FPNCalibrate",      0x0000E95C,  WO, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PRNUCalibrate",     0x0000E960,  WO, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "FPNEnable",         0x0000E970,  RW, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PRNUEnable",        0x0000E96C,  RW, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},

	{ "CorrectionSamples", 0x0000E984,  RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  2,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "RegionOfInterestX", 0x0000E97C,  RW, TRUE, integerReg, 4,  4,  0,  0,  1, 1, 4095, 0xffffffff,  0xffffffff, NULL,    "",        ""},

/*    |"FeatureName"                    |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                                 |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "RegionOfInterestWidth",            0x0000E980,  RW, TRUE, integerReg, 4,  4,  0,  0,  1,  1, 4095, 0xffffffff, 0xffffffff, NULL,    "",        ""},

	{ "PRNUCalibrationTarget",            0x0000E964,  RW, TRUE, integerReg, 4,  4,  0,  0,  1024, 1024,  4095,   0xffffffff,  0xffffffff, NULL,    "",        ""},
   { "PRNUCalibrationAlgorithmSelector", 0x0000E968,  RW, TRUE, integerReg, 4,  4,  0,  0,  2, 2,  2,    0xffffffff,  0xffffffff, NULL,    "",        ""},

	{ "ReadFFCCalibrationResult",         0x0000E988,  WO, TRUE, integerReg, 4,  4,  0,  0,  1, 0, 1,     0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "FFCCalibrationResult",             0x0000E974,  RO, TRUE, integerReg, 4,  4,  0,  0,  0, 0, 65535, 0xffffffff,  0xffffffff, NULL,    "",        ""},
};


static GEV_REGISTER m_Spyder3_04k80_SG_32_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceManufacturerInfo",        0x000000A8, RO, TRUE,  stringReg, 48, 48,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceReset",                   0x0000D340, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  1,         0x1,        0x1, NULL,    "",        ""}, 
	{ "UserSetSelector",               0x0000D1AC, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   0x0000D1B0, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetSave",                   0x0000D1B4, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetDefaultSelector",        0x0000D1B8, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceTemperature",             0x0000E808, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceSerialNumber",            0x0000E80C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceVoltage",                 0x0000E814, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "deviceInputVoltage",            0x0000E814, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "ReadVoltageTemperature",        0x0000E810, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""}, 
	{ "SubModelID",                    0x0000E818, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "DeviceScanType",                0x0000D32C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},

	{ "SensorWidth",                   0x0000E908, RO, TRUE,  integerReg, 4,  4,  0,  0,  4096, 2048,4096, 0xffffffff, 0x00000000, NULL,    "",   ""},	
	{ "CameraSensorWidth",             0x0000E908, RO, TRUE,  integerReg, 4,  4,  0,  0,  4096, 2048,4096, 0xffffffff, 0x00000000, NULL,    "",   ""},	

	{ "PixelCoding",                   NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  11,           0,          0, NULL,    "",        ""},
	{ "PixelColorFilter",              NOREF_ADDR, RO, TRUE,  fixedVal,   4,  4,  0,  0,  0,  0,  0,           0,          0, NULL,    "",        ""}, 	 
	{ "ExposureMode",                  0x0000E820, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "DeviceMaxThroughput",           NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0, 80, 80, 80,  0xffffffff, 0,          NULL,    "",        ""},


	{ "ExposureTime",                  NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeRaw",               NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeMin",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3300,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "AcquisitionLineRate",           NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie
	{ "AcquisitionLineRateRaw",        NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie
	{ "ExposureAlignment",             NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "CameraScanType",                NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) not in standard	

	{ "SensorDigitizationTaps",        0x0000D324, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "WidthMax",                      0x0000E908, RO, TRUE,  integerReg, 4,  4,  0,  0,  4096,  1024, 4096,  0xffffffff, 0x00000000, NULL,    "",        ""}, // depends on binning !!!
	{ "HeightMax",                     NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  16383, 16383, 16383, 0xffffffff, 0xffffffff, NULL,   "",        ""},	
	{ "Width",                         0x0000D300, RW, TRUE,  integerReg, 4,  4,  0,  0,  4096,  8,  4096,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 // depends on binning !!!
	{ "Height",                        0x0000D304, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  16383,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetX",                       0x0000D31C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  16376,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "ReverseX",                      0x0000E844, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "PixelFormat",                   0x0000D308, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TestImageSelector",             0x0000D33C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "AcquisitionMode",               0x0000D310, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              0x0000D314, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               0x0000D318, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionFrameCount",         0x0000D334, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,255,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "AcquisitionLineRateAbs",        0x0000E81C, RW, TRUE, integerReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie	 
	{ "ExposureTimeAbs",               0x0000E824, RW, TRUE, integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// formula - then accesses ExposureTimeRaw!!

	{ "LineSelector",                  0x0000E854, RW, TRUE, intVal,     4,  4,  0,  0,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "",             ""},					
	{ "LineFormat",                    0x0000E858, RW, TRUE, integerReg, 4,  4,  0,  3,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	// Not in GENIE

	{ "PayloadSize",                   0x0000D30C, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "ReadoutMode",                   0x0000E83C, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "EventSelector",                 NOREF_ADDR, RW, TRUE, integerReg, 4,  4,  0,  0,  1,  1,  8,  0xffffffff, 0xffffffff, NULL,    "",              ""},
};

static GEV_REGISTER m_Spyder3_04k80_SG_34_RegList[] = {
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
 	{ "DeviceVendorName",              0x00000048, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",               0x00000068, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceManufacturerInfo",        0x000000A8, RO, TRUE,  stringReg, 48, 48,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",                 0x00000088, RO, TRUE,  stringReg, 32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceID",                      0x000000D8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",                  0x000000E8, RO, TRUE,  stringReg, 16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceReset",                   0x0000D340, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  1,         0x1,        0x1, NULL,    "",        ""}, 
	{ "UserSetSelector",               0x0000D1AC, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",                   0x0000D1B0, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetSave",                   0x0000D1B4, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "UserSetSelector", ""}, 
	{ "UserSetDefaultSelector",        0x0000D1B8, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceSerialNumber",            0x0000E80C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceFirmwareVersion",         0x0000E81C, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "DeviceTemperature",             0x0000E800, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "CameraTemperature",             0x0000E800, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "DeviceVoltage",                 0x0000E810, RO, TRUE,    floatReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "CameraVoltage",                 0x0000E810, RO, TRUE,    floatReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "ReadVoltageTemperature",        0x0000E814, WO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "SubModelID",                    0x0000E818, RO, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "SensorWidth",                   0x0000E8EC, RO, TRUE,  integerReg, 4,  4,  0,  0,  2048, 2048,4096, 0xffffffff, 0x00000000, NULL,    "",   ""},	
	{ "CameraSensorWidth",             0x0000E8EC, RO, TRUE,  integerReg, 4,  4,  0,  0,  2048, 2048,4096, 0xffffffff, 0x00000000, NULL,    "",   ""},	
	{ "ExposureMode",                  0x0000E880, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "DALSAExposureMode",             0x0000E880, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "ExposureTime",                  0x0000E884, RW, TRUE,    floatReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeAbs",               0x0000E884, RW, TRUE,    floatReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeRaw",               NOREF_ADDR, RW, TRUE,  integerReg, 4,  4,  0,  0,  3300,  3,  3300,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "ExposureTimeMin",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "ExposureTimeMax",               NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  3300,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard 
	{ "AcquisitionLineRate",           0x0000E87C, RW, TRUE,    floatReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie
	{ "AcquisitionLineRateAbs",        0x0000E87C, RW, TRUE,    floatReg, 4,  4,  0,  0,  68000,  300,  68000,  0xffffffff, 0xffffffff, NULL,    "",        ""},  // (**)not in Genie

	{ "SensorTaps",                    0x0000D324, RO, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "SensorDigitizationTaps",        0x0000D324, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "WidthMax",                      NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  2048,  2048, 4096,  0xffffffff, 0x00000000, NULL,    "",        ""}, // depends on binning !!!
	{ "HeightMax",                     NOREF_ADDR, RO, TRUE,  integerReg, 4,  4,  0,  0,  16383, 16383, 16383, 0xffffffff, 0xffffffff, NULL,   "",        ""},	
	{ "Width",                         0x0000D300, RW, TRUE,  integerReg, 4,  4,  0,  0, 4096, 8, 4096,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 // depends on binning !!!
	{ "Height",                        0x0000D304, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,  16383,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetX",                       0x0000D31C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  16376,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ReverseX",                      0x0000E890, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "PixelFormat",                   0x0000D308, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TestImageSelector",             0x0000D33C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "AcquisitionMode",               0x0000D310, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",              0x0000D314, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",               0x0000D318, WO, TRUE,  bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionFrameCount",         0x0000D334, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  1,255,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "PayloadSize",                   0x0000D30C, RO, FALSE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 

	{ "ReadoutMode",                   0x0000E88C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "", ""},	
	{ "EventSelector",                 NOREF_ADDR, RW, TRUE, integerReg, 4,  4,  0,  0,  1,  1,  8,  0xffffffff, 0xffffffff, NULL,    "",              ""},

	{ "LineSelector",                  0x0000E840, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "",             ""},					
	{ "LineFormat",                    0x0000E844, RW, TRUE, integerReg, 4,  4,  0,  3,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "LineSelector", ""},	
	{ "OutputSelector",                0x0000E848, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "",             ""},					
	{ "OutputFormat",                  0x0000E84C, RW, TRUE, integerReg, 4,  4,  0,  3,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "OutputSelector", ""},	

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "DigitalGainRaw",				     0x0000E8F8, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "DigitalGainSelector", ""},	
	{ "BackgroundSubtractRaw",			  0x0000E8FC, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  4095,  0xffffffff, 0xffffffff, NULL,    "BackgroundSubtractSelector", ""},	
	{ "ColorCorrectionValueRaw",		  0x0000E904, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0, 65535,  0xffffffff, 0xffffffff, NULL,    "BackgroundSubtractSelector", ""},	
	{ "ColorCorrectionInputChannel",	  0x0000E908, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  3,     0xffffffff, 0xffffffff, NULL,    "BackgroundSubtractSelector", ""},	
	{ "ColorCorrectionOutputChannel",  0x0000E90C, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,     0xffffffff, 0xffffffff, NULL,    "BackgroundSubtractSelector", ""},	
	{ "DigitalGainMinRaw",				  0x0000E910, RO, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "DigitalGainSelector", ""},	
	{ "DigitalGainMaxRaw",				  0x0000E914, RO, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "DigitalGainSelector", ""},	

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "LightSource",                   0x0000E918, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  4,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "ColorTapSelector",              0x0000E91C, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  2,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "ColorSelector",                 0x0000E920, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0,  3,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "ColorGainAbs",                  0x0000E924, RW, TRUE,   floatReg, 4,  4,  0,  0,  0,  0,  3,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "ColorGainReferenceUpdate",      0x0000E928, WO, TRUE, integerReg, 4,  4,  0,  0,  1,  1,  1,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "CalibrateWhiteBalance",         0x0000E92C, WO, TRUE, integerReg, 4,  4,  0,  0,  1,  1,  1,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "CalibrateWhiteBalanceTarget",   0x0000E8AC, RW, TRUE, integerReg, 4,  4,  0,  0,  0,  0, 4095,0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "ColorGainReferenceRaw",         0x0000E930, RO, TRUE, integerReg, 4,  4,  0,  0,  1,  1,  1,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "ColorGainMinAbs",               0x0000E934, RO, TRUE,	  floatReg, 4,  4,  0,  0,  1,  1,  1,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "ColorGainMaxAbs",               0x0000E938, RO, TRUE,	  floatReg, 4,  4,  0,  0,  1,  1,  1,  0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "CalibrateWhiteBalanceResult",   0x0000E93C, RO, TRUE, integerReg, 4,  4,  0,  0,  0,  0, 4095,0xffffffff,  0xffffffff, NULL,    "",      ""},	
	{ "ReadCalibrateWhiteBalanceResult",0x0000E940,WO, TRUE, integerReg, 4,  4,  0,  0,  0,  0, 4095,0xffffffff,  0xffffffff, NULL,    "",      ""},	

/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "DeviceScanType",                0x0000D32C, RW, TRUE,  integerReg, 4,  4,  0,  0,  1,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "TriggerConfiguration",          0x0000B81C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GrbCh0TrigCfg",                 0x0000B81C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/*	Frame Trigger configuration is a number of bits.									*/	
/*		D17    = Automatic retrigger															*/
/*		D15:D4 = FramesToSkip (0 to 4095)													*/
/*		D3     = FrameTriggerEnable (1 / 0) - use PLC config to trigger grabber.*/
/*		D2:D1  = FramesInPipe (0 to 2) - 0 is continuous								*/
/*    Set to 0x8 for trigger enable, no frames skipped								*/

	{ "LineTriggerMode", 0x0000E834, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Line trigger is 0 for OFF, 1 for ON 	*/ 


/*    |"FeatureName"        |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                     |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "PLC_I0toI7",           0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I0",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I1",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I2",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I3",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I4",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I5",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I6",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PLC_I7",               0x0000BB48, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/* PLC Routing block (above) - 8 Groups of 4 bits , (but I don't do bit ranges yet - so do the whole thing at once or do Rd/Mod/Wr	*/
/* A set of definitions is elsewhere */
/* PLC_I1 is the Frame Trigger control */

/*    |"FeatureName" |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "Q0",            0x0000A100, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q1",            0x0000A104, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q2",            0x0000A108, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q3",            0x0000A10C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q4",            0x0000A110, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q5",            0x0000A114, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q6",            0x0000A118, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q7",            0x0000A11C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q8",            0x0000A120, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q9",            0x0000A124, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q10",           0x0000A128, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q11",           0x0000A12C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q12",           0x0000A130, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q13",           0x0000A134, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q14",           0x0000A138, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q15",           0x0000A13C, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q16",           0x0000A140, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "Q17",           0x0000A144, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/* PLC LUT (above) - bit masks represent the expression {Variable0 : Operator0 : Variable1 : Operator1 : Variable2 : Operator2 : Variable3 } */
/*						 - for each of 18 channels that can be programmed 	*/
/* (Valid definitions elsewhere) */
/* Q12 Variable0 (along with PLC_I1) controls edge triggered (fixed length) versus level triggered (variable length) frame acquisition */


/*    |"FeatureName"                    |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                                 |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "FrameValidConfiguration",          0x0000B860, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "PixelBusFrameValidFunctionSelect", 0x0000B860, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

/* Frame valid source configuration (above) - part of the programmable I/O architecture */
/*	D18:D16  (0 = Internal, 1 = Internal AND PLC_Q12, 2 = Internal OR PLC_Q12, 3 = PLC_Q12, 4 = Internal AND Spare, 5 = Internal OR Spare) */

/*          |"FeatureName"                 |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*          |                              |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GrbCh0AcqCfgIncludeMetadataInImage",  0x0000B818, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Metadata in image is 0 for OFF, 1 for ON 	*/ 

	{ "GrabberMetadata",                     0x0000B9A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GrbCh0MetadataInsertionMode",         0x0000B9A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GrbCh0MetadataSourceSelection",       0x0000B9A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GrbCh0MetadataMsbOverride",           0x0000B9A0, RW, TRUE,  integerReg, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* A set of bits to control the metadata selection                                 */
/* For "BlockID" aka line # inserted into all lines - set this to 0x81000000       */
/* For "BlockID" aka line # inserted into first line only - set this to 0x01000000 */
/* For metadata insertion control - above */

/*    |"FeatureName"                   |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                                |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GevIPConfigurationStatus",        0x0000A030, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 4,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevCurrentIPConfiguration",       0x00000014, RW, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
   { "GevCurrentIPConfigurationLLA",    0x00000014, RW, TRUE,  bitReg,      4,  4,  0,  0,  0,29,29,  0xffffffff, 0xffffffff, NULL,    "",        ""},		
	{ "GevCurrentIPConfigurationDHCP",   0x00000014, RW, TRUE,  bitReg,      4,  4,  0,  0,  0,30,30,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "GevCurrentIPConfigurationPersistentIP", 0x00000014, RW, TRUE,bitReg,  4,  4,  0,  0,  0,31,31,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Current IP Configuration : 0=None, 1=PersistentIP, 2=DHCP, 3=LLA, 4=ForceIP */

/*    |"FeatureName"                   |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                                |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "GevTimestampCounterSelector",     0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 14, 14,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Timestamp counter selector - mask is 0x00020000 (0 for GevTimestamp, 0x00020000 for Counter1) */
	{ "GevTimestampSetSource",           0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 23, 25,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Source signal for "Timestamp Set" cmd - mask is 0x000001C0 (0-7 for Q3,Q7,Q8,Q9,Q10,Q11,Q16,Q17 */
	{ "GevTimestampSetActivation",       0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 27, 28,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Enable/disable "Timestamp Set" from PLC signal - mask is 0x00000018 (0 for disabled, 0x00000010 for enabled, 0x00000008 to do a "Set" */
	{ "GevTimestampResetSource",         0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 16, 18,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Source signal for "Timestamp Reset" cmd - mask is 0x0000E000 (0-7 for Q3,Q7,Q8,Q9,Q10,Q11,Q16,Q17 */
	{ "GevTimestampResetActivation",     0x0000B8DC, RW, TRUE,  bitReg,      4,  4,  0,  0,  0, 20, 21,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Enable/disable "Timestamp Reset" from PLC signal - mask is 0x00000C00 (0 for disabled, 0x00000080 for enabled, 0x00000040 to do a "Reset" */

	{ "GevTimestampValueAtSet",          0x0000B8E0, RW, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

	{ "GevTimestampTickFrequencyHigh",   0x0000093C, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevTimestampTickFrequencyLow",    0x00000940, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},

	{ "GevTimestampValueHigh",           0x00000948, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevTimestampValueLow",            0x0000094C, RO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "GevTimestampControl",             0x00000944, WO, TRUE,  integerReg,  4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
/* Write a 2 to TimestampControl to latch the counter into the Value regs. Write a 1 to reset it to 0 */

/*    |"FeatureName"              |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                           |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "SensorScanDirection",        0x0000E850, RW,  TRUE, integerReg, 4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "ReadSensorScanDirection",    0x0000E858, WO,  TRUE, integerReg, 4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "SensorScanExternalDirection",0x0000E854, RO,  TRUE, integerReg, 4,  4,  0,  0,  0, 0, 0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},


/*	Calibration related features.....                                                                                                       */
/*    |"FeatureName"     |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                  |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */

	{ "PixelSetReset",     0x0000E8BC,  WO, TRUE, integerReg, 4,  4,  0,  4,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PixelSetSelector",  0x0000E898,  RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  8,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PixelSetLoad",      0x0000E89C,  WO, TRUE, integerReg, 4,  4,  0,  4,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "PixelSetSelector", ""},
   { "PixelSetPRNUSave",  0x0000E8A0,  WO, TRUE, integerReg, 4,  4,  1,  4,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "PixelSetSelector", ""},
	{ "PixelSetFPNSave",   0x0000E8A4,  WO, TRUE, integerReg, 4,  4,  1,  4,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "PixelSetSelector", ""},

	{ "FPNCalibrate",      0x0000E8A8,  WO, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PRNUCalibrate",     0x0000E8B0,  WO, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "FPNEnable",         0x0000E8B4,  RW, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PRNUEnable",        0x0000E8B8,  RW, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  1,   0xffffffff,  0xffffffff, NULL,    "",        ""},

	{ "CorrectionSamples", 0x0000E8E8,  RW, TRUE, integerReg, 4,  4,  0,  0,  0, 0,  2,   0xffffffff,  0xffffffff, NULL,    "",        ""},

/*    |"FeatureName"                    |Addres     |Acc|Avail |  Type    |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector | Index  */
/*    |                                 |           |   |      |          |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string   | string */
	{ "CalibrateWhiteBalanceTarget",      0x0000E8AC,  RW, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  4095,   0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "PRNUCalibrationTarget",            0x0000E8AC,  RW, TRUE, integerReg, 4,  4,  0,  0,  1, 0,  4095,   0xffffffff,  0xffffffff, NULL,    "",        ""},

	{ "ReadFFCCalibrationResult",         0x0000E8C4,  WO, TRUE, integerReg, 4,  4,  0,  0,  1, 0, 1,     0xffffffff,  0xffffffff, NULL,    "",        ""},
	{ "FFCCalibrationResult",             0x0000E8C0,  RO, TRUE, integerReg, 4,  4,  0,  0,  0, 0, 65535, 0xffffffff,  0xffffffff, NULL,    "",        ""},
};

static GEV_REGISTER m_DPIGC3_0_RegList[] = {
/*    |"FeatureName"           |Address    |Acc|Avail|  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                        |           |   |     |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "DeviceVendorName",        0x00000048, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",         0x00000068, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",           0x00000088, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",   0x180000E0, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceProductIDBuild",    0x180000E0, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                0x000000D8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "HardwareRevision",        0x20000240, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "DeviceUserID",            0x000000E8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceScanType",          NOREF_ADDR, RO, TRUE, intVal,       4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "deviceAcquisitionType",   NOREF_ADDR, RO, TRUE, intVal,       4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceReset",             0x18000010, WO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "DeviceTemperature",       0x200001B0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "DeviceRegistersValid",    NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0,  1,  0,  1,  0x1,        0,          NULL,    "",        ""},
	{ "SensorWidth",             NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0, 1316, 0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",            NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0, 1318, 0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0, 1316, 0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "HeightMax",               NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0, 1318, 0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""}, 
	{ "Width",                   0x20000070, RO, TRUE, integerRegLE, 4,  4,  0,  0, 1316, 2,1548,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                  0x20000090, RO, TRUE, integerRegLE, 4,  4,  0,  0, 1318, 2,1548,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "OffsetX",                 NOREF_ADDR, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetY",                 NOREF_ADDR, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "BinningHorizontal",       NOREF_ADDR, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningVertical",         NOREF_ADDR, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "PixelSize",               NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0, 14,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},     

	{ "PixelFormat",             0x20000060, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   { "PixelCoding",             0x20000130, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "ROIStartV",               0x200002B0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,1516,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ROIStopV",                0x200002D0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0, 31,1547,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ROIStartH",               0x200002A0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,1516,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "ROIStopH",                0x200002C0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0, 31,1547,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
 	{ "SettingsRevision",        0x200001C0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "ExtendedExposure",        0x20000280, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0x3E7FFFF,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "ReadOutMode",             0x20000290, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0x00000000, NULL,    "",        ""},	
   { "FullWell",                0x20000260, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   { "Offset",                  0x20000250, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   { "Standby",                 0x200002E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TestImageSelector",       0x200000E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "ImageSourceSelector",     0x200000E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "SensorDigitalTestPatternValue", 0x20000310, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  31,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "TriggerMode",             0x20000270, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0x00000000, NULL,    "",        ""},	
   
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */

	{ "AcquisitionMode",          0x20000040, RW, TRUE, integerReg, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",         0x20000000, WO, TRUE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",          0x20000010, WO, TRUE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",         0x20000020, WO, TRUE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionArm",           0x20000030, WO, TRUE, bitReg,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},
	{ "AcquisitionFrameCount",    0x20000050, RW, TRUE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameCountMax", 0x20000058, RO, TRUE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameCountMin", 0x20000054, RO, TRUE, integerReg, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},

	{ "EventSelector",            0x100000A0, RW, TRUE,  integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",              ""}, 
	{ "EventNotification",        0x100000A4, RW, TRUE,  integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "EventSelector", ""}, 

	{ "UserSetDefaultSelector",   0x08000000, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",          NOREF_ADDR, RW, TRUE, intVal,       4,  4,  0,  0,  6,  4,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",              0x08000020, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  6,         0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetSave",              0x08000010, RW, TRUE, integerRegLE, 4,  4,  0,  0,  4,  4,  5,         0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetError",             0x08000050, RO, TRUE, integerRegLE, 4,  4,  0,  0,  4,  4,  5,         0x1,        0x1, NULL,    "UserSetSelector"}, 
};


static GEV_REGISTER m_GenieTS_RegList[] = {
/*    |"FeatureName"           |Address    |Acc|Avail|  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                        |           |   |     |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "DeviceVendorName",        0x00000048, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",         0x00000068, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",           0x00000088, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",   0x180000E0, RO, TRUE, stringReg,   20, 20,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                0x000000D8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",            0x000000E8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceScanType",          NOREF_ADDR, RO, TRUE, intVal,       4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceReset",             0x18000010, WO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "SensorWidth",             0x20000AB0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",            0x20000AC0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                0x20000AB0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "HeightMax",               0x20000AC0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""}, 
	{ "Width",                   0x2000006C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  4, 4,8192,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                  0x2000012C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0, 1,4096,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                 0x200000CC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0, 0,8188,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetY",                 0x2000018C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0, 0,4094,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "BinningHorizontal",       0x200001DC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningVertical",         0x200001EC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "PixelFormat",             0x2000005C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TestImageSelector",       0x2000098C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionMode",         0x20000020, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",        0x20000000, RW, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",         0x20000010, RW, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",        0x20000540, RW, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionArm",          0x200003A0, RW, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},
	{ "AcquisitionFrameCount",   0x20000030, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameRateMax", 0x20000388, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateMin", 0x20000384, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateRaw", 0x2000038C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

/*    |"FeatureName"                |Address    |Acc|Avail|  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                             |           |   |     |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "autoBrightnessMode",           0x2000570C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessSequence",       0x20005620, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetSource",   0x20005780, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "autoBrightnessTarget",         0x200055CC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetMin",      0x2000571C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetMax",      0x2000572C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetInc",      0x2000573C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetRangeVariation", 0x200055EC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetRangeMin", 0x2000574C, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetRangeMax", 0x2000575C, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 
 	{ "autoBrightnessAlgorithm",      0x20005790, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoHistogramWindowingLowerBoundary", 0x200064E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0, 99,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoHistogramWindowingUpperBoundary", 0x200064F0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1, 99,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoMinTimeActivationRaw",0x20005600, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1000,  0,  16000000,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoConvergenceTimeRaw",  0x20005610, RW, TRUE, integerRegLE, 4,  4,  0,  0,  100000,  100000,  16000000,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessSequence",                0x20005620, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoSource",        0x200057A0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoHostIPAddress", 0x200057B0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoHostIPPort",    0x200057C0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  65535,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 
 	{ "autoBrightnessROISelector",    NOREF_ADDR, RO, TRUE, intVal, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessROIMode",        0x2000563C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessROIWidth",       0x2000653C, RW, TRUE, integerRegLE, 4,  4,  0,  0, 4091,  4,  4091,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
  	{ "autoBrightnessROIHeight",      0x2000657C, RW, TRUE, integerRegLE, 4,  4,  0,  0, 4095,  0,  4095,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessROIOffsetX",     0x2000655C, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0,  0,  4091,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessROIOffsetY",     0x2000659C, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0,  0,  4095,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "ExposureAuto",                    0x200056CC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureAutoMaxValue",            0x200056DC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureAutoMaxValue_Streaming",  0x200056D8, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureAutoMinValue",            0x2000598C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	

	{ "ExposureMode",                    0x20000B3C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureTime",                 	 0x2000597C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureTimeRaw",              	 0x2000597C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureTimeMinValue",            0x20005974, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureTimeMaxValue",            0x20005978, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureTimeMin",                 0x20005974, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureTimeMax",                 0x20005978, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureAlignment",               0x20000B2C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureDelay",                   0x200051C0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureDelayMin",                0x200051C4, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureDelayMax",                0x200051C8, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	

	{ "GainSelector",            NOREF_ADDR, RW, TRUE, intVal,       4,  4,  0,  0,  0,  0,  1,           0,          0, NULL,    "",        ""},	
	{ "GainRaw",                 0x200002A0, RW, TRUE, integerRegLE, 4, 1056,0,  1,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "analogGainRaw",           0x200002A0, RW, TRUE, integerRegLE, 4, 1056,0,  1,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "digitialGainRaw",         0x200006C0, RW, TRUE, integerRegLE, 4, 1056,0,  1,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "BlackLevelRaw",           0x200002BC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 
	{ "BlackLevelMin",           0x200002B4, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 
	{ "BlackLevelMax",           0x200002B8, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 
  	{ "GainAuto",                0x200056EC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
  	{ "multiSlopeSensorResponseMode", 0x20000B8C, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "UserSetDefaultSelector",  0x08000000, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",         NOREF_ADDR, RW, TRUE, intVal,       4,  4,  0,  0,  6,  4,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",             0x08000020, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  6,         0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetSave",             0x08000010, RW, TRUE, integerRegLE, 4,  4,  0,  0,  4,  4,  5,         0x1,        0x1, NULL,    "UserSetSelector"}, 

  	{ "sensorId",                0x2000576C, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   { "PixelCoding",             0x20004F8C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "TriggerSelector",               NOREF_ADDR, RW, TRUE,  intVal,       4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,       "",                ""},	 
  	{ "TriggerMode",                   0x20004D7C, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "triggerIsAllowed",              0x20004E90, RO, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "triggerFrameCount",             0x20004DA0, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  1,  1, 65535,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	 
 	{ "TriggerSource",                 0x20004DD0, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0, 18,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "TriggerActivation",             0x20004E00, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  1,  0,  5,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "TriggerDelay",                  0x20004E60, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0,  2000000,  0xffffffff, 0xffffffff, NULL, "TriggerSelector", ""},	 
  	{ "TriggerOverlap",                0x20004E30, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "TriggerSoftware",               0x20004F70, WO, TRUE,  integerRegLE, 4,  4,  0,  0,  1,  0,  1,  0xffffffff, 0xffffffff, NULL,       "",                ""},	 

  	{ "LineSelector",                  NOREF_ADDR, RW, TRUE,  intVal,       4,  4,  0,  0,  0,  0,  7,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
  	{ "LineFormat",                    NOREF_ADDR, RO, TRUE,  intVal,       4,  4,  0,  7,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector",        ""},	 
  	{ "LineStatus",                    0x20000750, RW, TRUE,  integerRegLE, 4, 16,  0,  7,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "LineSelector",        ""},	 
  	{ "LineInverter",                  0x2000063C, RW, TRUE,  integerRegLE, 4, 16,  0,  7,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "LineSelector",        ""},	 
  	{ "LineStatusAll",                 0x200007D0, RO, TRUE,  integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

  	{ "lineDetectionLevel",            0x2000062C, RW, TRUE,  integerRegLE, 4,  4,  0,  0,  0x50,  0,  0xD0,  0xffffffff, 0xffffffff, NULL,   "",        ""},	 
  	{ "lineDeBouncingPeriod",          0x200005EC, RW, TRUE,  integerRegLE, 4, 16,  0,  3,  0,  0,  255,  0xffffffff, 0xffffffff, NULL,       "LineSelector",        ""},	 
  	{ "outputLineSource",              0x2000082C, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  0,  0,  0x1029,  0xffffffff, 0xffffffff, NULL,    "LineSelector",        ""},	 
  	{ "outputLinePulseActivation",     0x20000CEC, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,         "LineSelector",        ""},	 
  	{ "outputLinePulseDelay",          0x2000086C, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  1,  0,  16777215,  0xffffffff, 0xffffffff, NULL,  "LineSelector",        ""},	 
  	{ "outputLinePulseDuration",       0x200008AC, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  1,  0,  16777215,  0xffffffff, 0xffffffff, NULL,  "LineSelector",        ""},	 
  	{ "outputLineValue",               0x200007EC, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,          "LineSelector",        ""},	 
  	{ "outputLineSoftwareLatchControl",NOREF_ADDR, RO, TRUE,  intVal,       4, 16,  4,  7,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,         "LineSelector",        ""},	 
  	{ "outputLineSoftwareCmd",         0x200008E0, RW, TRUE,  integerRegLE, 4,  4,  0,  7,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL, "",        ""},	   
};

static GEV_REGISTER m_GenieTS_1_RegList[] = {
/*    |"FeatureName"           |Address    |Acc|Avail|  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                        |           |   |     |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "DeviceVendorName",        0x00000048, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",         0x00000068, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",           0x00000088, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",   0x180000E0, RO, TRUE, stringReg,   20, 20,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                0x000000D8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",            0x000000E8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceScanType",          NOREF_ADDR, RO, TRUE, intVal,       4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceReset",             0x18000010, WO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "SensorWidth",             0x20000AB0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",            0x20000AC0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                0x20000AB0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},
	{ "HeightMax",               0x20000AC0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""}, 
	{ "Width",                   0x20000060, RW, TRUE, integerRegLE, 4,  4,  0,  0,  4, 4,8192,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                  0x20000120, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0, 1,4096,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                 0x200000C0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0, 0,8188,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "OffsetY",                 0x20000180, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0, 0,4094,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "BinningHorizontal",       0x200001DC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningVertical",         0x200001EC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "PixelFormat",             0x20000050, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "TestImageSelector",       0x20000980, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionMode",         0x20000020, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",        0x12000360, RW, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",         0x12000370, RW, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",        0x12000380, RW, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionArm",          0x12000390, RW, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},
	{ "AcquisitionFrameCount",   0x20000030, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameRateMax", 0x20000388, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateMin", 0x20000384, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameRateRaw", 0x2000038C, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

/*    |"FeatureName"                |Address    |Acc|Avail|  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                             |           |   |     |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "autoBrightnessMode",           0x20005700, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessSequence",       0x20005620, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetSource",   0x20005780, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "autoBrightnessTarget",         0x200055C0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetMin",      0x20005710, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetMax",      0x20005720, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetInc",      0x20005730, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetRangeVariation", 0x200055E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetRangeMin", 0x20005740, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessTargetRangeMax", 0x20005750, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,65535,0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 
 	{ "autoBrightnessAlgorithm",      0x20005790, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoHistogramWindowingLowerBoundary", 0x200064E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0, 99,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoHistogramWindowingUpperBoundary", 0x200064F0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1, 99,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoMinTimeActivationRaw",0x20005600, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1000,  0,  16000000,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoConvergenceTimeRaw",  0x20005610, RW, TRUE, integerRegLE, 4,  4,  0,  0,  100000,  100000,  16000000,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessSequence",                0x20005620, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoSource",        0x200057A0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoHostIPAddress", 0x200057B0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessAlgoHostIPPort",    0x200057C0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  65535,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 
 	{ "autoBrightnessROISelector",    NOREF_ADDR, RO, TRUE, intVal, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessROIMode",        0x20005630, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessROIWidth",       0x20006530, RW, TRUE, integerRegLE, 4,  4,  0,  0, 4091,  4,  4091,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
  	{ "autoBrightnessROIHeight",      0x20006570, RW, TRUE, integerRegLE, 4,  4,  0,  0, 4095,  0,  4095,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessROIOffsetX",     0x20006550, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0,  0,  4091,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
 	{ "autoBrightnessROIOffsetY",     0x20006590, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0,  0,  4095,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "ExposureAuto",                    0x200056C0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureAutoMaxValue",            0x200056D0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureAutoMaxValue_Streaming",  0x200056D8, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureAutoMinValue",            0x20005980, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	

	{ "ExposureMode",                    0x20000B30, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureTime",                 	 0x20005970, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureTimeRaw",              	 0x20005970, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureTimeMinValue",            0x20005974, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureTimeMaxValue",            0x20005978, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureTimeMin",                 0x20005974, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "ExposureTimeMax",                 0x20005978, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureAlignment",               0x20000B20, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureDelay",                   0x200051C0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureDelayMin",                0x200051C4, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	
	{ "exposureDelayMax",                0x200051C8, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	// (*) in Genie but not in standard	

	{ "GainSelector",            NOREF_ADDR, RW, TRUE, intVal,       4,  4,  0,  0,  0,  0,  1,           0,          0, NULL,    "",        ""},	
	{ "GainRaw",                 0x200002A0, RW, TRUE, integerRegLE, 4, 1056,0,  1,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "analogGainRaw",           0x200002A0, RW, TRUE, integerRegLE, 4, 1056,0,  1,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "digitialGainRaw",         0x200006C0, RW, TRUE, integerRegLE, 4, 1056,0,  1,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "GainSelector", ""},	 
	{ "BlackLevelRaw",           0x200002B0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 
	{ "BlackLevelMin",           0x200002B4, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 
	{ "BlackLevelMax",           0x200002B8, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "BlackLevelSelector", ""},	 
  	{ "GainAuto",                0x200056EC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
  	{ "multiSlopeSensorResponseMode", 0x20000B80, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "UserSetDefaultSelector",  0x08000000, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",         NOREF_ADDR, RW, TRUE, intVal,       4,  4,  0,  0,  6,  4,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",             0x08000020, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  6,         0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetSave",             0x08000010, RW, TRUE, integerRegLE, 4,  4,  0,  0,  4,  4,  5,         0x1,        0x1, NULL,    "UserSetSelector"}, 

  	{ "sensorId",                0x20005760, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   { "PixelCoding",             0x20004F80, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "TriggerSelector",               NOREF_ADDR, RW, TRUE,  intVal,       4,  4,  0,  0,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,       "",                ""},	 
  	{ "TriggerMode",                   0x20004D70, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "triggerIsAllowed",              0x20004E90, RO, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "triggerFrameCount",             0x20004DA0, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  1,  1, 65535,  0xffffffff, 0xffffffff, NULL,    "TriggerSelector", ""},	 
 	{ "TriggerSource",                 0x20004DD0, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0, 18,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "TriggerActivation",             0x20004E00, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  1,  0,  5,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "TriggerDelay",                  0x20004E60, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0,  2000000,  0xffffffff, 0xffffffff, NULL, "TriggerSelector", ""},	 
  	{ "TriggerOverlap",                0x20004E30, RW, TRUE,  integerRegLE, 4, 16,  0,  2,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,       "TriggerSelector", ""},	 
  	{ "TriggerSoftware",               0x20004F70, WO, TRUE,  integerRegLE, 4,  4,  0,  0,  1,  0,  1,  0xffffffff, 0xffffffff, NULL,       "",                ""},	 

  	{ "LineSelector",                  NOREF_ADDR, RW, TRUE,  intVal,       4,  4,  0,  0,  0,  0,  7,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
  	{ "LineFormat",                    NOREF_ADDR, RO, TRUE,  intVal,       4,  4,  0,  7,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "LineSelector",        ""},	 
  	{ "LineStatus",                    0x20000750, RW, TRUE,  integerRegLE, 4, 16,  0,  7,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "LineSelector",        ""},	 
  	{ "LineInverter",                  0x20000630, RW, TRUE,  integerRegLE, 4, 16,  0,  7,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "LineSelector",        ""},	 
  	{ "LineStatusAll",                 0x200007D0, RO, TRUE,  integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

  	{ "lineDetectionLevel",            0x20000620, RW, TRUE,  integerRegLE, 4,  4,  0,  0,  0x50,  0,  0xD0,  0xffffffff, 0xffffffff, NULL,   "",        ""},	 
  	{ "lineDeBouncingPeriod",          0x200005E0, RW, TRUE,  integerRegLE, 4, 16,  0,  3,  0,  0,  255,  0xffffffff, 0xffffffff, NULL,       "LineSelector",        ""},	 
  	{ "outputLineSource",              0x20000820, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  0,  0,  0x1029,  0xffffffff, 0xffffffff, NULL,    "LineSelector",        ""},	 
  	{ "outputLinePulseActivation",     0x20000CE0, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  0,  0,  2,  0xffffffff, 0xffffffff, NULL,         "LineSelector",        ""},	 
  	{ "outputLinePulseDelay",          0x20000860, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  1,  0,  16777215,  0xffffffff, 0xffffffff, NULL,  "LineSelector",        ""},	 
  	{ "outputLinePulseDuration",       0x200008A0, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  1,  0,  16777215,  0xffffffff, 0xffffffff, NULL,  "LineSelector",        ""},	 
  	{ "outputLineValue",               0x200007E0, RW, TRUE,  integerRegLE, 4, 16,  4,  7,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,          "LineSelector",        ""},	 
  	{ "outputLineSoftwareLatchControl",NOREF_ADDR, RO, TRUE,  intVal,       4, 16,  4,  7,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,         "LineSelector",        ""},	 
  	{ "outputLineSoftwareCmd",         0x200008E0, RW, TRUE,  integerRegLE, 4,  4,  0,  7,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL, "",        ""},	   
};

static GEV_REGISTER m_Linea_0_RegList[] = {
/*    |"FeatureName"           |Address    |Acc|Avail|  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                        |           |   |     |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "DeviceVendorName",        0x00000048, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",         0x00000068, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",           0x00000088, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",   0x180000E0, RO, TRUE, stringReg,   20, 20,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                0x000000D8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceSerialNumber",      0x000000D8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",            0x000000E8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "deviceAcquisitionType",   NOREF_ADDR, RO, TRUE, intVal,       4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceReset",             0x18000010, WO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "DeviceTemperature",       0x200001B0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "UserSetDefaultSelector",   0x08000000, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  8,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",          NOREF_ADDR, RW, TRUE, intVal,       4,  4,  0,  0,  6,  4,  8,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",              0x08000020, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  8,         0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetSave",              0x08000010, RW, TRUE, integerRegLE, 4,  4,  0,  0,  4,  4,  8,         0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetError",             0x08000050, RO, TRUE, integerRegLE, 4,  4,  0,  0,  4,  4,  8,         0x1,        0x1, NULL,    "UserSetSelector"}, 

	{ "deviceBIST",              0x18008190, WO, TRUE, integerRegLE,  4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "deviceBISTStatus",        0x180081A0, RO, TRUE, integerRegLE,  4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "deviceBISTStatusAll",     0x180081AC, RO, TRUE, integerRegLE,  4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},

	{ "DeviceScanType",          NOREF_ADDR, RO, TRUE, intVal,       4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "sensorColorType",         0x20000140, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	

	{ "SensorWidth",             0x200000F0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",            0x20000100, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                0x200000F0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "HeightMax",               0x20000098, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	

	{ "Width",                   0x20000070, RW, TRUE, integerRegLE, 4,  4,  0,  0,1024,128,16384,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                  0x20000090, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0, 2,1024,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                 0x20000080, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0, 0,16255,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "BinningHorizontal",       0x20002EEC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "BinningVertical",         0x20002EFC, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  1,  4,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "PixelSize",               NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0,  1,  1,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},     
	{ "PixelColorFilter",        NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},     

	{ "PixelFormat",             0x20000060, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   { "PixelCoding",             0x20000130, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
   { "ReverseX",                0x20003100, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  1,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "AcquisitionLineRateRaw",  0x200001D0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xA100000,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionLineRateMax",  0x200001D4, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionLineRateMin",  0x200001D8, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	


	{ "TestImageSelector",       0x200000E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  7,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "TestPattern",             0x200000E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  7,  0xffffffff, 0xffffffff, NULL,    "",        ""},
   
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */

	{ "AcquisitionMode",          0x20000040, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",         0x12000360, WO, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",          0x12000370, WO, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",         0x12000380, WO, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionArm",           0x12000390, WO, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},
	{ "AcquisitionFrameCount",    0x20000050, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameCountMax", 0x20000058, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameCountMin", 0x20000054, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	
};

static GEV_REGISTER m_Nano_0_RegList[] = {
/*    |"FeatureName"           |Address    |Acc|Avail|  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                        |           |   |     |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */
 	{ "DeviceVendorName",        0x00000048, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceModelName",         0x00000068, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceVersion",           0x00000088, RO, TRUE, stringReg,   32, 32,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},  
	{ "DeviceFirmwareVersion",   0x180000E0, RO, TRUE, stringReg,   20, 20,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceID",                0x000000D8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceSerialNumber",      0x000000D8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "DeviceUserID",            0x000000E8, RO, TRUE, stringReg,   16, 16,  0,  0,  0,  0,  0,  0,          0,          NULL,    "",        ""},
	{ "deviceAcquisitionType",   NOREF_ADDR, RO, TRUE, intVal,       4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "DeviceReset",             0x18000010, WO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "DeviceTemperature",       0x20001570, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "UserSetDefaultSelector",  0x08000000, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""}, 
	{ "UserSetSelector",         NOREF_ADDR, RW, TRUE, intVal,       4,  4,  0,  0,  6,  4,  6,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "UserSetLoad",             0x08000020, RW, TRUE, integerRegLE, 4,  4,  0,  0,  6,  4,  6,         0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetSave",             0x08000010, RW, TRUE, integerRegLE, 4,  4,  0,  0,  4,  4,  6,         0x1,        0x1, NULL,    "UserSetSelector"}, 
	{ "UserSetError",            0x08000050, RO, TRUE, integerRegLE, 4,  4,  0,  0,  4,  4,  6,         0x1,        0x1, NULL,    "UserSetSelector"}, 

	{ "deviceBIST",              0x18008190, WO, TRUE, integerRegLE,  4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "deviceBISTStatus",        0x180081A0, RO, TRUE, integerRegLE,  4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},
	{ "deviceBISTStatusAll",     0x180081AC, RO, TRUE, integerRegLE,  4,  4,  0,  0,  1,  0,  0,  0x1,        0,          NULL,    "",        ""},

	{ "DeviceScanType",          NOREF_ADDR, RO, TRUE, intVal,       4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0,          NULL,    "",        ""},
	{ "sensorColorType",         0x20000140, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	

	
	{ "SensorWidth",             0x200000F0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "SensorHeight",            0x20000100, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "WidthMax",                0x200000F0, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	
	{ "HeightMax",               0x20000100, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0x00000000, NULL,    "",        ""},	

	{ "Width",                   0x20000070, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0, 0, 0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "Height",                  0x20000090, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0, 0, 0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetX",                 0x20001280, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0, 0, 0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
	{ "OffsetY",                 0x20001380, RW, TRUE, integerRegLE, 4,  4,  0,  0, 0, 0, 0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "PixelSize",               NOREF_ADDR, RO, TRUE, fixedVal,     4,  4,  0,  0,  1,  1,  3,  0xffffffff, 0xffffffff, NULL,    "",        ""},     
	{ "PixelColorFilter",        0x20001580, RO, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},     

	{ "PixelFormat",             0x20000060, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xffffffff,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 
    { "PixelCoding",             0x20000130, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	 

	{ "AcquisitionFrameRateRaw", 0x200000B0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xA100000,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameRateMax", 0x200000B4, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameRateMin", 0x200000B8, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	

	{ "TestImageSelector",       0x200000E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "TestPattern",             0x200000E0, RW, TRUE, integerRegLE, 4,  4,  0,  0,  0,  0,  0xff,  0xffffffff, 0xffffffff, NULL,    "",        ""},
   
/*    |"FeatureName"                 |Addres     |Acc|Avail |  Type      |Sz |Inc|Sel|Sel|Val|Val|Val| Read Mask  |Write Mask |Feature|Selector */
/*    |                              |           |   |      |            |   |   |Mn |Mx |Cur|Mn |Mx |            |           |struct |string  */

	{ "AcquisitionMode",          0x20000040, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  2,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionStart",         0x20000000, WO, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionStop",          0x20000010, WO, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	 
	{ "AcquisitionAbort",         0x20000020, WO, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},	
	{ "AcquisitionArm",           0x20000030, WO, TRUE, bitRegLE,     4,  4,  0,  0,  1,  0,  0,         0x1,        0x1, NULL,    "",        ""},
	{ "AcquisitionFrameCount",    0x20000050, RW, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},	
	{ "AcquisitionFrameCountMax", 0x20000058, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	{ "AcquisitionFrameCountMin", 0x20000054, RO, TRUE, integerRegLE, 4,  4,  0,  0,  1,  0,  0,  0xffffffff, 0xffffffff, NULL,    "",        ""},
	
};

//! Update default camera register structure with changed values.
/*! 
	This function overrides the entries in the GEV_REGISTER input_list with
	any corresponding entries in the update_list.
	\param [in] register_list	Default list of register structs.
	\param [in] update_list		List of updated register structs.
	\return None
	\note None
*/
static void UpdateCameraGigeRegs( GEV_CAMERA_HANDLE handle, DALSA_GENICAM_GIGE_REGS *register_list, GEV_REGISTER *update_list, int num_updates)
{
	if ( (register_list != NULL) && (update_list != NULL))
	{
		int i = 0;
		int j = 0;
		int found = FALSE;
		int numLeftover = 0;
		int input_size = sizeof(DALSA_GENICAM_GIGE_REGS) / sizeof(GEV_REGISTER);
		GEV_REGISTER *input_list = (GEV_REGISTER *)register_list;
		GEV_REGISTER *extraRegs = NULL;

		// Set up the leftover register list.
		extraRegs = (GEV_REGISTER *)malloc( num_updates * sizeof(GEV_REGISTER));

		if (extraRegs != NULL)
		{
			// Update the standard DALSA_GENICAM_GIGE_REGS structure.
			for (i = 0; i < num_updates; i++)
			{
				found = FALSE;
				for (j = 0; j < input_size; j++)
				{
					if ( strncmp( input_list[j].featureName, update_list[i].featureName, FEATURE_NAME_MAX_SIZE) == 0)
					{
						memcpy( &input_list[j], &update_list[i], sizeof(GEV_REGISTER));
						found = TRUE;
						break;
					}
				}
				if (!found)
				{
					// Place this register in the leftover register list.
					memcpy( &extraRegs[numLeftover++], &update_list[i], sizeof(GEV_REGISTER));
				}
			}

			// Finished processing the update list. Add any leftovers (not part of 
			//	the DALSA_GENICAM_GIGE_REGS structure) to the handle.
			GevSetCameraRegList( handle, numLeftover, extraRegs);
			
			free(extraRegs);
		}
	}
}


typedef struct _CAMERA_GEV_REG_DEFS
{
	char *name;
	cameraType type;
	int numRegs;
	GEV_REGISTER *regList;
} CAMERA_GEV_REG_DEFS, *PCAMERA_GEV_REG_DEFS;

static CAMERA_GEV_REG_DEFS m_defRegs[] = {
	{ "Genie HM", cameraGenieHM,    sizeof(m_GenieHM_RegList)/sizeof(GEV_REGISTER),    (GEV_REGISTER *)&m_GenieHM_RegList    },
	{ "Genie C",  cameraGenieColor, sizeof(m_GenieColor_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_GenieColor_RegList },
	{ "Genie M",  cameraGenieMono,  sizeof(m_GenieMono_RegList)/sizeof(GEV_REGISTER),  (GEV_REGISTER *)&m_GenieMono_RegList  },
	{ "Genie TS", cameraGenieTS,    sizeof(m_GenieTS_RegList)/sizeof(GEV_REGISTER),  (GEV_REGISTER *)&m_GenieTS_RegList  },
	{ "Genie_TS", cameraGenieTS,    sizeof(m_GenieTS_1_RegList)/sizeof(GEV_REGISTER),  (GEV_REGISTER *)&m_GenieTS_1_RegList  },
	{ "Cetus",    cameraDracoBased, sizeof(m_Cetus_Demo_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Cetus_Demo_RegList },
	{ "Xineos",   cameraDracoBased, sizeof(m_Cetus_Demo_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Cetus_Demo_RegList },
	{ "Peters EvalBoard",  cameraDracoBased, sizeof(m_Cetus_Demo_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Cetus_Demo_RegList },
	{ "CD30M221X1",  cameraDracoBased, sizeof(m_Cetus_Demo_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Cetus_Demo_RegList },
	{ "CORVUS",  cameraDracoBased, sizeof(m_Cetus_Demo_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Cetus_Demo_RegList },
	{ "CD32M321X1",  cameraDracoBased, sizeof(m_Cetus_Demo_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Cetus_Demo_RegList },
   { "CD45M2121",  cameraGenieTS, sizeof(m_DPIGC3_0_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_DPIGC3_0_RegList },	
	{ "Spyder GigE SG-11", cameraSpyder3SG114K, sizeof(m_Spyder3_04k80_SG_11_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Spyder3_04k80_SG_11_RegList },
	{ "Spyder GigE SG-11", cameraSpyder3SG111K, sizeof(m_Spyder3_01k80_SG_11_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Spyder3_01k80_SG_11_RegList },
	{ "Spyder GigE SG-14", cameraSpyder3SG144K, sizeof(m_Spyder3_04k80_SG_14_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Spyder3_04k80_SG_14_RegList },
	{ "Spyder GigE SG-32", cameraSpyder3SG324K, sizeof(m_Spyder3_04k80_SG_32_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Spyder3_04k80_SG_32_RegList },
	{ "Spyder GigE Colour Camera", cameraSpyder3SG344K, sizeof(m_Spyder3_04k80_SG_34_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Spyder3_04k80_SG_34_RegList },
   { "Linea",  cameraGenieTS, sizeof(m_Linea_0_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Linea_0_RegList },	
   { "Nano",  cameraGenieTS, sizeof(m_Nano_0_RegList)/sizeof(GEV_REGISTER), (GEV_REGISTER *)&m_Nano_0_RegList },	
};


//! Init camera register structure.
/*! 
	This function obtains the camera register structure for the camera accessed through 'handle'
	and loads them to the internal register strcture for the handle.
	\param [in] handle				Camera handle.
	\return GEV status code
	\note None
*/
#if COR_LINUX
	#define STRNICMP_FUNC	strncasecmp
#endif
#if COR_WIN32
	#define STRNICMP_FUNC	_strnicmp
#endif
GEV_STATUS GevInitCameraRegisters( GEV_CAMERA_HANDLE handle)
{
	GEV_STATUS      status = GEVLIB_ERROR_SOFTWARE;
	GEV_CAMERA_INFO *info = NULL;
	cameraType       type = cameraUnknown;
	BOOL             fSupportedDalsaCamera = FALSE;
	DALSA_GENICAM_GIGE_REGS *camera_registers = NULL;
	

	// Get the device info data structure from the handle's pointer.
	if (handle != NULL)
	{
		info = GevGetCameraInfo( handle );

		if (info != NULL)
		{
			// Allocate default camera register list.
			camera_registers = (DALSA_GENICAM_GIGE_REGS *)malloc( sizeof(DALSA_GENICAM_GIGE_REGS));
			if (camera_registers != NULL)
			{	
				int i = 0;
				int numSupported = sizeof(m_defRegs) / sizeof(CAMERA_GEV_REG_DEFS);

				// Set up the default registers and settings.
				memcpy(camera_registers, &m_Default_Regs, sizeof(m_Default_Regs));
				type = cameraUnknown;
				fSupportedDalsaCamera = FALSE;

				// Check if it is a supported camera.
				if ( Gev_IsSupportedCamera(handle) )	
				{
					// Find the current camera.
					for (i = 0; i < numSupported; i++)
					{
						if ( !STRNICMP_FUNC( info->model, m_defRegs[i].name, strlen(m_defRegs[i].name)))
						{
							// Found the camera.
							type = m_defRegs[i].type;
							fSupportedDalsaCamera = TRUE;
							UpdateCameraGigeRegs( handle, camera_registers, m_defRegs[i].regList, m_defRegs[i].numRegs);
							break;
						}
					}
				}
				
				// Update the camera DALSA_GENICAM_GIGE_REGS register info in the handle.
				status = GevSetCameraRegInfo( handle, type, fSupportedDalsaCamera, camera_registers, sizeof(DALSA_GENICAM_GIGE_REGS));
				free( camera_registers);
			}
		}
	}
	return status;
}



