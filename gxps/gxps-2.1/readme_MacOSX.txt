gXPS v. 2.1 - 08/18/2010
----------------------------------------------

1) Description
	
	XPS File Conversion from Omicron EIS (VAMAS files) and Presenter (SPECTRA files) to ASCII files. 
	It performs background subtraction and curve fitting. 
	XMGrace (version 5.1.2x) is  is required to plot ASCII files and perform fitting.
	XMGrace (freeware) can be installed through the Linux package manager.
	Curve plotting can be turned off (from the 'settings' menu) if XMGrace is not installed. However
	some features (fitting) may not be available, since they depend on the XMGrace package.
		
	Version 2.0 introduces several new security improvements in the code, making it stronger and less prone to fail. 


2) Installation
   
	Binaries for Mac OSX are no longer provided. Therefore we strongly suggest to recompile xps from source. 
	It has been tested with XCode (based on gcc). 
	After installing gnuplot (version 4.0 or higher) and X11 for Mac OSX, xps works on Mac OSX. 

	Information on installing XMGrace on Mac OSX can be found online. Google for "XMGrace OSX". 
	
	
3) Compiling

	The source has been tested with the following compilers:
	MS VC6, gcc 3.3, 4.0 and 4.1.
	Only standard C++ libraries are used. 

	gcc: Use the provided makefile. To compile the binary type "make". To install it type: "make install"
	to uninstall it, type: "make uninstall". By default optimization flags (-O3) are turned on.


4) Test file

	Several "vms" examples can be found in the folder "example". 
	A test SPECTRA file is also provided ("spectra.1" and "spectra.dat"). 
	"1.vms" consists of a two regions scan.	
	"2.vms" consist of a six region scan (clean Si(111)). 
	"casaxps.vms" is a test file created with CasaXPS.


5) License

	This program (source code and binaries) is free software; 
	you can redistribute it and/or modify it under the terms of the
	GNU General Public License as published by the Free Software 
	Foundation, either in version 3 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You can find a complete copy of the GNU General Public License at:
	http://www.gnu.org/licenses/gpl.txt

6) Contact

	For any suggestion, bug, comments: Nicola Ferralis: feranick@hotmail.com
	An updated version of this program can be found at:

	http://electronsoftware.googlepages.com/xps


