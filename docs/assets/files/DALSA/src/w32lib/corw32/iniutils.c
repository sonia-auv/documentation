/****************************************************************************** 
Copyright (c) 2003 Coreco inc / 2010 DALSA Corp.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer. 

  - Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation 
    and/or other materials provided with the distribution. 

  - Neither the name of the DALSA nor the names of its contributors 
    may be used to endorse or promote products derived from this software 
    without specific prior written permission. 


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

===============================================================================

iniutils.c			 										

Description:
   Ini file access utilities.

Platform:
	-Generic Posix.

History:
   1.00 October 1st, 2003, parhug

$Log: iniutils.c $
Revision 1.6  2005/07/22 16:30:32  PARHUG
Merge with 1.5.1.4
Revision 1.5.1.4  2005/07/19 09:37:04  PARHUG
Fix FindKey to better strip out blanks before "=". Fix GetPrivateProfileSections to terminate list properly (double NULL).
Revision 1.5.1.3  2005/05/31 14:13:15  PARHUG
Fixed FindKey (again) to use the full known key and found key and not include the delimiter ('=').
Revision 1.5.1.2  2005/05/27 15:06:36  PARHUG
Fixed FindKey to handle missing key names and to compare the  
known key with the entire found key (and not only up to the known key length).
Fixed strnicmp to compare up to the maximum input length.
Removed MAXIMUM / MINIMUM macros, use MAX (from corposix.h) instead.
Revision 1.5  2005/03/21 11:50:21  PARHUG
Close ini file if memory allocation fails (in GetPrivateProfileString).
Revision 1.4  2005/02/25 15:46:04  parhug
Added GetPrivateProfileSectionNames and GetPrivateProfileSection (requires new StrStrpBrackets function)
Revision 1.3  2004/11/03 09:29:36  parhug
Add function _strrev to reverse characters in a string.
Revision 1.2  2004/10/25 10:24:48  BOUERI
- Fix bug in key write.
Revision 1.1  2004/08/19 12:26:07  parhug
Initial revision

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


#ifndef __CELL__
#include <cordef.h>
#endif

/**========================================================================**/
/*
* #define constants.
*/

#define MAX_LINE_LEN	512
#define MAXCONVERT	255

/**========================================================================**/
/*
* Function globals.
*/

/**========================================================================**/
/*
* Function prototypes.
*/
#ifdef __CELL__
#ifndef MAX
#define MAX( a, b ) ( ( (a) > (b) ) ? (a) : (b) )
#endif
#endif


/**============================== CODE ====================================**/

char * itoa ( int value, char * str, int base )
{
	sprintf(str,"%d",value);
	return str;
}

/*============================================================================
*
*  Name        : str2lwr
*  Description : This function converts a string to lower case
*
*  Parameters  :	pszSrc			: the source string (NULL terminated)
*						pszDest			: the destination string.
*
*  Returns     : Void
*
*  Context/Restrictions:
*===========================================================================*/
void str2lwr (const char *pszSrc, char *pszDest)
{
    char    *pszPnt = (char *)pszSrc;
    char    *pszPnt1 = pszDest;
    int     i, len;

	 len = strlen(pszSrc);
	 len++;

    for (i = 0; i < len; i++, pszPnt++, pszPnt1++)
    {
        if (*pszPnt == '\0')
        {
		  		*pszPnt1 = *pszPnt;
            return;
        }
        *pszPnt1 = tolower (*pszPnt);
    }
}


int strnicmp (const char *pszS1, const char *pszS2, size_t stMaxlen)
{
    char    *pszNew1 = NULL, *pszNew2 = NULL;
    int     len1, len2;
    int     iVal;

	 len1 = strlen(pszS1);
	 pszNew1 = (char *)malloc((len1+1)*sizeof(char));
	 if ( pszNew1 != NULL)
	 {
	 	str2lwr(pszS1, pszNew1);

		len2 = strlen(pszS2);
		pszNew2 = (char *)malloc((len2+1)*sizeof(char));

		if ( pszNew2 != NULL)
		{
			str2lwr(pszS2, pszNew2);

			// Both strings are the same case. Do the compare.
			iVal = strncmp( pszNew1, pszNew2, stMaxlen);

			// Free the memory.
			free(pszNew1);
			free(pszNew2);
			return iVal;
		}
		// Free the memory (allocation of second string failed).
		free(pszNew1);
	}
	return -1;
}


