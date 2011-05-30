// Main.cpp
//
//////////////////////////////////////////////////////////////////////
//
// Asfpp 1.0
// The Alajar Scripting Format Preprocessor
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "Osal/File.h"

void RunAsfpp (const char* pszFileName);

char* GetNextCharacter();
void WriteCodeBlock();
int WriteIncludeBlock();
void WriteIndent();
char* FindNextCodeBlock();
void WriteHTMLBlock();


/* Start with <% Includes, defines, etc. %> */

// Buffer specs
int iBufferSize;
char* pszBuffer;

// File specs
size_t g_iPos;
size_t g_iFileSize;
FILE* fWrite;
FILE* fHWrite;

// Text parsing variables
char g_pszOpen [] = "<%";
char g_pszClose [] = "%>";

char g_pszDefineWrite [] = "m_pHttpResponse->WriteText";

bool g_bUseClass = true;
char g_pszClassName [] = "HtmlRenderer";
char g_pszPrefix [] = "Render_";

bool g_bPassInRequestResponse = false;
bool g_bGenerateHFile = false;

char g_pszInclude [] = "#include \"../HtmlRenderer/HtmlRenderer.h\"\n\n";

bool g_bIncludeAlajar = false;

size_t g_iOpenLength;
size_t g_iCloseLength;
size_t g_iIndentLevel;

char* g_pszBigBuffer = NULL;

// Syntax: Asfpp File.asf
// Produces File.h and File.cpp with function Render()
int main (int argc, char* argv[]) {

    // Check for wrong number of arguments
    if (argc != 2) {
        fprintf (stderr, "Usage: asfpp <filename>\n");
        exit(-1);
    }

    // Prepare file and method names
    int i, iNumFiles;
    const char** ppszFileName;

    FileEnumerator fEnum;

    int iErrCode = File::EnumerateFiles (argv[1], &fEnum);
    if (iErrCode != OK) {
        fprintf (stderr, "No files were found");
        exit(-1);
    }
    
    ppszFileName = fEnum.GetFileNames();
    iNumFiles = fEnum.GetNumFiles();

    printf ("asfpp v1.0\n");
    printf ("Copyright (c) 1998 Max Attar Feingold\n\n");
    printf ("Processing files...\n");
    for (i = 0; i < iNumFiles; i ++) {
        RunAsfpp (ppszFileName[i]);
    }

    switch (iNumFiles) {
    case 0:
        printf ("No files were processed\n");
        break;

    case 1:
        printf ("\n%i file successfully processed\n", iNumFiles);
        break;

    default:
        printf ("\n%i files successfully processed\n", iNumFiles);
        break;
    }
}

