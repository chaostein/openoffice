/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#include <iostream>
#include <fstream>
#include <stdio.h>


#include <string.h>
#include "../support/cmdline.hxx"
#include "cr_metho.hxx"
#include "cr_html.hxx"
#include "cr_index.hxx"
#include "xmlelem.hxx"
#include "xmltree.hxx"
#include "parse.hxx"
#include "../support/syshelp.hxx"
#include "../support/heap.hxx"



int      				Do_IndexCommandLine(
							const CommandLine &		i_rCommandLine );
int      				Do_SingleFileCommandLine(
							const CommandLine &		i_rCommandLine );
void					Create_TypeInfo(
							const char *		  	o_sOutputFile,
							ModuleDescription &	    i_rData );


int
#ifdef WNT
_cdecl
#endif
main( int 		argc,
	  char *    argv[] )
{
	// Variables
	CommandLine				aCommandLine(argc, argv);
	int ret = 0;

	if (! aCommandLine.IsOk())
	{
        std::cerr <<  aCommandLine.ErrorText() << std::endl ;
		return 1;
	}

	if ( aCommandLine.IsIndexCommand() )
		ret = Do_IndexCommandLine(aCommandLine);
	else
		ret = Do_SingleFileCommandLine(aCommandLine);

	return ret;
}


int
Do_SingleFileCommandLine(const CommandLine & i_rCommandLine)
{
	ModuleDescription	aDescr;
	X2CParser			aParser(aDescr);

	// Load file and create Function-file
	bool bLoadResult = aParser.LoadFile(i_rCommandLine.XmlSrcFile());
	if (! bLoadResult)
	{
		std::cerr << "Error: File " << i_rCommandLine.XmlSrcFile() << " could not be loaded." << std::endl;
		return 1;
	}

	if ( strlen(i_rCommandLine.FuncFile()) > 0 )
	{
		Create_AccessMethod( i_rCommandLine.FuncFile(),
							 aParser.PureText() );

    	std::cout << "File "
	    	 << i_rCommandLine.FuncFile()
		     << " with component_getDescriptionFunc() is created now."
             << std::endl;
	}

	// Parse
	aParser.Parse();

	// Produce output
	if ( strlen(i_rCommandLine.HtmlFile()) > 0 )
	{
		HtmlCreator aHtmlCreator( i_rCommandLine.HtmlFile(),
								  aDescr,
								  i_rCommandLine.IdlRootPath() );
		aHtmlCreator.Run();
	}

	if (strlen(i_rCommandLine.TypeInfoFile()) > 0)
	{
		Create_TypeInfo( i_rCommandLine.TypeInfoFile(),
						 aDescr );
	}

	return 0;
};

int
Do_IndexCommandLine(const CommandLine & i_rCommandLine)
{
	// Parsen files:
	List<Simstr>	aFiles;
	Index	        aIndex( i_rCommandLine.OutputDirectory(),
							i_rCommandLine.IdlRootPath(),
							i_rCommandLine.IndexedTags() );

	std::cout << "Gather xml-files ..." << std::endl;
	GatherFileNames( aFiles, i_rCommandLine.XmlSrcDirectory() );

	std::cout << "Create output ..." << std::endl;
	aIndex.GatherData(aFiles);
	aIndex.WriteOutput( i_rCommandLine.IndexOutputFile() );

	std::cout << "... done." << std::endl;

	return 0;
};



//********************      Creating of typeinfo       ********************//


void					Put2StdOut_TypeInfo(
							ModuleDescription &	    i_rData );
void					Put2File_TypeInfo(
                            const char *            i_sOutputFile,
							ModuleDescription &	    i_rData );
void					StreamOut_TypeInfo(
                            std::ostream &               o_rOut,
							ModuleDescription &	    i_rData,
                            const char *            i_sSeparator );