int stricmp (const char *pszS1, const char *pszS2)
{
	int len1, len2;
	len1 = strlen(pszS1);
	len2 = strlen(pszS2);
 	return strnicmp (pszS1, pszS2, MAX(len1, len2));
}


// Reverse the characters in a string.
char *_strrev( char *str)
{
	int len = strlen(str);
	unsigned char ch;
	int num = len/2;
	int i;

	for (i = 0; i < num; i++)
	{
		ch = str[i];
		str[i] = str[len-i];
		str[len-i] = ch;
	}
	return str;
}


/*============================================================================
*
*  Name        : StrStrip
*  Description : This function strips off any comments, then removes any
*                leading and/or trailing white space (spaces and tabs).
*
*  Parameters  : pszStr         : the string
*
*  Returns     : Void
*
*  Context/Restrictions:
*===========================================================================*/

static void StrStrip (char *pszStr)
{
	int	iIndex;
	char	*stripPtr;
	char    tmpStr[MAX_LINE_LEN]={0};

	// Make sure string not NULL pointer.
	if (pszStr == NULL)
	{
		return;
	}

	// Knock off any comment.
	//pszStr = strtok (pszStr, ";");
	stripPtr = strchr( pszStr, ';');
	if ( stripPtr != NULL)
	{
		stripPtr[0] = '\0';
	}
	iIndex = strlen (pszStr);
	if (iIndex == 0)
	{
		return;
	}

	// Knock off trailing spaces, tabs, or carriage return.
	iIndex--;

	while ((pszStr[iIndex] == ' ') || (pszStr[iIndex] == '\t') ||
			(pszStr[iIndex] == '\n') || (pszStr[iIndex] == '\r' ))
	{
		pszStr[iIndex--] = '\0';
		if (iIndex < 0)
		{
			return;
		}
	}

	// Knock off leading spaces and tabs.
	iIndex = 0;
	while ((pszStr[iIndex] == ' ') || (pszStr[iIndex] == '\t'))
	{
		iIndex++;
	}
	if (iIndex > 0)
	{
		strcpy (tmpStr, &pszStr[iIndex]);
		strcpy (pszStr,tmpStr);
	}
}

static void StrStripBrackets (char *pszStr)
{
	int	iIndex;
	char	*stripPtr;
	char    tmpStr[MAX_LINE_LEN]={0};

	// Make sure string not NULL pointer.
	if (pszStr == NULL)
	{
		return;
	}

	// Knock off trailing brackets.
	stripPtr = strchr( pszStr, ']');
	if ( stripPtr != NULL)
	{
		stripPtr[0] = '\0';
	}
	iIndex = strlen (pszStr);
	if (iIndex == 0)
	{
		return;
	}

	// Knock off leading bracket
	iIndex = 0;
	while (pszStr[iIndex] != '[')
	{
		iIndex++;
	}
	strcpy (tmpStr, &pszStr[iIndex+1]);
	strcpy (pszStr,tmpStr);

}



/*============================================================================
*
*  Name        : FindKey
*
*  Description : Returns the offset in the file of the specified key string.
*	The search is limited to the size of the section. If the key is not found
*	in the section, an "endOfSection" flag is set. The length of the line where
*	the key is located is also returned.

*
*  Parameters  :	fp			: File poiner (already open for read access).
*						pszBuf	: Buffer to return the line containing the key in.
*						bufSize	: Size of the buffer to return the key line in.
*						key		: The key being searched for in the file.
*						*lineSize		: Size of the line found.
*						*offset			: Offset in the file to the start of the line.
*
*  Returns     :	0 (FALSE) if key was not fond in the section.
*						1 (TRUE) otherwise.
*
*  Context/Restrictions:
*===========================================================================*/
static int FindKey( FILE *fp, char *pszBuf, int bufSize, const char *key,
							long *lineSize, long *offset)
{
	int len = 0;
	int keylen;
	int matchlen;
   int i;

	keylen = strlen(key);

	// Read one line at a time until parameter found, or new section.
	do
	{
 		// Get the next line of the file.
		*offset = ftell(fp);
		if (fgets (pszBuf, bufSize, fp) == NULL )
		{
			// End of file found (must be the last section)
			*lineSize = 0;
			return 0;
		}

		// Get the line length.
		len = strlen(pszBuf);

		// Strip off comments, and leading and trailing white space.
		StrStrip (pszBuf);

		// Not found if new section.
		if (pszBuf[0] == '[')
		{
			// End of section found
			*lineSize = 0;
			return 0;
		}

		// Not found if no actual key.
		if (pszBuf[0] == '=')
		{
			// Delimiter found without key.
			*lineSize = len;
			return 0;
		}

      // Find the "=" on the line
      for (i = 0; i < bufSize; i++)
      {
         if (pszBuf[i] == '=')
         {
           matchlen = i-1;
            break;
         }
         if (pszBuf[i] == '\0')
         {
            break;
         }
      }

		// Strip whitespace between 'key' and '='.
		while( (matchlen > 0) && (pszBuf[matchlen] == ' ' || pszBuf[matchlen] == '\t'))
		{
			matchlen--;
		}
		StrStrip( &pszBuf[matchlen+1]);

	} while( (strnicmp(pszBuf, key, MAX(keylen,matchlen+1)) != 0));

	// Return the found line size.
	*lineSize = len;
	return 1;
}