void RunAsfpp (const char* pszAsfName) {

    // Initialize globals
    g_iPos = 0;
    g_iFileSize = 0;
    g_iOpenLength = 0;
    g_iCloseLength = 0;
    g_iIndentLevel = 0;

    char* pszAsfCopy = new char [strlen (pszAsfName) + 1];
    strcpy (pszAsfCopy, pszAsfName);

    char* pszPageName = strtok (pszAsfCopy, ".");

    if (pszPageName == NULL) {
        fprintf (stderr, "Usage: asfpp <filename>\n");
        exit(-1);
    }

    char* pszCppName = new char [strlen (pszPageName) + 5];
    strcpy (pszCppName, pszPageName);
    strcat (pszCppName, ".cpp");

    char* pszHName = NULL;
    
    if (g_bGenerateHFile) {
        pszHName = new char [strlen (pszPageName) + 3];
        strcpy (pszHName, pszPageName);
        strcat (pszHName, ".h");
    }

    // Initialize input file
    FILE* fp = fopen (pszAsfName, "r");
    if (fp == NULL) {
        fprintf (stderr, "Error opening input file %s\n", pszAsfName);
        exit (-1);
    }
    
    // Initialize output files
    fWrite = fopen (pszCppName, "wt");
    if (fWrite == NULL) {
        fprintf (stderr, "Error opening output file %s\n", pszCppName);
        exit (-1);
    }

    if (g_bGenerateHFile) {
    
        fHWrite = fopen (pszHName, "wt");
        if (fHWrite == NULL) {
            fprintf (stderr, "Error opening output file %s\n", pszHName);
            exit (-1);
        }

        fprintf (fHWrite, "#ifndef _%s_H_\n#define _%s_H_\n\n", pszPageName, pszPageName);
        fprintf (
            fHWrite, 
            "int %s%s%s%s%s(%s);\n", 
            g_bUseClass ? g_pszClassName : "",
            g_bUseClass ? "::" : "",
            g_pszPrefix,
            pszPageName,
            g_bPassInRequestResponse ? " " : "",
            g_bPassInRequestResponse ? "IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse" : ""
            );
        
        fprintf (fHWrite, "\n#endif");
        fclose (fHWrite);
    }

    // Obtain input file's size
    struct _stat stFileInfo;
    if (_stat (pszAsfName, &stFileInfo) != 0) {
        fprintf (stderr, "Error opening input file %s\n", pszAsfName);
        exit (-1);
    }

    // Allocate a buffer for the file
    iBufferSize = stFileInfo.st_size;
    pszBuffer = new char [iBufferSize + 1];

    g_pszBigBuffer = new char [iBufferSize + 1];
    
    // Read input file into memory
    g_iFileSize = fread (pszBuffer, sizeof (char), iBufferSize, fp);
    pszBuffer[g_iFileSize] = '\0';
    fclose (fp);
    g_iOpenLength = strlen (g_pszOpen);
    g_iCloseLength = strlen (g_pszClose);
    g_iIndentLevel = 0;

    // Make sure the file has content
    char* pszTemp;
    if ((pszTemp = GetNextCharacter()) == NULL) {
        fprintf (stderr, "Input file %s contains no data\n", pszAsfName);
        exit (-1);
    }

    // Write system includes
    if (g_bIncludeAlajar) {
        fprintf (fWrite, "#include \"Alajar.h\"\n");
    }

    // Check for include section
    int iRetVal = 0;
    if (strncmp (pszTemp, g_pszOpen, g_iOpenLength) == 0) {
        iRetVal = WriteIncludeBlock();
    }

    if (g_bGenerateHFile) {

        fprintf (fWrite, "#include \"");
        fprintf (fWrite, pszHName);
        fprintf (fWrite, "\"\n\n");
    }

    fprintf (fWrite, g_pszInclude);

    fprintf (fWrite, "#define Write %s\n\n", g_pszDefineWrite);

    // Write Render header
    fprintf (
        fWrite, 
        "// Render the %s page\n"\
        "int %s%s%s%s%s(%s) {\n", 
        pszPageName,
        g_bUseClass ? g_pszClassName : "",
        g_bUseClass ? "::" : "",
        g_pszPrefix,
        pszPageName,
        g_bPassInRequestResponse ? " " : "",
        g_bPassInRequestResponse ? "IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse" : ""
        );

    g_iIndentLevel = 1;

    if (iRetVal == 1) {
        WriteCodeBlock();
    }

    while (g_iPos < g_iFileSize) {

        // Write HTML block
        WriteHTMLBlock();

        if (g_iPos < g_iFileSize) {
            WriteCodeBlock();
        }
    }

    // Close cpp file
    fprintf (fWrite, "\n}");
    fclose (fWrite);

    printf ("%s\n", pszAsfName);

    delete [] pszCppName;

    if (g_bGenerateHFile) {
        delete [] pszHName;
    }

    delete [] pszAsfCopy;
    delete [] pszBuffer;

    delete [] g_pszBigBuffer;
}


char* GetNextCharacter() {

    while (g_iPos < g_iFileSize) {
        if (pszBuffer[g_iPos] != ' ' && 
            pszBuffer[g_iPos] != '\t' &&
            pszBuffer[g_iPos] != '\n' &&
            pszBuffer[g_iPos] !=  '\r') {
            return &(pszBuffer[g_iPos]);
        }

        g_iPos ++;
    }

    return NULL;
}

void WriteCodeBlock() {

    g_iPos += g_iOpenLength;

    GetNextCharacter();
    fprintf (fWrite, "\n");

    bool bIndented = false;

    while (g_iPos < g_iFileSize) {
        if (strncmp (&(pszBuffer[g_iPos]), g_pszClose, g_iCloseLength) == 0) {
            g_iPos += g_iCloseLength;
            return;
        }
        
        switch (pszBuffer[g_iPos]) {
            
        case '{':
            //g_iIndentLevel ++;
            
            if (!bIndented) {
                WriteIndent();
                bIndented = true;
            }

            fprintf (fWrite, "{");
            break;
            
        case '}':
            //g_iIndentLevel --;

            if (!bIndented) {
                WriteIndent();
                bIndented = true;
            }

            fprintf (fWrite, "}");
            break;

        case '\n':
            fprintf (fWrite, "\n");
            bIndented = false;
            break;
            
        default:
            
            if (!bIndented) {
                WriteIndent();
                bIndented = true;
            }

            fprintf (fWrite, "%c", pszBuffer[g_iPos]);
            break;
        }

        g_iPos ++;
    }
}