void
Create_TypeInfo( const char *	   		o_sOutputFile,
				 ModuleDescription &	i_rData )
{
    if ( strcmp(o_sOutputFile, "stdout") == 0 )
        Put2StdOut_TypeInfo(i_rData);
    else
        Put2File_TypeInfo(o_sOutputFile,i_rData);

#if 0
	std::ofstream aOut(o_sOutputFile, std::ios::out
#if defined(WNT) || defined(OS2)
											   | std::ios::binary
#endif
	);
	if ( !aOut )
	{
		std::cerr << "Error: " << o_sOutputFile << " could not be created." << std::endl;
		return;
	}

	Heap 	aTypesHeap(12);
	Simstr	sLibPrefix = i_rData.ModuleName();

	// Gather types:
	List< const MultipleTextElement * > aTypes;
	i_rData.Get_Types(aTypes);

	for ( unsigned t = 0; t < aTypes.size(); ++t )
	{
		unsigned i_max = aTypes[t]->Size();
		for ( unsigned  i = 0; i < i_max; ++i )
		{
			aTypesHeap.InsertValue( aTypes[t]->Data(i), "" );
		}  // end for
	}

	// Write types:
	WriteStr( aOut, sLibPrefix );
	WriteStr( aOut, "_XML2CMPTYPES= ");

	HeapItem * pLastHeapTop = 0;
	for ( HeapItem * pHeapTop = aTypesHeap.ReleaseTop(); pHeapTop != 0; pHeapTop = aTypesHeap.ReleaseTop() )
	{
		if (pLastHeapTop != 0)
		{
			if ( 0 == strcmp(pHeapTop->Key(), pLastHeapTop->Key()) )
				continue;
			delete pLastHeapTop;
			// pLastHeapTop = 0;
		}
		pLastHeapTop = pHeapTop;

		WriteStr( aOut, "\t\\\n\t\t" );

		const char * sEnd = strchr( pHeapTop->Key(), ' ' );
		if (sEnd != 0)
		{
			const char * sQuali = strrchr( pHeapTop->Key(), ' ' )+1;
			WriteStr( aOut, sQuali );
			WriteStr( aOut, "." );
			aOut.write( pHeapTop->Key(), sEnd - pHeapTop->Key() );
		}
		else
			WriteStr( aOut, pHeapTop->Key() );
	}	// end for

	if (pLastHeapTop != 0)
	{
		delete pLastHeapTop;
		pLastHeapTop = 0;
	}

	aOut.close();
#endif // 0
}

void
Put2StdOut_TypeInfo( ModuleDescription &	i_rData )
{
    StreamOut_TypeInfo(std::cout, i_rData, " ");
}

void
Put2File_TypeInfo( const char *            i_sOutputFile,
				   ModuleDescription &	   i_rData )
{
	std::ofstream aOut(i_sOutputFile, std::ios::out
#if defined(WNT) || defined(OS2)
											   | std::ios::binary
#endif
	);
	if ( !aOut )
	{
		std::cerr << "Error: " << i_sOutputFile << " could not be created." << std::endl;
		return;
	}

	Simstr	sLibPrefix = i_rData.ModuleName();
	WriteStr( aOut, sLibPrefix );
	WriteStr( aOut, "_XML2CMPTYPES= ");

    StreamOut_TypeInfo(aOut, i_rData, "\t\\\n\t\t");

    aOut.close();
}

void
StreamOut_TypeInfo( std::ostream &               o_rOut,
				    ModuleDescription &	    i_rData,
                    const char *            i_sSeparator )
{
	Heap 	aTypesHeap(12);

	// Gather types:
	List< const MultipleTextElement * > aTypes;
	i_rData.Get_Types(aTypes);

	for ( unsigned t = 0; t < aTypes.size(); ++t )
	{
		unsigned i_max = aTypes[t]->Size();
		for ( unsigned  i = 0; i < i_max; ++i )
		{
			aTypesHeap.InsertValue( aTypes[t]->Data(i), "" );
		}  // end for
	}

	// Write types:
	HeapItem * pLastHeapTop = 0;
	for ( HeapItem * pHeapTop = aTypesHeap.ReleaseTop(); pHeapTop != 0; pHeapTop = aTypesHeap.ReleaseTop() )
	{
		if (pLastHeapTop != 0)
		{
			if ( 0 == strcmp(pHeapTop->Key(), pLastHeapTop->Key()) )
				continue;
			delete pLastHeapTop;
			// pLastHeapTop = 0;
		}
		pLastHeapTop = pHeapTop;

		WriteStr( o_rOut, i_sSeparator );

		const char * sEnd = strchr( pHeapTop->Key(), ' ' );
		if (sEnd != 0)
		{
			const char * sQuali = strrchr( pHeapTop->Key(), ' ' ) + 1;
			WriteStr( o_rOut, sQuali );
			WriteStr( o_rOut, "." );
			o_rOut.write( pHeapTop->Key(), sEnd - pHeapTop->Key() );
		}
		else
			WriteStr( o_rOut, pHeapTop->Key() );
	}	// end for

	if (pLastHeapTop != 0)
	{
		delete pLastHeapTop;
		pLastHeapTop = 0;
	}
}