/*============================================================================
*
*  Name        : FindNextSection
*
*  Description : Returns the offset in the file of the next section. It also
*		indicates whether the EOF has been encountered before the section was found.
*		(meaning that the current section is the last one).
*
*  Parameters  :	
*		fp							: File poiner (already open for read access).
*		pszSectionName			: Buffer to return the name of the next section.
*		sectionNameSize		: Size of the buffer to return the name in.
*		*offset					: Offset in the file to the start of the named section.
*		*previous_end_offset : Offset in the file to the end of the previous section.
*		*endOfFile				: True/False indicating if EOF was found.
*
*  Returns     :	0 (FALSE) if section has a"\n" within the brackets.
*						1 (TRUE) otherwise.
*
*  Context/Restrictions:
*===========================================================================*/
static int FindNextSection( FILE *fp, char *pszSectionName, int sectionNameSize,
										long *offset, long *previous_end_offset, int *endOfFile)
{
	long start_offset = 0;
	
	// Mark that we are not at the End Of File.
	*endOfFile = 0;

	// Remember the start as (possibly) the end of the previous
	// section. (To handle the blank lines between sections).
	start_offset = ftell(fp);
	*previous_end_offset = start_offset;
	
	// Read one line at a time until the section is found.
	do
	{
		// Remember the offset to the start  of this line.
		*offset = ftell(fp);

		// If end of file, we need to create the section and write out the value.
		if (fgets (pszSectionName, sectionNameSize, fp) == NULL)
		{
			// End of file (This is not an error).
			*endOfFile = 1;
			return 1;
		}
		
		// Check for end of a section (blank line following text)
		if ( pszSectionName[0] == '\n')
		{
			if (*previous_end_offset == start_offset)
			{
				*previous_end_offset = ftell(fp);
			}
		}

		// Strip off comments, and leading and trailing white space.
		StrStrip (pszSectionName);
	} while (strchr (pszSectionName, '[') == 0);

	// Start of section found. Make sure end of section is in the same line.
	if ( strchr(pszSectionName, ']') == 0 )
	{
		// End of section not on the same line. This is an error.
		return 0;
	}

	return 1;
}

/*============================================================================
*
*  Name        : FindNamedSection
*
*  Description : Returns the offset in the file of the named section. It also
*		indicates whether the EOF has been encountered before the section was found.
*
*  Parameters  :	
*		fp							: File poiner (already open for read access).
*		pszSectionTitle		: Name of section (usually [name]. Brackets already attached).
*		*offset					: Offset in the file to the start of the named section.
*		*previous_end_offset : Offset in the file to the end of the previous section.
*		*endOfFile				: True/False indicating if EOF was found.
*
*  Returns     : 0 (FALSE) is memory allocation error. 1 (TRUE) otherwise.
*
*  Context/Restrictions:
*===========================================================================*/

static int FindNamedSection( FILE *fp, char *pszSectionTitle, long *offset, 
									long *previous_end_offset, int *endOfFile)
{
	char *pszLine = NULL;

	// Prepare buffers to search for section.
	pszLine = (char *)malloc( MAX_LINE_LEN * sizeof(char));
	if (pszLine == NULL)
	{
		// No memory. We are done.
		return 0;
	}

	// Set the read pointer to the start of the file.
	fseek(fp, 0, SEEK_SET);
	*endOfFile = 0;

	// Read one line at a time until the section is found.
	do
	{
		// Remember the offset to the start  of this line.
		*offset = ftell(fp);

		if ( FindNextSection( fp, pszLine, MAX_LINE_LEN*sizeof(char), offset, previous_end_offset, endOfFile) )
		{
			if ( *endOfFile )
			{
				free(pszLine);
				return 1;
			}
		}
		else
		{
			// There was a problem with the section header. It contained a newline
			// char before the final "]".
			// Just continue on (check for an EOF).
			// Maybe in the future there issomething we can do different here.
			if ( *endOfFile )
			{
				free(pszLine);
				return 1;
			}
		}

		// Strip off comments, and leading and trailing white space.
		StrStrip (pszLine);
    } while (stricmp (pszLine, pszSectionTitle) != 0);

	 // Section found. *offset contains position in file of start of section title.
		free(pszLine);
		return 1;
}