int WriteIncludeBlock() {

    bool bInclude = false;

    g_iPos += g_iOpenLength;

    GetNextCharacter();
    fprintf (fWrite, "\n");

    while (g_iPos < g_iFileSize) {
        
        if (strncmp (&(pszBuffer[g_iPos]), g_pszClose, g_iCloseLength) == 0) {
            g_iPos += g_iCloseLength;
            return 0;
        }
        
        switch (pszBuffer[g_iPos]) {
            
        case '#':
                        
            fprintf (fWrite, "#");
            bInclude = true;

            break;

        case '\n':
            fprintf (fWrite, "\n");
            bInclude = false;
            break;
            
        default:

            if (bInclude) {
                fprintf (fWrite, "%c", pszBuffer[g_iPos]);
            } else {
                g_iPos -= g_iOpenLength;
                return 1;
            }

            break;
        }

        g_iPos ++;
    }

    return 0;
}


void WriteIndent() {

    for (size_t i = 0; i < g_iIndentLevel; i ++) {
        fprintf (fWrite, "\t");
    }
}


char* FindNextCodeBlock() {
    return strstr (&(pszBuffer [g_iPos]), g_pszOpen);
}


void WriteHTMLBlock() {
    
    char* pszCode = NULL, * pszEndCode, * pszTemp;
    bool bEmpty;
    
    // Find next non-empty code block
    while ((pszCode = FindNextCodeBlock()) != NULL) {

        pszEndCode = strstr (pszCode + 1, g_pszClose);
        if (pszEndCode == NULL) {
            printf ("Error: could not find code block closure.  File may not compile properly");
            break;
        } else {
            pszTemp = pszCode + 2;
            bEmpty = true;
            
            while (pszTemp < pszEndCode) {
                if (*pszTemp != ' ' &&
                    *pszTemp != '\n' &&
                    *pszTemp != '\r' &&
                    *pszTemp != '\t') {
                    bEmpty = false;
                    break;
                }
                pszTemp ++;
            }
            
            if (bEmpty) {
                strcpy (pszCode, pszEndCode + 2);
                g_iFileSize -= (pszEndCode + 2 - pszCode);
            } else {
                break;
            }
        }
    }

    size_t stIndex = 0;
    g_pszBigBuffer[0] = '\0';

    if (pszCode == NULL) {
        
        while (g_iPos < g_iFileSize) {
            
            switch (pszBuffer[g_iPos]) {
            case '\n':
                g_pszBigBuffer[stIndex ++] = '\\';
                g_pszBigBuffer[stIndex ++] = 'n';
                break;

            case '\t':
                g_pszBigBuffer[stIndex ++] = '\\';
                g_pszBigBuffer[stIndex ++] = 't';
                break;

            case '\"':
                g_pszBigBuffer[stIndex ++] = '\\';
                g_pszBigBuffer[stIndex ++] = '\"';
                break;

            case '\\':
                g_pszBigBuffer[stIndex ++] = '\\';
                g_pszBigBuffer[stIndex ++] = '\\';
                break;

            case '%':
                g_pszBigBuffer[stIndex ++] = '%';
                g_pszBigBuffer[stIndex ++] = '%';
                break;

            default:

                g_pszBigBuffer[stIndex ++] = pszBuffer[g_iPos];
                break;
            }

            g_iPos ++;
        }
        
    } else {

        while (&(pszBuffer[g_iPos]) < pszCode) {    
            
            switch (pszBuffer[g_iPos]) {
            case '\n':
                g_pszBigBuffer[stIndex ++] = '\\';
                g_pszBigBuffer[stIndex ++] = 'n';
                break;

            case '\t':
                g_pszBigBuffer[stIndex ++] = '\\';
                g_pszBigBuffer[stIndex ++] = 't';
                break;

            case '\"':
                g_pszBigBuffer[stIndex ++] = '\\';
                g_pszBigBuffer[stIndex ++] = '\"';
                break;

            case '\\':
                g_pszBigBuffer[stIndex ++] = '\\';
                g_pszBigBuffer[stIndex ++] = '\\';
                break;

            case '%':
                g_pszBigBuffer[stIndex ++] = '%';
                g_pszBigBuffer[stIndex ++] = '%';
                break;

            default:

                g_pszBigBuffer[stIndex ++] = pszBuffer[g_iPos];
                break;
            }

            g_iPos ++;
        }
    }

    g_pszBigBuffer[stIndex ++] = '\0';

    fprintf (fWrite, "\n");
    WriteIndent();

    fprintf (fWrite, "Write (\"");
    fprintf (fWrite, g_pszBigBuffer);
    fprintf (fWrite, "\", sizeof (\"");
    fprintf (fWrite, g_pszBigBuffer);
    fprintf (fWrite, "\") - 1);");
}