/*============================================================================
*
*  Name        : MoveDataInFile
*
*  Description : Moves data from one part of a file to another. The block from the
*	read offset until the end of file is moved to the location pointed to by the
*	write offset.
*	If the write offset occurs before the read offset in the file, the file is
*	truncated at the end of the new position of the read block.
*	If the read offset occurs before the write offset, the file is extended with
*	spaces inserted between the two blocks.
*
*  Parameters  :	fp				: File poiner (open for read/write access).
*						writeOffset	: Location to write to (relative to file start).
*						readOffset	: Location to read from (relative to file start).
*
*  Returns     : 0 (FALSE) is memory allocation error. 1 (TRUE) otherwise.
*
*  Context/Restrictions:
*===========================================================================*/
int MoveDataInFile( FILE *fp, long writeOffset, long readOffset)
{
	char *szbuf;
	long size;
//	int  i;
	size_t in;

	// Quick sanity check.
	if (writeOffset == readOffset)
		return 1;

	// Get the size remaining in the file.
	fseek(fp, 0, SEEK_END);
	size = ftell(fp) - readOffset;

	// Allocate a buffer for this data.
	szbuf = (char *)malloc(size);
	if (szbuf == NULL)
	{
		return 0;
	}

	// Read the entire block to be moved into memory.
	fseek(fp, readOffset, SEEK_SET);
	in = fread(szbuf, sizeof(char), (size_t)size, fp);

	// Set up the write pointer.
	fseek(fp, writeOffset, SEEK_SET);

	// Do the move depending on the relative locations of the write and read positions.
	if ( writeOffset < readOffset )
	{
		// Data from the read offset (closer to the end of the file)
		// is to be written starting at the write offset.

		fwrite( szbuf, sizeof(char), (size_t)size, fp);

#if 0
		// Pad the file with blanks in case ftruncate doesn't work on this platform.
		// (Entirely possible since it varies with library implementation).
		for (i = 0; i < (readOffset - writeOffset); i++)
		{
			fputc(' ', fp);
		}
#endif
		ftruncate( fileno(fp), (off_t)(writeOffset+(in*sizeof(char))));
	}
	else
	{
		fwrite( szbuf, sizeof(char), size, fp);
	}
	free(szbuf);
	return 1;
}

/*============================================================================
*
*  Name        : GetPrivatProfileString
*  Description : Reads a string from an INI-style config file.
*
*  Parameters  : pszSection - the name of the section
*                pszParameter - the name of the parameter
*                pszDefault - the default value if parameter not found
*                pszReturnBuffer - a pointer to the return buffer
*                iBufferSize - the size of the return buffer
*                pszFileName - the name of the .ini file
*
*  Returns     : The size of the string returned.
*                The string is returned in ReturnBuffer.
*
*  Context/Restrictions:
*===========================================================================*/
int GetPrivateProfileString(const char *pszSection, const char *pszParameter, char *pszDefault,
    char *pszReturnBuffer, int iBufferSize, const char *pszFileName)
{
   // Local variables.
	FILE *pFilePtr;				// Open file for reading.
	char *pszValuePtr;			// Pointer to the parameter value.
	char *pszFmtSection;			// Formatted section name.
	char *pszLine;					// One line of the .INI file.
	int status, endOfFile;
	long startOffset, prevOffset, paramSize;

	// Return default if the file cannot be opened.
	if ((pFilePtr = fopen (pszFileName, "r")) == NULL)
	{
		strncpy (pszReturnBuffer, pszDefault, iBufferSize);
		return (strlen (pszReturnBuffer));
	}

	// Format the section name as [SectionName].
	pszFmtSection = (char *)malloc(MAX_LINE_LEN * sizeof(char));
	if ( pszFmtSection == NULL)
	{
		fclose (pFilePtr);
		strncpy (pszReturnBuffer, pszDefault, iBufferSize);
		return (strlen (pszReturnBuffer));
	}
	sprintf (pszFmtSection, "[%s]", pszSection);

	pszLine = (char *)malloc(MAX_LINE_LEN * sizeof(char));
	if ( pszLine == NULL )
	{
		fclose (pFilePtr);
		free(pszFmtSection);
		strncpy (pszReturnBuffer, pszDefault, iBufferSize);
		return (strlen (pszReturnBuffer));
	}

	// Find the section
	status = FindNamedSection( pFilePtr, pszFmtSection, &startOffset, &prevOffset, &endOfFile);

	if (!status || endOfFile)
	{
		// Section not found. Return the default.
		fclose (pFilePtr);
		strncpy (pszReturnBuffer, pszDefault, iBufferSize);
		free(pszFmtSection);
		free(pszLine);
		return (strlen (pszReturnBuffer));
	}

	 // Find the key in this section.
	if ( !FindKey( pFilePtr, pszLine, MAX_LINE_LEN*sizeof(char), pszParameter, &paramSize, &startOffset) )
	{
		// Key not found. Return the default.
		fclose (pFilePtr);
		strncpy (pszReturnBuffer, pszDefault, iBufferSize);
		free(pszFmtSection);
		free(pszLine);
		return (strlen (pszReturnBuffer));
	}

	// Key was found. Extract the corresponding string.
	// Find the '='
	pszValuePtr = strchr (pszLine, '=');
	if ( pszValuePtr != NULL)
	{
		// '=' found. Point to the first non-whitespace after that.
		pszValuePtr++;
		StrStrip (pszValuePtr);

		// Check if there are actual characters.
		if (strlen(pszValuePtr) > 0)
		{
			// Copy up to iBufferSize chars to ReturnBuffer.
			strncpy (pszReturnBuffer, pszValuePtr, iBufferSize - 1);

			// Append the string termination character.
			pszReturnBuffer[iBufferSize - 1] = '\0';
		}
		else
		{
			// No characters left. Use the default.
			strncpy (pszReturnBuffer, pszDefault, iBufferSize);
		}
	}
	else
	{
		// No '=' found. Use the default.
		strncpy (pszReturnBuffer, pszDefault, iBufferSize);
	}

	// Close the file and return the string length.
	fclose (pFilePtr);
	free(pszFmtSection);
	free(pszLine);
	return strlen (pszReturnBuffer);

}

/*============================================================================
*
*  Name        : GetPrivateProfileInt
*
*  Description : Get an integer from a ".INI"-style file.
*
*  Parameters  : section     	: .INI section name
*                key   			: .INI parameter name
*                default       	: default integer value if key not found.
*                filename    	: .INI file name
*
*  Returns     : Integer value.
*
*  Context/Restrictions:
*===========================================================================*/
int		GetPrivateProfileInt( const char *section, const char *key,
							  int defaultValue, const char *filename)
{
	int 	Value;
	char	*hexDelimiter;
	char	valueString[16], defaultString[16];
	// Check if file exists.
	if ( CheckFileExists( filename, 0) )
	{
		// Set up default string.
		sprintf( defaultString, "%d", defaultValue);

		// Get string from file.
		GetPrivateProfileString( section, key, defaultString, valueString,
					sizeof(valueString), (char *)filename);

		// Convert string to integer (check for any hex characters beforehand).
		str2lwr(valueString, valueString);
		hexDelimiter = strchr(valueString, 'x');
		if ( hexDelimiter == NULL)
		{
			Value = strtol(valueString, NULL, 10);
		}
		else
		{
			Value = strtol( valueString, NULL, 16);
		}
	}
	else
	{
		// File does not exist. Set up default.
		Value = defaultValue;
	}
	return Value;
}

/*============================================================================
*
*  Name        : GetPrivateProfileFloat
*
*  Description : Get a float from a ".INI"-style file.
*
*  Parameters  : section     	: .INI section name
*                key   			: .INI parameter name
*                default       	: default float value if key not found.
*                filename    	: .INI file name
*
*  Returns     : Float value.
*
*  Context/Restrictions:
*===========================================================================*/
float	GetPrivateProfileFloat( const char *section, const char *key,
								float defaultValue, const char *filename)
{
	float	Value;
	char 	*hexDelimiter;
	char	valueString[16], defaultString[16];
	// Check if file exists.
	if ( CheckFileExists( filename, 0) )
	{
		// Set up default string.
		sprintf( defaultString, "%f", defaultValue);

		// Get string from file.
		GetPrivateProfileString( section, key, defaultString, valueString,
					sizeof(valueString), (char *)filename);

		// Convert string to integer (check for any hex characters beforehand).
		str2lwr(valueString, valueString);
		hexDelimiter = strchr(valueString, 'x');
		if ( hexDelimiter == NULL)
		{
			Value = (float)atof(valueString);
		}
		else
		{
			// Hex not allowed for floating point.
			Value = defaultValue;
		}
	}
	else
	{
		// File does not exist. Set up default.
		Value = defaultValue;
	}
	return Value;
}



unsigned long GetPrivateProfileSectionNames( char *sectionNames, unsigned long nSize, const char *filename)
{
   // Local variables.
	FILE *fp;				// Open file for reading.
	unsigned long count = 0;	// Count of characters in output buffer.
	unsigned long len = 0;		// Length of each line.
	unsigned long remaining = nSize;	// Size remaining in output buffer.
	char *pszSection;				// Section name.
	int endOfFile;
	long offset, prev_offset;

	// Initialize the output array.
	if ( sectionNames != NULL )
	{
		sectionNames[0] = '\0';
		if (nSize > 1)
		{
			sectionNames[1] = '\0';
		}
	}
	else
	{
		return count;
	}

	// Return default if the file cannot be opened.
	if ((fp = fopen (filename, "r")) == NULL)
	{
		return count;
	}

	// Get a line buffer to contain section names as [SectionName].
	pszSection = (char *)malloc(MAX_LINE_LEN * sizeof(char));
	if ( pszSection == NULL)
	{
		return count;
	}
	// Set the read pointer to the start of the file.
	fseek(fp, 0, SEEK_SET);
	endOfFile = 0;

	// Read one line at a time until the section is found.
	while(1)
	{
		// Remember the offset to the start  of this line.
		offset = ftell(fp);

		if ( FindNextSection( fp, pszSection, MAX_LINE_LEN*sizeof(char), &offset, &prev_offset, &endOfFile) )
		{
			StrStrip (pszSection);
			if ( endOfFile )
			{
				if (remaining > 2)
				{
					sectionNames[count] = '\0';
					count = count+1;
				}
				else
				{
					// Buffer is full. Last 2 spaces need to be NULL.
					sectionNames[nSize-1] = '\0';
					sectionNames[nSize-2] = '\0';
					count = nSize - 2;
				}
				free(pszSection);
				fclose(fp);
				return count;
			}
			else
			{
				// Output this section to the output buffer.
				StrStripBrackets(pszSection);
				len = snprintf( &sectionNames[count], remaining, "%s", pszSection);
				remaining -= len;
				if (remaining > 2)
				{
					count += len;
					sectionNames[count] = '\0';
					count += 1;
					remaining -= 1;
				}
				else
				{
					// Buffer is full. Last 2 spaces need to be NULL.
					sectionNames[nSize-1] = '\0';
					sectionNames[nSize-2] = '\0';
					fclose(fp);
					free(pszSection);
					return (nSize - 2);
				}
			}
		}
		else
		{
			// There was a problem with the section header. It contained a newline
			// char before the final "]".
			// Just continue on (check for an EOF).
			// Maybe in the future there issomething we can do different here.
			if ( endOfFile )
			{
				fclose(fp);
				free(pszSection);
				return count;
			}
		}
	}
	fclose(fp);
	free(pszSection);
	return count;
}

unsigned long GetPrivateProfileSection( char *section, char *sectionData, unsigned long nSize, const char *filename)
{
   // Local variables.
	FILE *pFilePtr;				// Open file for reading.
	char *pszFmtSection;			// Formatted section name.
	char *pszLine;					// One line of the .INI file.
	unsigned long count = 0;	// Count of characters in output buffer.
	unsigned long len = 0;		// Length of each line.
	unsigned long remaining = nSize;	// Size remaining in output buffer.
	int status, endOfFile;
	long offset, startOffset, prevOffset;

	// Initialize the output array.
	if ( sectionData != NULL )
	{
		sectionData[0] = '\0';
		if (nSize > 1)
		{
			sectionData[1] = '\0';
		}
	}
	else
	{
		return count;
	}

	// Return default if the file cannot be opened.
	if ((pFilePtr = fopen (filename, "r")) == NULL)
	{
		return count;
	}

	// Format the section name as [SectionName].
	pszFmtSection = (char *)malloc(MAX_LINE_LEN * sizeof(char));
	if ( pszFmtSection == NULL)
	{
		return count;
	}
	sprintf (pszFmtSection, "[%s]", section);

	pszLine = (char *)malloc(MAX_LINE_LEN * sizeof(char));
	if ( pszLine == NULL )
	{
		free(pszFmtSection);
		return count;
	}

	// Find the section
	status = FindNamedSection( pFilePtr, pszFmtSection, &startOffset, &prevOffset, &endOfFile);

	if (!status || endOfFile)
	{
		// Section not found. Return the default.
		fclose (pFilePtr);
		free(pszFmtSection);
		free(pszLine);
		return count;
	}

	// Output all of the lines in this section.
	// Mark that we are not at the End Of File.
	endOfFile = 0;

	// Read one line at a time until the section end is found (at start of next one).
	do
	{
		// Remember the offset to the start  of this line.
		offset = ftell(pFilePtr);

		// If end of file, we need to create the section and write out the value.
		if (fgets (pszLine, MAX_LINE_LEN * sizeof(char), pFilePtr) == NULL)
		{
			// End of file (This is not an error).
			endOfFile = 1;
			break;
		}

		// Strip off comments, and leading and trailing white space.
		StrStrip (pszLine);

		// Place this line in the output buffer.
		len = snprintf( &sectionData[count], remaining, "%s", pszLine);
		remaining -= len;
		if (remaining > 2)
		{
			count += len;
			sectionData[count] = '\0';
			count += 1;
			remaining -= 1;
		}
		else
		{
			// Buffer is full. Last 2 spaces need to be NULL.
			sectionData[nSize-1] = '\0';
			sectionData[nSize-2] = '\0';
			fclose (pFilePtr);
			free(pszFmtSection);
			free(pszLine);
			return (nSize - 2);
		}

	} while (strchr (pszLine, '[') == 0);

	// Start of next section found (already placed in buffer, back it out).
	if (remaining > 2)
	{
		count -= len + 1;
		sectionData[count++] = '\0';
		sectionData[count] = '\0';
	}
	else
	{
		// Buffer is full. Last 2 spaces need to be NULL.
		sectionData[nSize-1] = '\0';
		sectionData[nSize-2] = '\0';
		count = nSize - 2;
	}

	fclose (pFilePtr);
	free(pszFmtSection);
	free(pszLine);
	return count;
}

int WritePrivateProfileSection( char *section, char *sectionData, const char *filename)
{
	// Not implemented yet.
	return 0;
}


/*============================================================================
*
*  Name        : WritePrivatProfileString
*  Description : Writes a string to an INI-style config file.
*
*  Parameters  : section 	- the name of the section
*                key 		- the name of the key
*                string 	- the string to be written
*                filename	- the name of the .ini file
*
*  Returns     : TRUE/FALSE for success/failure.
*
*  Context/Restrictions:
*===========================================================================*/
int WritePrivateProfileString( const char *section, const char *key, char *string, const char * filename)
{
	FILE *fp = NULL;
	int  status = 1, endOfFile;
	long startOffset, endOffset, keyOffset, prevOffset;
	char *pszLine;					// One line of the .INI file.
	char *pszFmtSection;			// Section header in the .INI file.
	long lineSize;
	int  len;

	// Open / create the file.
	if ( !CheckFileExists( filename, 1) )
	{
		// File does not exist. See if it can be created.
		fp = fopen( filename, "w+");
		if ( fp == NULL )
		{
			// Unable to create file.
			return 0;
		}
	}
	else
	{
		// Open the file for R/W access.
		fp = fopen( filename, "r+");
	}

	// Prepare buffer to search for section.
	pszLine = (char *)malloc( MAX_LINE_LEN * sizeof(char));
	if (pszLine == NULL)
	{
		fclose(fp);
		return 1;
	}

	pszFmtSection = (char *)malloc( MAX_LINE_LEN * sizeof(char));
	if (pszLine == NULL)
	{
		fclose(fp);
		free(pszLine);
		return 0;
	}

	sprintf(pszFmtSection,"[%s]", section);

	if ( !FindNamedSection( fp, pszFmtSection, &startOffset, &prevOffset, &endOfFile) )
	{
		// Error allocating memory.
		fclose (fp);
		free(pszLine);
		free(pszFmtSection);
		return 0;
	}

	// Check for EOF (append new section and key value).
	if (endOfFile)
	{
		// Write the new section header.
		fprintf(fp,"\n%s\n", pszFmtSection);

		// Write the key and string.
		fprintf(fp,"%s=%s\n\n", key, string);

		// Close the file and return.
		fclose (fp);
		free(pszLine);
		free(pszFmtSection);
		return 1;
	}

	// Find the offset to the next section.
	endOffset = ftell(fp);
	fseek(fp, endOffset, SEEK_SET);

	// We have the section. Check if the key is blank.
	if ( !strcmp(key,"") || !strcmp(key," ") )
	{
		long dummy;
		
		// Find the next section.
		FindNextSection( fp, pszFmtSection, MAX_LINE_LEN*sizeof(char), &endOffset, &dummy, &endOfFile);

		// Delete the section. (Move data from after the section to start at the section).
		//MoveDataInFile(fp, startOffset, endOffset);
		MoveDataInFile(fp, prevOffset, endOffset);

		// Delete the section (depends if EOF or not).
		if ( endOfFile )
		{
			// Delete the last section by truncating the file.
			if ( !ftruncate( fileno(fp), (off_t) startOffset) )
			{
				// File truncated successfully.
				fclose(fp);
				free(pszLine);
				free(pszFmtSection);
				return 1;
			}
		}
		else
		{
			// Delete the section. (Move data from after the section to start at the section).
			//MoveDataInFile(fp, startOffset, endOffset);

			// Get the current position in the file opened for R/W.
			startOffset = ftell(fp);

			// Truncate the file.
			ftruncate(fileno(fp), (off_t)startOffset);
			fflush(fp);
			fclose(fp);
			free(pszLine);
			free(pszFmtSection);
			return 1;
		}
	}
	else
	{
		// Need to add or modify the key in this section.
		// Prepare the key string in a buffer.
		pszFmtSection[0] = '\0';
		len = sprintf(pszFmtSection,"%s=%s", key, string);

		// Find the key, returning the length of the line it is on.
		status = FindKey( fp, pszLine, MAX_LINE_LEN*sizeof(char), key, &lineSize, &keyOffset);

		// Check for end of section.
		if (!status)
		{
			// Append new key to section. Move the data and insert the new string.
			status = MoveDataInFile( fp, (endOffset+len+1), endOffset);
			fseek(fp, endOffset, SEEK_SET);
			fprintf(fp, "%s\n", pszFmtSection);
		}
		else
		{
			// Insert key into section, overwriting the old key.
			if ( len < lineSize )
			{
				// New string will fit in current location (with a newline).
				fseek(fp, keyOffset, SEEK_SET);
				fprintf(fp, "%s\n", pszFmtSection);
				status = MoveDataInFile( fp, (keyOffset+len+1), (keyOffset+lineSize));
			}
			else
			{
				// New string will not fit. Move the data and insert the new string.
				status = MoveDataInFile( fp, (keyOffset+len+1), (keyOffset+lineSize));
				fseek(fp, keyOffset, SEEK_SET);
				fprintf(fp, "%s\n", pszFmtSection);
			}
		}
	}

	// Close the file and free the memory.
	fclose(fp);
	free(pszLine);
	free(pszFmtSection);
	return status;
}

/*============================================================================
*
*  Name        : WritePrivatProfileInt
*  Description : Writes a string to an INI-style config file.
*
*  Parameters  : section 	- the name of the section
*                key 		- the name of the key
*                value	 	- the int to be written
*                filename	- the name of the .ini file
*
*  Returns     : TRUE/FALSE for success/failure.
*
*  Context/Restrictions:
*===========================================================================*/
int WritePrivateProfileInt( const char *section, const char * key, int value, const char * filename)
{
	char valueString[16];

	// Write the int to a string.
	sprintf( valueString, "%d", value);

	// Add it to the file.
	return WritePrivateProfileString( section, key, valueString, filename);
}

/*============================================================================
*
*  Name        : WritePrivatProfileFloat
*  Description : Writes a string to an INI-style config file.
*
*  Parameters  : section 	- the name of the section
*                key 		- the name of the key
*                value	 	- the float to be written
*                filename	- the name of the .ini file
*
*  Returns     : TRUE/FALSE for success/failure.
*
*  Context/Restrictions:
*===========================================================================*/
int WritePrivateProfileFloat( const char *section, const char * key, float value, const char * filename)
{
	char valueString[16];

	// Write the int to a string.
	sprintf( valueString, "%f", value);

	// Add it to the file.
	return WritePrivateProfileString( section, key, valueString, filename);
}
