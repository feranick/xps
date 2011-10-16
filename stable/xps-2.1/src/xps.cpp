//*****************************************************************************
//                                                         
//		XPS							 
//														 
//		v. 2.1
//
//		2007-2010 - Nicola Ferralis 					 
//                                                        
//		From VAMAS, SPECTRA data, extract ASCII files. 
//		Applies automatic or manual background subtraction. 
//		Perform multi-peak fit on data.			
//
//		This program (source code and binaries) is free software; 
//		you can redistribute it and/or modify it under the terms of the
//		GNU General Public License as published by the Free Software 
//		Foundation, in version 3 of the License.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//		GNU General Public License for more details.
//
//		You can find a complete copy of the GNU General Public License at:
//		http://www.gnu.org/licenses/gpl.txt
//												             
//**********************************************************************************

//////////////
#if     _MSC_VER
// Use these for MSVC
#include <fstream>
#include <iostream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <windows.h>

#define snprintf sprintf_s

using namespace std; 

#define popen _popen 
#define pclose     _pclose   
#define MSini 1   // 1: save config file in C:/ (MS32 only)    0: save it in the same folder 
#include "direct.h"			

#else
// Use these for gcc
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <math.h>
#include <stdio.h>  
#include <errno.h>
#define MSini 0
using namespace std; 
#endif

#define GPFIFO "./gpio" 

#define PATH_MAX 200
#define CMAXI 200

#define maxNregions 10

#if     _MSC_VER
	#if MSini ==1					
	char inipath[]="C:/";					
	char mainpath[200];	
	#endif
#else
char mainpath[PATH_MAX+1];
#endif

#if     _MSC_VER
int const Max=5000;
#else
int const Max=10000;
#endif

int outtype, infofile, type, smoothT, shirleyT, ecorrT, flag, vplot, steps, npeaks, ctsconv, bconv, xaxis, dsheet;
int Ytypef, KEflag;
int i, j, h, s, f1, f2, f;

int outtype0=1;
int infofile0=1;
int smoothT0=2;
int shirleyT0=2;
int ecorrT0=2;
int vplot0=1;
int ctsconv0=1;
int bconv0=1;
int dsheet0=1;

double X[Max], Y[Max], KE;
double ec = 0.28;   //energy correction factor
double PI = 3.14159265359;
double KEAl = 1486.6;
double KEAl2 = 1486.7;
double KEMg = 1253.6;

char Ytype[20];
char version[] = "2.1 - 20100817";
char developer[] = "Nicola Ferralis - feranick@hotmail.com";

typedef struct coord{
	double x;
	double y;
	} coord;

typedef struct mcoord{
	float x[Max];
	float y[Max];
	} mcoord;

#if     _MSC_VER
char ini[]="xps.cfg";
#else
char ini[]=".xps";
#endif

char datextension[]=".dat";
char bsextension[]="_bs";
char infoextension[]=".info.txt";
char regextV[]=".vr";
char regextS[]=".sr";

int extract(char* name, char *nameOUT, int infofile, int ext);
int extractSPECTRA(char *nameIN, char * nameOUT, int infofile, int ext);
void BackSub(char* name, char* nameout);
void BackSub2(char* name, char* nameout);

int Smooth();
int Shirley();
void Ecorr();
int norm();
int XRange();
int DnD(char *name);
int savesheet (char *nameIN, int n, int d);

coord Maximum();

void IniPlot(char* name);
mcoord IniPlotFit(char *name);
void Plot1(char* name);
void Plot2(char* name, char* nameout);
void PlotSettings();
void ClosePlot();

int Fit(char* name);
int Peak(char*name, char const *cytype, int peaktype);

int ReadFile(char* name);
int SaveFile(char *nameout);

void SleepP(int time);
int ReadKey();
float ReadKeyF();
void PreferencePanel();
void clearscreen();



FILE *command, *gpin;


int main(int argc, char *argv[])
{	
	#if     _MSC_VER
		#if MSini ==1
		_chdir(_getcwd(mainpath, 200));  
		_chdir(inipath);					
		#endif
	#else
	int rc=0;
	char *pt;	
	pt=getcwd(mainpath, PATH_MAX+1);	
	rc = chdir(getenv("HOME"));	
	#endif

	#if     _MSC_VER
	ifstream	infile(ini);   //MSVC
	#else
	ifstream	infile(ini);    		  //gcc
	#endif

	if(!infile)			// the first time the program runs, it saves the settings 
		{
		ofstream outfile2(ini); 
		outfile2<<outtype0<<"\n"<<infofile0<<"\n"<<smoothT0<<"\n"<<shirleyT0<<"\n"<<ecorrT0<<"\n"<<vplot0<<"\n"<<ctsconv0<<"\n"<<bconv0<<"\n"<<dsheet0<<"\n";
		outtype=outtype0;
		infofile=infofile0;
		smoothT=smoothT0;
		shirleyT=shirleyT0;
		ecorrT=ecorrT0;
		vplot=vplot0;
		ctsconv=ctsconv0;
		bconv=bconv0;
		dsheet=dsheet0;
		}

	infile>>outtype>>infofile>>smoothT>>shirleyT>>ecorrT>>vplot>>ctsconv>>bconv>>dsheet;   // if the config file exists, it reads the settings from it. 
	if(outtype==0 || infofile==0 || smoothT==0 || shirleyT ==0 || ecorrT==0 || vplot==0 || ctsconv==0 || bconv==0 || dsheet==0)
		{ofstream outfile2(ini); 
		outfile2<<outtype0<<"\n"<<infofile0<<"\n"<<smoothT0<<"\n"<<shirleyT0<<"\n"<<ecorrT0<<"\n"<<vplot0<<"\n"<<ctsconv0<<"\n"<<bconv0<<"\n"<<dsheet0<<"\n";
		outtype=outtype0;
		infofile=infofile0;
		smoothT=smoothT0;
		shirleyT=shirleyT0;
		ecorrT=ecorrT0;
		vplot=vplot0;
		ctsconv=ctsconv0;
		bconv=bconv0;
		dsheet=dsheet0;}

	infile.close();

	#if     _MSC_VER
		#if MSini ==1
		_chdir(mainpath);			
		#endif
	#else
		rc = chdir(mainpath);
	#endif	
	
	if(argc<2)
	{
//**************User interface******************************************************
	cout<< "______________________________________________________________________________";
	cout<< "\nXPS: Files Conversion, Background Subtraction and Multi-Peak Fitting \n";
	cout << "\n0) Exit";
	cout<<"\n1) Open VAMAS files (EIS)\t4) Convert from Presenter \t7) File Browser";
	cout<<"\n2) Background subtraction\t5) Save Datasheet\t\t8) User Guide";
	cout<<"\n3) Peak fitting\t\t\t6) Preferences\t\t\t9) About\n";

//**********************************************************************************

// Menu option #0: "Exit"

	type=0;
	type=ReadKey();
	clearscreen();

	}
	
if(argc>=2)
	{for(i=1; i<argc; i++)
		{
		cout<<"Converting: "<<argv[i]<<"\n";
		DnD(argv[i]);
		}
	

	return 0; }

	if((type!=1) & (type!=2) & (type!=3) & (type!=4) & (type!=5) & (type!=6) & (type!=7) & (type!=8) & (type!=9) )
		{cout<<"Bye Bye\n";
		return 0;}


// Menu option #7: "File browser" - perform an operation similar to 'ls' in UNIX or 'dir' in Win32

	if(type==7)
		{
		#if     _MSC_VER
		command = popen("dir","w");
		#else
		command = popen("pwd","w");
		command = popen("ls","w");
		#endif
		pclose(command);
		return main(0,0);}


// Menu option #8: "User Guide"

	if(type==8)
		{
		cout<<"User guide\n";
		cout<<" 1. Convert VAMAS files saved from Omicron EIS into individual ASCII files \n    for each scan region, with the possibility to plot each region and \n    subtract the background.\n";
		cout<<" 2. Automatic or manual background subtraction from previously converted curves \n    (Opt. 1): \n";
		cout<<"\ta. 5-point smoothing; \n";
		cout<<"\tb. Shirley correction; \n";
		cout<<"\tc. Transmission function of the analyzer: energy-dependence correction \t\t   (I*KineticE^"<<ec<<").\n";
		cout<<"\td. X-axis normalization (custom energy step).\n";
		cout<<"\te. Select a new range for the x-axis.\n";
		cout<<" 3. \"1 to 5\" peak fitting (using Lorentzian or Gaussian distributions).\n"; 
		cout<<"    This option is not available if plotting is turned off (in the \n    \"Preferences\" menu) or if GnuPlot is not installed.\n"; 
		cout<<" 4. Convert SPECTRA files as output from Omicron Presenter (background already \n    subtracted with Presenter) into individual ASCII files for each scan region.\n";

		return main(0,0);}

// Menu option #9: "About"

	if(type==9)
		{cout<<"\n XPS: File Conversion, Background Subtraction and Milti-Peak Fitting\n v. "<<version;
		cout<<"\n\n GnuPlot (version 4.0 or higher) is required to plot ASCII files.\n";
		cout<<" GnuPlot (freeware) can be downloaded at http://www.gnuplot.info \n\n";
		cout<<" An updated version of this program can be found at:\n http://electronsoftware.googlepages.com/xps \n\n";
		cout<<" Suggestions and bugs:  "<<developer<<"\n\n";
		cout<<" This program and source code are released under the: \n Gnu Public License v. 3.0.\n http://www.gnu.org/licenses/gpl.txt\n\n";
		return main(0,0);}

// Menu option #6: "Preferences" 

	if (type==6)
		{	
		int *tmp, *preftype;
		tmp=(int *)malloc(sizeof(int));
		preftype=(int *)malloc(sizeof(int));
		*preftype=1;
		
		while(*preftype!=0)
			{PreferencePanel();
			cout<<"  Type: (1 to 9: change individual settings)\n\t(10: restore default)   (0: exit)  ";
			*preftype=ReadKey();

			if (*preftype==1)
				{cout<<"\n\nWith this option you can specify the name of the output file:";
				cout<<"\n 1) Automatic: output is saved in a file with extension \".dat\".;";
				cout<<"\n 2) Manual: the program asks for the name of the output file.";		
				cout<<"\n\n (1: Automatic)  (2: Manual)  (other: no change) ";
				*tmp=outtype;
				outtype=ReadKey();
				if (outtype !=1 && outtype !=2)
					{cout<<" Value not updated\n";
					outtype=*tmp;}
				}
			
			if (*preftype==2)
				{cout<<"\nWith this option you can save the acquisition information in a separate file:";
				cout<<"\n 1) Saved: info saved in a file with extension \".info.txt\".;";		
				cout<<"\n 2) Not saved: the program does not save the info.";
				cout<<"\n\n (1: Saved)  (2: Not saved)  (other: no change) ";
				*tmp=infofile;
				infofile=ReadKey();
				if (infofile !=1 && infofile !=2)
					{cout<<" Value not changed\n";
					infofile=*tmp;}
				}		
		
			if (*preftype==3)
				{
				cout<<"\nAutomatic conversion of the intesity label to \"counts/s\". ";
				cout<<"\n (1: Yes)  (2: No)  (other: no change) ";	
				*tmp=ctsconv;
				ctsconv=ReadKey();
				if(ctsconv!=1 && ctsconv !=2)
					{cout<<" Value not changed\n";
					ctsconv=*tmp;}
				}
			
			if (*preftype==4)
				{
				cout<<"\nAutomatic conversion of the x axis to \"binding energy\". ";
				cout<<"\n (1: Yes)  (2: No)  (other: no change) ";	
				*tmp=bconv;
				bconv=ReadKey();
				if(bconv!=1 && bconv !=2)
					{cout<<" Value not changed\n";
					bconv=*tmp;}
				}

			if (*preftype==5)
				{
				cout<<"\n5-point smooth. ";			
				cout<<"\n (1: No)  (2: All but first)  (3: All)  (other: no change) ";
				*tmp=smoothT;		
				smoothT=ReadKey();
				if(smoothT!=1 && smoothT !=2 && smoothT!=3)
					{cout<<" Value not changed\n";
					smoothT=*tmp;}
				}

			if (*preftype==6)
				{cout<<"\nShirley correction. ";				
				cout<<"\n (1: No)  (2: All but first)  (3: All)  (other: no change) ";		
				*tmp=shirleyT;
				shirleyT=ReadKey();
				if(shirleyT!=1 && shirleyT !=2 && shirleyT!=3)
					{cout<<" Value not changed\n";
					shirleyT=*tmp;}
				}
		
			if (*preftype==7)
				{cout<<"\nTransmission function of the analyzer: energy-dependence correction. ";				
				cout<<"\n (1: No)  (2: All but first)  (3: All)  (other: no change) ";	
				*tmp=ecorrT;
				ecorrT=ReadKey();
				if(ecorrT!=1 && ecorrT !=2 && ecorrT!=3)
					{cout<<" Value not changed\n";
					ecorrT=*tmp;}
				}

			if (*preftype==8)
				{cout<<"\nSave regions in one ASCII datasheet ";				
				cout<<"\n (1: Yes)  (2: No)  (other: no change) ";	
				*tmp=dsheet;
				dsheet=ReadKey();
				if(dsheet!=1 && dsheet !=2)
					{cout<<" Value not changed\n";
					dsheet=*tmp;}
				}
		
			if (*preftype==9)
				{cout<<"\nDo you want the spectra to be plotted? ";
				cout<<"\n (GnuPlot needs to be installed to take advantage of this feature)";
				
				cout<<"\n (1: Plotted)  (2: NOT plotted)  (other: no change) ";
				*tmp=vplot;
				vplot=ReadKey();
				if(vplot==2)
					{cout<<" Some features (fitting) may not be available if plotting is not enabled. \n";}
				if(vplot!=1 && vplot!=2)
					{cout<<" Value not changed\n\n";
					vplot=*tmp;}
				}

			if (*preftype==10)
				{outtype=outtype0;
				infofile=infofile0;
				smoothT=smoothT0;
				shirleyT=shirleyT0;
				ecorrT=ecorrT0;
				vplot=vplot0;
				ctsconv=ctsconv0;
				bconv=bconv0;
				dsheet=dsheet0;
				cout<<"\n Default parameters succesfully restored!\n\n";}
			}
		free(tmp);	
		free(preftype);	

		#if     _MSC_VER
			#if MSini ==1
			_chdir(inipath);			
			#endif
		#else
		rc = chdir(getenv("HOME"));
		#endif

		
		ofstream outfile2(ini);
		outfile2<<outtype<<"\n"<<infofile<<"\n"<<smoothT<<"\n"<<shirleyT<<"\n"<<ecorrT<<"\n"<<vplot<<"\n"<<ctsconv<<"\n"<<bconv<<"\n"<<dsheet<<"\n";
		
		#if     _MSC_VER
			#if MSini ==1
			_chdir(mainpath);			
			#endif
		#else
		rc = chdir(mainpath);
		#endif

		
		outfile2.close();
		return main(0,0);
		}


// VMS and SPECTRA Converter

	if (type==1 || type==4)
	{	KEflag=0;
		
		if(type==1)
			{cout<<"name Vamas file (WITHOUT the \".vms\" extension): ";}			
		
		if(type==4)
			{cout<<"name SPECTRA file (WITHOUT the extension, either \".dat\" or \".1\"): ";}

		char *nameIN, *nameOUT;
		nameIN=(char *)malloc(sizeof(char[CMAXI]));
		nameOUT=(char *)malloc(sizeof(char[CMAXI]));
		cin>> nameIN;	
		//////////////////////////////// fix nameout	
		if (outtype!=1)
			{cout<<"name output file  (WITHOUT extension): ";
			cin>> nameOUT;

		}
		else
			{snprintf(nameOUT, CMAXI, "%s", nameIN);}

		if(type==1)
			{flag=0;
			extract(nameIN, nameOUT, infofile, 0);
			
			if(flag==0)
				{
				cout<<"Conversion completed.\n";
			
				if(vplot==1)
					{							
					f1=1;
					while(f1!=0)
						{						
						cout<<"\nPlot and subtract background of region number (1 to "<<s<<", 0: EXIT): ";
						f1=ReadKey();
						if(f1>s || f1==0)
							{break;}
									
						char *nametmp, *nametmp2;
						nametmp=(char *)malloc(sizeof(char[CMAXI]));
						nametmp2=(char *)malloc(sizeof(char[CMAXI]));
						
						snprintf(nametmp, CMAXI, "%s%s%d%s"  , nameOUT, regextV, f1, datextension);
						snprintf(nametmp2, CMAXI, "%s%s%d%s%s"  , nameOUT, regextV, f1, bsextension, datextension);
						
						BackSub2(nametmp, nametmp2);
						free(nametmp);
						free(nametmp2);						
						}
					}
				else
					{}
				flag=0;
				}
			
			}
			
		if(type==4)
			{flag=0;
			
			extractSPECTRA(nameIN, nameOUT, infofile, 0);
					
			if(flag==0)
				{cout<<"\nConversion completed.\n\n";}
			}

		if(flag==0)		
			{
			if(dsheet==1)			
				{
				if(type==1)			
					{savesheet(nameIN, s, 1);}
				if(type==4)
					{savesheet(nameIN, s, 2);}
				}	
			}
			
	free(nameIN);
	free(nameOUT);
	return main(0,0);
	}

//  Background subtraction

if (type==2)
	{ int *bstype;
	bstype=(int *)malloc(sizeof(int));
	cout<<"(1: Automatic)   (2: Manual)   (Other: Exit)   ";
	*bstype=ReadKey();
	char difextension[]=".vms";
	char *nameIN, *nameOUT;
	nameIN=(char *)malloc(sizeof(char[CMAXI]));
	nameOUT=(char *)malloc(sizeof(char[CMAXI]));
	if(*bstype==1)
		{	
		flag=1;
		cout<<"The file must have the name formatted as: \nbasename"<<regextV<<"#"<<difextension<<"\nwhere # is the increasing, progressive number of the scan.\n";
		cout<<"Example of progressive files: data"<<regextV<<"1."<<difextension<<", data"<<regextV<<"2"<<difextension<<", ...\n";
		cout<<"\nFile base name (without \""<<regextV<<"\"): ";
		cin>> nameIN;
		cout<<"Number first file: ";
		f1=ReadKey();
		cout<<"Number final file: ";
		f2=ReadKey();

		for (f=f1; f<=f2; f++)
			{
			char* name;
			name=(char *)malloc(sizeof(char[CMAXI]));
			snprintf(name, CMAXI, "%s%s%d%s" ,nameIN, regextV,f, datextension); 
			snprintf(nameOUT, CMAXI, "%s%s%d%s%s" ,nameIN, regextV,f, bsextension, datextension); 
			
			
			BackSub(name, nameOUT);
			free(name);

			if(flag==0)
				{break;}
			}
		}

	if (*bstype==2)
	{	
		flag=1;
		cout<<"name file (WITHOUT the \".dat\" extension): ";

		char* name;
		name=(char *)malloc(sizeof(char[CMAXI]));

		cin>> nameIN;	
		snprintf(name, CMAXI,  "%s%s", nameIN, datextension);		
		snprintf(nameOUT, CMAXI, "%s%s%s" , nameIN, bsextension, datextension);		
		BackSub2(name, nameOUT);
		free(name);
		}

	if(*bstype!=1 && *bstype!=2)
		{return main(0,0);}
		
	if(flag!=0)
		{if(*bstype==2)
			{cout<<"\nThe output file is: "<<nameOUT;}
		cout<<"\nManual background subtraction completed\n\n";	}
	
	free(bstype);
	free(nameIN);
	free(nameOUT);	
	return main(0,0);
	
	}

// Option menu #3: Curve fitting

if(type==3)
	{
	if(vplot!=1)
		{cout<<"\nPlease enable plotting (in the settings menu) to perform fitting. \n";
		return main(0,0);}
	
	cout<<"name file (WITHOUT the \".dat\" extension): ";
	
	char* name, *nametmp;
	name=(char *)malloc(sizeof(char[CMAXI]));
	nametmp=(char *)malloc(sizeof(char[CMAXI]));

	cin>> nametmp;	
		
	snprintf(name, CMAXI, "%s%s", nametmp, datextension);
	Fit(name);
	free(name);
	free(nametmp);	
	return main(0,0);
	}

// Menu option #5: Save individual regions (after processing) into one datasheet.

if(type==5)
		{
		char *nameIN;				
		nameIN=(char *)malloc(sizeof(char[CMAXI]));
		cin>>nameIN;
		int *regions, *datatype;
		regions=(int *)malloc(sizeof(int));
		datatype=(int *)malloc(sizeof(int));
		cout<<" Name the base file (no \""<<regextV<<"*.dat\" or \""<<regextS<<"*.dat\"): ";
		cin>>nameIN;
		cout<<" How many regions: ";
		*regions=ReadKey();
	cout<<" Data from: 1. VMS  or  2. SPECTRA:  ";
		*datatype=ReadKey();
		if(*datatype==1)			
			{savesheet(nameIN, *regions, 1);}
		if(*datatype==2)			
			{savesheet(nameIN, *regions, 2);}
		free(regions);
		free(datatype);
		free(nameIN);
		return main(0,0);}

return main(0,0);
}

//*********************************************************************
// Drag and drop conversion

int DnD(char *name)
	{
	#if     _MSC_VER
	ifstream infile(name);		
	#else
	ifstream infile(name);		
	#endif
	if(!infile)
		{cout<<"\n file \""<< name<<"\" not found\n";
		return 0;}

	char *test, *tmp;
	int sptype=0;
	
	test=(char *)malloc(sizeof(char[CMAXI]));
	tmp=(char *)malloc(sizeof(char[CMAXI]));
	infile>>test;

	infile.close();
	if(strcmp(test,"VAMAS")==0)
	{	snprintf(tmp, CMAXI, "%s", name);
		tmp[strlen(tmp) - 4] = '\0';
		extract(name, tmp, infofile, 1);
		if(dsheet==1)
			{savesheet(tmp, s, 1);}
		}

	if(strcmp(test,"*M*")==0)
		{snprintf(tmp, CMAXI, "%s", name);
		
		for(unsigned int i=0; i<strlen(tmp); i++)
			{
			if (strcmp(&tmp[i],".1")==0)
				{sptype=1;}
			}		

		if (sptype==1)
			{tmp[strlen(tmp) - 2] = '\0';}
		else
			{tmp[strlen(tmp) - 4] = '\0';}

		extractSPECTRA(tmp, tmp, infofile, 0);
		if(dsheet==1)
			{savesheet(tmp, s, 2);}
		}
	free(test);
	free(tmp);

	 return 0;
	}

//*********************************************************************
// Extract VAMAS (VMS) data from EIS or CasaXPS: Option 1

int extract(char *name, char *nameOUT, int infofile, int ext)
	{
	char* namein, *nameout;
	namein=(char *)malloc(sizeof(char[CMAXI]));
	nameout=(char *)malloc(sizeof(char[CMAXI]));
	
	if(ext==0)
		{snprintf(namein, CMAXI, "%s%s", name, ".vms");}
	else
		{snprintf(namein, CMAXI, "%s", name);}
 
	#if     _MSC_VER
	ifstream infile(namein);		
	#else
	ifstream infile(namein);		
	#endif

	double sweepStart, sweepEnd, timePerStep, Nsweeps, evPerStep;
	char Year[20], Month[20], Day[20], test[20];

	int casa = 0;	
	int comm = 0;	
	xaxis = 0;
	
	
	if(!infile)
		{cout<<"\n file \""<< namein<<"\" not found\n";
		flag=1;
		return 0;}
	
	while(strcmp(test,"REGULAR")!=0)
		{ 
		infile>>test;
		if(infile.eof())
			{cout<<" \n There is a problem reading the file \""<< name<<"\". Please email it to the developer: \n "<<developer<<" \n for debugging.\n\n";
			flag=1;			
			return 0;
			}
		if(strcmp(test,"Casa")==0)
			casa=1;	
		}
	
	if (casa ==1)
		{
		for(i=0; i<10; i++)
			{infile>>test;}}
	else
		{for(i=0; i<7; i++)
			{infile>>test;}
			}
	s=0;	
	int g=0;
	int g0=0;
	while(!infile.eof())
		{g0=g;
		infile>>Year;
		if(strcmp(Year, "None")==0)
			{infile>>Year;}
			
		if(strcmp(Year, "end")==0)
			{break;}

		infile>>Month>>Day;
		string line;
   		 char (*comment)[200]= new char[20][200];


		if(casa==1)
			{
			while(strcmp(test,"XPS")!=0 )
				{
				infile>>test;	
				if(strcmp(test, "CASA")==0)
				{getline(infile, line);
					snprintf(comment[g], 300, "%s",  line.c_str());

					#if     _MSC_VER
					cout<<comment[g]<<"\n";	
					#endif
					g++;
					}
				}
				
		
			for(i=0; i<2; i++)
				{infile>>test;}	
			infile>>KE;
			KEflag=1;
					
			for(i=0; i<16; i++)
				{infile>>test;}

			}

		if(casa==0)
			{for(i=0; i<5; i++)
				{infile>>test;}
			infile>>test;
			if(strcmp(test,"XPS")!=0)
				{getline(infile, line);
				snprintf(comment[g], 300, "%s %s", test, line.c_str());
				infile>>test;
				comm=1;
				g++;}
			
			infile>>KE;
			KEflag=1;
					
			for(i=0; i<15; i++)
				{infile>>test;}}


		
		infile>>test;
		if(strcmp(test,"binding")==0)
			{xaxis=0;}
		if(strcmp(test,"kinetic")==0)
			{xaxis=1;}
		
		for(i=0; i<2; i++)
			{infile>>test;}

		infile>>sweepStart>>evPerStep;
	
		for(i=0; i<3; i++)
			{infile>>test;}

		if(strcmp(test, "rate")==0)
			{infile>>test;
			Ytypef=2;}
		else
			{Ytypef=1;}			
			
		
		snprintf(Ytype, 20, "%s", test);
		infile>>test>>test>>timePerStep>>Nsweeps;
		
		for(i=0; i<5; i++)
			{infile>>test;}

		infile>>steps>>test>>test;

		sweepEnd=sweepStart+(steps-1)*evPerStep;
		s++;

		snprintf(nameout, CMAXI, "%s%s%d%s" , nameOUT, regextV, s, datextension);
		
		ofstream outfile(nameout);

		
		
		if(infofile==1)
			{char *nameinfo;
			nameinfo=(char *)malloc(sizeof(char[CMAXI]));
			snprintf(nameinfo, CMAXI, "%s%s", nameOUT, infoextension);
			
			ofstream outinfo(nameinfo, ios::app);
			outinfo<<"-----------------------------------------------------------\n";
			outinfo<<"Name file: "<<namein<<"\n";
			outinfo<<"Acquisition date: "<<Month<<"-"<<Day<<"-"<<Year;
			outinfo<<"\nSource: ";
			if(KE==KEAl || KE==KEAl2 )
				outinfo<<"Aluminum ("<<KE<<" eV)";
			else
				outinfo<<"Magnesium ("<<KE<<"eV)";
			outinfo<<"\nSweep Start= "<< sweepStart<< " eV\nSweep End= "<<sweepEnd<<" eV";
			outinfo<<"\neV per Step= "<<evPerStep<<" eV\nTime per Step= "<< timePerStep<<" s\nNumber of sweeps: "<<Nsweeps;
			if(casa==1 && g0!=g)
				{outinfo<<"\n-----------\nCasaXPS:";
				
				for(i=g0; i<g; i++)
					{outinfo<<"\n"<<comment[i];
					#if     _MSC_VER
					outinfo<<"\n";	
					#endif
				}
				outinfo<<"\n-----------";}

			if(comm==1)
				{for(i=g0; i<g; i++)
					{outinfo<<"\nNote: "<<comment[i];
					#if     _MSC_VER
					outinfo<<"\n";	
					#endif
				}}
			
			if(xaxis==1)
				{outinfo<<"\nLabel X asis in the original VMS file: kinetic energy (\"eV\")";
				if(bconv==1)
					{outinfo<<"\nLabel X asis in saved file: binding energy (\"eV\")";}
				if(bconv==2)
					{outinfo<<"\nLabel X asis in saved file: kinetic energy (\"eV\")";}
				}

			if(xaxis==0)
				{outinfo<<"\nLabel X asis in the original VMS file: kinetic energy (\"eV\")";}

			if(Ytypef==1)
				{outinfo<<"\nIntensity label in the original VMS file: counts (\"d\")";
				if(ctsconv==1)
						{outinfo<<"\nIntensity label in the saved file: count rate (\"c/s\").";}
				if(ctsconv==2)
						{outinfo<<"\nIntensity label in the saved file: counts (\"d\").";}
					}
			if(Ytypef==2)
				{outinfo<<"\nIntensity label: count rate ("<<Ytype<<")";}
			outinfo<<"\nCurve saved in: "<<nameout<<"\n";
			outinfo<<"-----------------------------------------------------------\n";
			outinfo.close();
			free(nameinfo);
			}
			
		for(j=0; j<steps; j++)
			{infile>>Y[j];
				
				if(Ytypef==1 && ctsconv==2)
					{}
				else
					{Y[j]=Y[j]/(timePerStep*Nsweeps);}
									
				if(xaxis==1 && bconv==1)
					{X[j]=	KE-(sweepStart+j*(evPerStep));}
				else
					{X[j]=	sweepStart+j*(evPerStep);}
			}
		for(j=0; j<steps; j++)
			{outfile<<X[j]<<"\t"<<Y[j]<<"\n";}

		outfile.close();

		
		}
	infile.close();	
		
	cout<<"\nSource: ";
				if(KE==KEAl || KE==KEAl2)
					cout<<"Aluminum ("<<KE<<" eV).";
				else
					cout<<"Magnesium ("<<KE<<"eV).";	

	if(xaxis==1)
		{cout<<"\n\nLabel X asis in the original VMS file: kinetic energy (\"eV\")";
		if(bconv==1)
			{cout<<"\nLabel X asis in saved file: binding energy (\"eV\")";}
		if(bconv==2)
			{cout<<"\nLabel X asis in saved file: kinetic energy (\"eV\")";}
		}

		if(xaxis==0)
			{cout<<"\n\nLabel X asis in the original VMS file: kinetic energy (\"eV\")";}

	if(Ytypef==1)
		{cout<<"\n\nIntensity label in the original VMS file: counts (\"c\")";
		if(ctsconv==1)
			{cout<<"\nIntensity label in the saved ASCII files: count rate (\"c/s\").\n\n";}
		if(ctsconv==2)
			{cout<<"\nIntensity label in the saved ASCII files: counts (\"c\").\n\n";}
		}
	if(Ytypef==2)
		{cout<<"\n\nIntensity label in the original VMS file and \nthe saved ASCII files: count rate (\"c/s\").\n\n";}

	free(nameout);
	free(namein);	
	return 0;
}

//*********************************************************************
// Extract SPECTRA data from Presenter: Option 4

int extractSPECTRA(char* nameIN, char * nameOUT, int infofile, int ext)
	{
	static double sweepStart, sweepEnd, timePerStep, Nsweeps, step, evPerStep;
	static char test[20];
	char spectraext[]=".dat";
	int spflag, peaknum, dattype, overflow;
	
	char* namein, *nameout;
	namein=(char *)malloc(sizeof(char[CMAXI]));
	nameout=(char *)malloc(sizeof(char[CMAXI]));
	
	if(ext==0)
		{snprintf(namein, CMAXI, "%s%s", nameIN, ".1");}
	else
		{snprintf(namein, CMAXI, "%s", nameIN);}

	cout<<nameIN<<"\t"<<nameOUT<<"\n";

	spflag =0;
	
	#if     _MSC_VER
	ifstream infile0(namein);		
	#else
	ifstream infile0(namein);		
	#endif
 	
	if(!infile0)
		{snprintf(namein, CMAXI, "%s%s", nameIN, spectraext);
		spflag=1;		
		}
	infile0.close();
	
	if(spflag==1)
	{
		#if     _MSC_VER
		ifstream infile1(namein);		
		#else
		ifstream infile1(namein);		
		#endif

		if(!infile1)
			{	
			cout<<"\n file \""<< namein<<"\" not found\n";
			flag=1;
			spflag=2;}
		infile1.close();
	}	
	
	if(spflag==2)
		{return 0;}

	#if     _MSC_VER
	ifstream infile(namein);		
	#else
	ifstream infile(namein);		
	#endif	

	infile>>test;
	s=0;
	overflow=0;
		while(!infile.eof())
		{	overflow++;
			if(overflow>12)
				{cout<<"\n File \""<<nameIN<<"\" is not fully recognized. Abort\n";
				flag=1;
				break;}
		
			if(infile.eof())
				{break;}
			infile>>sweepEnd;
			infile>>sweepStart>>evPerStep;
						
			infile>>Nsweeps>>timePerStep>>step;
			infile>>test>>test;
			s++;

			snprintf(nameout, CMAXI, "%s%s%d%s" , nameOUT, regextS, s, datextension);			
			
			ofstream outfile(nameout);		

						
			if(strcmp(test, "Fit")!=0)
				{dattype=1;}
			
			else
				{dattype=2;
				infile>>test>>test;
				if(strcmp(test, "Peak")==0)
					{dattype=3;
						infile>>peaknum;}
				infile>>test;
				}
			

			Y[0]=atof(test);				
			X[0]=sweepEnd;	

			if(dattype==1)
					{outfile<<X[0]<<"\t"<<(Y[0]/(timePerStep*Nsweeps))<<"\n";}
			if(dattype==2 || dattype==3)
					{outfile<<X[0]<<"\t"<<Y[0]<<"\n";}
	
			for(j=1; j<step; j++)
				{infile>>Y[j];
				X[j]= sweepEnd-j*fabs(evPerStep);
			if(dattype==1)
					{outfile<<X[j]<<"\t"<<(Y[j]/(timePerStep*Nsweeps))<<"\n";}
				if(dattype==2 || dattype==3)
					{outfile<<X[j]<<"\t"<<Y[j]<<"\n";}
				}

			outfile.close();
				
				if(infofile==1)
				{char *nameinfo;
				nameinfo=(char *)malloc(sizeof(char[CMAXI]));
				snprintf(nameinfo, CMAXI, "%s%s", nameOUT, infoextension);				
				ofstream outinfo(nameinfo, ios::app);
				outinfo<<"--------------------------------------\n";
				outinfo<<"Name file: "<<nameIN<<"\n";
				if(dattype==2)
					outinfo<<"\nFit result Envelope";
				if(dattype==3)
					outinfo<<"\nFit result Peak "<<peaknum;
				outinfo<<"\nSweep Start= "<< sweepStart<< " eV\nSweep End= "<<sweepEnd<<" eV";
				outinfo<<"\neV per Step= "<<evPerStep<<"eV\nTime per Step= ";
				outinfo<< timePerStep<<" s\nNumber of sweeps: "<<Nsweeps<<"\nNumber of steps: "<<step;
				outinfo<<"\n\nIntensity saved in: counts per sec";
				outinfo<<"\nCurve saved in: "<<nameout<<"\n";
				outinfo<<"--------------------------------------\n";
				outinfo.close();
				free(nameinfo);
				}
		} 
	infile.close();	
	
	return 0;
}


//*********************************************************************
// Background subtraction on ASCII files - Automatic: Option 3

void BackSub(char* name, char* nameout)
	{ReadFile(name);
	Smooth();
	Shirley();
	Ecorr();
	SaveFile(nameout);
	}


//*********************************************************************
// Background subtraction on ASCII files - Manual: Option 4

void BackSub2(char* name, char* nameout)
	{	int *sbtype;
		sbtype = (int *)malloc(sizeof(int));

		ReadFile(name);
		if(flag!=0)
		{		
		IniPlot(name);

		*sbtype=5;
		while(*sbtype==1 || *sbtype==2 || *sbtype==3 || *sbtype==4 || *sbtype==5 || *sbtype==8 || *sbtype==9)
			{ 
			
			cout<<"\n 1: Smooth  \t2: Shirley  \t3: Energy correction \t    4: Norm. X-axis  \n 5: New range  \t8: Restore \t9: Auto back. subtraction   0: END  ";
			*sbtype=ReadKey(); 
			    	
			if(*sbtype==1)
				Smooth();
			if(*sbtype==2)
				Shirley();
			if(*sbtype==3)
				Ecorr();
			if(*sbtype==4)
				norm();
			if(*sbtype==5)				
				XRange();
			if(*sbtype==8)
				ReadFile(name);
			if(*sbtype==9)				
				{Smooth();
				Shirley();
				Ecorr();}
			
			SaveFile(nameout);
			SleepP(100);
			Plot2(name, nameout);
			}

		SaveFile(nameout);
		SleepP(300);
		if(vplot==1)		
			{fprintf(command,"replot \n");}
		ClosePlot();
		}
	free(sbtype);
	}
			


//*********************************************************************
//Routines for Background subtraction

// Smooth

int Smooth()      //5-point smooth
	{if(smoothT==2 || (smoothT==1 && s!=1))
		{
		double *Ytemp[Max]; 
		for (int i=0; i<Max; i++)
			{Ytemp[i] = (double *)malloc(sizeof(double));
			if (Ytemp[i] == 0)
				{printf("ERROR: Out of memory\n");
				return 0;}
			}
		
		for(j=0; j<steps; j++)
			{if(j<=2 || j>=steps-2)
				{*Ytemp[j]=Y[j];}
			else
				{*Ytemp[j]=(-3*Y[j-2]+12*Y[j-1]+17*Y[j]+ 12*Y[j+1]-3*Y[j+2])/35.0;}
			}
		
		for(j=0; j<steps; j++)
			{Y[j]=*Ytemp[j];}
	
		for (int k=0; k<Max; k++)
			{free(Ytemp[k]);}	
		}
	return 0;	
	}


// Shirley Background subtraction

int Shirley()      
	{int l, k, i;	
	double Sum, evPerStep; 
	double *A1[Max], *A2[Max], *S[Max];
	
	for (l=0; l<Max; l++)
		{A1[l] = (double *)malloc(sizeof(double));
	A2[l] = (double *)malloc(sizeof(double));
	S[l] = (double *)malloc(sizeof(double));
	if ((A1[l] == 0) || (A2[l] == 0 || S[l] == 0))
		{printf("ERROR: Out of memory\n");
		return 0; }
		}
	
	evPerStep=fabs(X[1]-X[0]);
	for(k=0; k<steps; k++)
		{*S[k]=0.0;}

	for(i=0; i<steps; i++)
		{
		Sum = 0;
		for(j=i; j<steps; j++)
			{Sum = Sum +(Y[j])*evPerStep;}
			*A1[i]=Sum;
			Sum = 0;
			for(j=0; j<i; j++)
				{Sum = Sum+(Y[j])*evPerStep;}
			*A2[i]=Sum;
			*S[i]=Y[i]-Y[0]+(fabs(Y[steps-1]-Y[0]))*(*A2[i]/(*A1[i]+*A2[i]));
		}
			
	for(k=0; k<steps; k++)
		{Y[k]=*S[k];}
	
	for (l=0; l<Max; l++)
		{free(A1[l]);
		free(A2[l]);
		free(S[l]);}
	
	return 0;
}


// Transmission function of the analyzer: energy-dependence correction 
void Ecorr()     
	{static coord max;
	if(KEflag==0)
		{int *tmp;
		tmp=(int *)malloc(sizeof(int));
		cout<<"\nWhat type of source (1: Al-1486.6 eV,  2: Mg-1253.6): ";
		*tmp=ReadKey();		
		if(*tmp==1)		
			{KE=KEAl;}
		if(*tmp==2)		
			{KE=KEMg;}
		free(tmp);
		KEflag=1;}
	max=Maximum();

	for(j=0; j<steps; j++)
		{Y[j]=(Y[j]/(max.y*pow((KE-max.x), ec)/max.y))*pow((KE-X[j]), ec);}		
	}

// Additional routines

// Maximum finder
coord Maximum()
	{
	static coord max;

	for(i=0; i<steps;i++)
		{if(Y[i]>=max.y)
			{max.y= Y[i];
			max.x=X[i];}
		}
	return max;
	}

// X-axis normalization

int norm()      
	{int l, f;
	double *X1[Max], *Y1[Max];
	
	for (l=0; l<Max; l++)
		{X1[l] = (double *)malloc(sizeof(double));
		Y1[l] = (double *)malloc(sizeof(double));
		if ((X1[l] == 0) || (Y1[l] == 0))
			{printf("ERROR: Out of memory\n");			
			return 0; }
		}
	int steps2, range, type2;
	double Egrid;
	
	steps2=steps;
	*X1[0]= X[0];
	*X1[1]=(X[steps-1]);
	Egrid=0.0;

	cout<<"\n (1: for fixed energy step)  (2: for fixed number of data points)    ";
	type2=ReadKey();

	if (type2==2)
		{
		cout<<"\n Number of points: ";
		range=ReadKey();
		if(range==0)
			{for (int i=0; i<Max; i++)
				{free(X1[i]);
				free(Y1[i]);}
			return 0;}

		Egrid=fabs((X[steps-1]-X[0])/ range);
		steps2=(long) fabs((*X1[1]-*X1[0])/ Egrid);}

	if (type2==1)
		{
		cout<<" Energy step (eV): ";
		Egrid=ReadKeyF();
		if(Egrid==0.0)
			{for (int i=0; i<Max; i++)
				{free(X1[i]);
				free(Y1[i]);}
			return 0;}

		steps2=(long) fabs((X[steps-1]-X[0])/ Egrid)+1;}
	
	if (type2 !=1 && type2 !=2)
		{for (int i=0; i<Max; i++)
			{free(X1[i]);
			free(Y1[i]);}
		return 0;}

		j=0;
		for(f=0;f<steps2; f++)
			{if(X[0]-X[1]<0)
				{*X1[f]=X[0]+f*Egrid;
				while(true)	
					{j++;
					if((X[j]>=*X1[f]) )
						{break;}
					}
				}
			else
				{*X1[f]=X[0]-f*Egrid;	
				while(true)	
					{j++;
					if((X[j]<*X1[f]) )
						{break;}
					}
				}
		j=j-1;
		*Y1[f]=(Y[j]+((Y[j+1]-Y[j])/(X[j+1]-X[j]))*(*X1[f]-X[j]));}

	for(f=0; f<steps2; f++)
		{Y[f]=*Y1[f];
		X[f]=*X1[f];}
		steps=steps2;

for (l=0; l<Max; l++)
	{free(X1[l]);
	free(Y1[l]);}
return 0;
}

// X-axis new range
int XRange()
{
	if(vplot==1)
		{	
		#if     _MSC_VER
		#else
			if (mkfifo(GPFIFO, 0700)) 
				{
    				if (errno != EEXIST) 
					{perror(GPFIFO);
					unlink(GPFIFO);
					return 1;}
				}
		#endif		
		double *Xi, *Xf;
		double *X1[Max], *Y1[Max];
		int dm =0;
	
		for (int l=0; l<Max; l++)
			{X1[l] = (double *)malloc(sizeof(double));
			Y1[l] = (double *)malloc(sizeof(double));
			if ((X1[l] == 0) || (Y1[l] == 0))
				{printf("ERROR: Out of memory\n");			
				return 0; }
			}
		Xi = (double *)malloc(sizeof(double));
		Xf = (double *)malloc(sizeof(double));

		
		#if     _MSC_VER
			*Xi=0.0;
			*Xf=0.0;
	     		cout<< "\n Enter the initial x coordinate: ";
	 		*Xi=ReadKeyF();
			cout<< "  Enter the final x coordinate: ";
	       		*Xf=ReadKeyF();	
		    		
		#else
			
			fprintf(command, "set print \"%s\"\n", GPFIFO);
			fflush(command);
	
			if (NULL == (gpin = fopen(GPFIFO,"r"))) 
				{perror(GPFIFO);
	   			pclose(command);
	   			return 1;}
		
			*Xi=0.0;
			*Xf=0.0;
			fprintf(command,"set mouse\n");
			fprintf(command,"replot\n");
			fflush(command);			
			cout<<"\n";
	    		fprintf(command, "pause mouse ' Click mouse in the position of the initial coordinate. '\n");
	   		fflush(command);
	       		fprintf(command, "print MOUSE_X\n");
	    		fflush(command);
			dm= fscanf(gpin, "%lf", Xi);
			fprintf(command, "pause mouse ' Click mouse in the position of the final coordinate. '\n");
	   		fflush(command);
	       		fprintf(command, "print MOUSE_X\n");
	    		fflush(command);
			dm = fscanf(gpin, "%lf", Xf);
			
			fclose(gpin);
			unlink (GPFIFO);
			if(*Xi>*Xf)
				{cout<<"\n The initial coordinate must be smaller than the final.\n";
				return 0;}
			cout<<"\n New range: "<<*Xi<<" < X < "<<*Xf<<"\n";
		
		#endif
		int o=0;
		for(j=0; j<steps; j++)
			{if(X[j]>=*Xi && X[j]<=*Xf)
				{*X1[o]=X[j];
				*Y1[o]=Y[j];
				o++;}
			}
		steps=o;
		for(j=0; j<steps; j++)
			{X[j]=*X1[j];
			Y[j]=*Y1[j];}

		for(j=0; j<steps; j++)
			{free(X1[j]);
			free(Y1[j]);}
		free(Xi);
		free(Xf);

		}
	else 
		{cout<<"\nPlease enable plotting (in the settings menu) to change range. \n";}
	return 0;
}

//************************************
//Fitting routines

int Fit(char* name)
{
	if(vplot==1)	
	{	int e, e1, peaktype;
		
		e=1;
		e1=0;
		ReadFile(name);
				
		if(flag!=0)
		{
			while (e!=0)
			{cout<<"\n Peak type: (1: Lorentzian,  2: Gaussian,  other: exit)? "; 
			peaktype=ReadKey();
			if(peaktype!=1 && peaktype!=2)
				{return 0;}
			
			if(peaktype==1)
				{Peak(name, "Lorentzian", 1);}
			if(peaktype==2)
				{Peak(name, "Gaussian", 2);}
		
			if (flag==0)
				{ClosePlot();
				return 0;}

			#if     _MSC_VER
				cout<<"\n Save the plot? (1: PNG, 2: EPS, 3: JPEG, 0: NO) ";
			#else	 
				fprintf(command,"pause 0  \"----------------------------------------------------------------    \" \n ");			
				fprintf(command,"pause 0  \"    \" \n ");
				fprintf(command,"pause -1  \" Save the plot? (1: PNG, 2: EPS, 3: JPEG, 0: NO)  \" \n ");
				fflush(command);
			#endif
			
			e1=ReadKey();
			if(e1!=0)
				{
				if(e1==1)
					{fprintf(command,"set terminal png giant size 1280,800\n");
					fprintf(command,"set out '%s.png' \n", name);}

				if(e1==2)
					{fprintf(command,"set terminal postscript eps enhanced color\n");
					fprintf(command,"set out '%s.eps' \n", name);}

				if(e1==3)
					{fprintf(command,"set terminal jpeg giant size 1280,800\n");
					fprintf(command,"set out '%s.jpg' \n", name);}

				fprintf(command,"set title '%s   (%d Peak-fit)' \n", name, npeaks);
				fflush(command);
				fprintf(command,"replot \n");
				fflush(command);}


			if(peaktype==1)
				{cout<<"\n Each Lorentzian has this form: ((2*A*w/PI)/(4*(x-xc)^2 + w^2))\n";}
			else 
				{cout<<"\n Each Gaussian has this form: \n  ((A/((w/(2*sqrt(log(2))))*sqrt(PI/2)))*exp(-(x-xc)^2/((w*w)/(4*log(2)))))) \n";}
			cout<<"\n Perform another fit? (1: YES, 0: NO) ";
			e=ReadKey();						
			ClosePlot();
			}

		cout<<"\n Final fitting parameters saved in \"fit.log\"\n";
		}
	}
	else
	{}
return 0;
}

//Multi-Peak fitting

int Peak(char*name, char const *cytype, int peaktype)
	{mcoord m = IniPlotFit(name);
	
	if(flag==0)
		{return 0;}

	fprintf(command,"y0= 100.0 \n");
	fprintf(command,"PI= %f\n", PI);
	for(int i=1; i<=npeaks; i++)
		{fprintf(command,"A%d = %f \n", i,m.y[i]);
		fprintf(command,"xc%d = %f \n", i,m.x[i]);
		
		fprintf(command,"w%d = 1.0\n", i);
		if(peaktype==1)
			{fprintf(command,"f%d(x) =  ((2*A%d*w%d/PI)/(4*(x-xc%d)*(x-xc%d) + w%d*w%d))\n",i,i,i,i,i,i,i);}
		else
			{fprintf(command,"f%d(x) =  ((A%d/((w%d/(2*sqrt(log(2))))*sqrt(PI/2)))*exp(-(x-xc%d)*(x-xc%d)/(w%d*w%d/(4*log(2))))) \n",i,i,i,i,i,i,i);}
		}
	
	if(npeaks==1)
	{fprintf(command,"f(x) =  y0 + f1(x)\n");}
	if(npeaks==2)
	{fprintf(command,"f(x) =  y0 + f1(x) + f2(x)\n");}
	if(npeaks==3)
	{fprintf(command,"f(x) =  y0 + f1(x) + f2(x) + f3(x)\n");}
	if(npeaks==4)
	{fprintf(command,"f(x) =  y0 + f1(x) + f2(x) + f3(x) + f4(x)\n");}
	if(npeaks==5)
	{fprintf(command,"f(x) =  y0 + f1(x) + f2(x) + f3(x) + f4(x) +f5(x)\n");}

	fprintf(command,"plot f(x) title '%s' w l ls 1, \"%s\" using 1:2 title 'experiment' w l ls 1\n", cytype, name);
	fflush(command);

	if(npeaks==1)
	{	fprintf(command,"fit f(x) \"%s\" using 1:2 via  y0,A1,xc1,w1 \n",name);
		fflush(command);
		fprintf(command,"plot \"%s\" using 1:2 title 'Experiment' w l lw 2, f(x) title 'Final' w l lw 2\n", name);}
	if(npeaks==2)
	{	fprintf(command,"fit f(x) \"%s\" using 1:2 via  y0,A1,xc1,w1,A2,xc2,w2 \n",name);
		fflush(command);
		fprintf(command,"plot \"%s\" using 1:2 title 'Experiment' w l lw 2, f(x) title 'Final' w l lw 2, f1(x)+y0 title '%s 1' w l lw 1, f2(x)+y0 title '%s 2' w l lw 1\n", name, cytype, cytype);}
	if(npeaks==3)
	{	fprintf(command,"fit f(x) \"%s\" using 1:2 via  y0,A1,xc1,w1,A2,xc2,w2,A3,xc3,w3 \n",name);
		fflush(command);
		fprintf(command,"plot \"%s\" using 1:2 title 'Experiment' w l lw 2, f(x) title 'Final' w l lw 2, f1(x)+y0 title '%s 1' w l lw 1, f2(x)+y0 title '%s 2' w l lw 1, f3(x)+y0 title '%s 3' w l lw 1\n", name, cytype, cytype, cytype);}
	if(npeaks==4)
	{	fprintf(command,"fit f(x) \"%s\" using 1:2 via  y0,A1,xc1,w1,A2,xc2,w2,A3,xc3,w3,A4,xc4,w4 \n",name);
		fflush(command);
		fprintf(command,"plot \"%s\" using 1:2 title 'Experiment' w l lw 2, f(x) title 'Final' w l lw 2, f1(x)+y0 title '%s 1' w l lw 1, f2(x)+y0 title '%s 2' w l lw 1, f3(x)+y0 title '%s 3' w l lw 1, f4(x)+y0 title '%s 4' w l lw 1\n", name, cytype, cytype, cytype, cytype);}
	if(npeaks==5)
	{	fprintf(command,"fit f(x) \"%s\" using 1:2 via  y0,A1,xc1,w1,A2,xc2,w2,A3,xc3,w3,A4,xc4,w4,A5,xc5,w5 \n",name);
		fflush(command);
		fprintf(command,"plot \"%s\" using 1:2 title 'Experiment' w l lw 2, f(x) title 'Final' w l lw 2, f1(x)+y0 title '%s 1' w l lw 1, f2(x)+y0 title '%s 2' w l lw 1, f3(x)+y0 title '%s 3' w l lw 1, f4(x)+y0 title '%s 4' w l lw 1, f5(x)+y0 title '%s 5' w l lw 1\n", name, cytype, cytype, cytype, cytype, cytype);}
	
	fflush(command);
return 0;
}


//************************************
//Plot routines

void IniPlot(char* name)
{	
	if(vplot==1)
	{
	#if     _MSC_VER
		command = popen("pgnuplot","w");
	#else
		command = popen("gnuplot","w");
	#endif
		
		PlotSettings();
		Plot1(name);
	}
	else
	{}
}

mcoord IniPlotFit(char *name)
{	mcoord m;
	m.x[0]=0.0;
	m.y[0]=0.0;
	if(vplot==1)
		{
			int dm = 0;
		#if     _MSC_VER
			command = popen("pgnuplot","w");
		#else
			if (mkfifo(GPFIFO, 0600)) 
				{
    				if (errno != EEXIST) 
					{perror(GPFIFO);
					unlink(GPFIFO);
					return m;}
				}
			command = popen("gnuplot","w");
		#endif		
		
		PlotSettings();
		Plot1(name);
		cout<<" Number of peaks (1-5): ";
		npeaks=ReadKey();
		if(npeaks!=1 &&	npeaks!=2 && npeaks!=3 && npeaks!=4 &&	npeaks!=5)	
			{flag=0;
			cout<<" \n Too many peaks. Max allowed: 5 \n";
			return m;}	

		#if     _MSC_VER
			for (int n=1; n<=npeaks; n++) 
				{m.x[n]=0.0;
				m.y[n]=0.0;
				cout<<"\n Enter the approximate coordinates of the peak #"<<n<<": \n";	     			
				cout<<"   X["<<n<<"]: ";
	 			m.x[n]=ReadKeyF();
				cout<<"   Y["<<n<<"]: (or type \"m\" for maximum, \"m2\" for maximum/2) ";
	       			m.y[n]=ReadKeyF();
				
				cout<<"   X["<<n<<"]= "<<m.x[n]<<";   Y["<<n<<"]= "<<m.y[n]<<"\n";}
					
		#else
		
			fprintf(command, "set print \"%s\"\n", GPFIFO);
			fflush(command);
	
			if (NULL == (gpin = fopen(GPFIFO,"r"))) 
				{perror(GPFIFO);
	   			pclose(command);
	   			return m;}
		
			for (int n=1; n<=npeaks; n++) 
				{m.x[n]=0.0;
				m.y[n]=0.0;
	 
	    			fprintf(command, "pause mouse ' Click mouse in the position corresponding to the maximum of peak # %d: '\n", n);   		 	
				fflush(command);
				cout<<"\n";   				//for new line in gnuplot v.4.2	       			
				fprintf(command, "print MOUSE_X, MOUSE_Y\n");
	    			fflush(command);
				dm=fscanf(gpin, "%f %f", &m.x[n], &m.y[n]);		
				cout<<"\n   X["<<n<<"]= "<<m.x[n]<<";   Y["<<n<<"]= "<<m.y[n]<<"\n";			
				}

			fclose(gpin);
			unlink (GPFIFO);
				
		#endif
		
		}
	else 
		{}
		
		
	return m;
}


void Plot1(char* name)
{	
	if(vplot==1)	
		{fprintf(command,"plot \"%s\" with lines\n", name);
		fflush(command);}
	else
		{}
	}

void Plot2(char* name , char* nameout)
{	
	if(vplot==1)	
		{fprintf(command,"plot \"%s\" with lines, \"%s\" with lines\n", name, nameout);
		fflush(command);}
	else
		{}
	}

void PlotSettings()
	{
	fprintf(command,"set mouse\n");
	fprintf(command,"set autoscale\n");     
	fprintf(command,"set grid\n");
	if(xaxis==1 && bconv==2)		
		{fprintf(command,"set xlabel 'Kinetic energy [eV]'\n");}
	else
		{fprintf(command,"set xlabel 'Binding energy [eV]'\n");}
	if(ctsconv==2 && Ytypef==1)
		{fprintf(command,"set ylabel 'counts'\n");}
	else
		{fprintf(command,"set ylabel 'counts/s'\n");}	
	fprintf(command,"set title 'Use the right button of your mouse to zoom. Type p to unzoom. Middle click to lock cursor.'\n");
	fflush(command);
	}	

void ClosePlot()
	{
	if(vplot==1)	
		{fprintf(command,"q\n");
		fflush(command);}
	else
		{}
}

//************************************
// File I/O

int ReadFile(char* name)
{
	#if     _MSC_VER
		ifstream infile(name);		//MSVC
	#else
		ifstream infile(name);		//gcc
	#endif

	flag=1;

	if(!infile)
		{cout<<"\n file \""<< name<<"\" not found\n";
		flag=0;
		return 0;}
			
	j=0;

	while(!infile.eof())
		{infile>>X[j]>>Y[j];
		j++;}
	steps=j-1;

	infile.close();
	return 0;
}

int SaveFile(char *nameout)
{	ofstream outfile(nameout);	
	for(j=0; j<steps; j++)
		{outfile<<X[j]<<"\t"<<Y[j]<<"\n";}
	outfile.close();
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////
// Save individual regions into one datasheet.

int savesheet(char* nameIN, int n, int d)

	{	if(n>maxNregions)
			{cout<<"Too many regions (Max: "<<maxNregions<<")\n";
			return 0;}
		
		double X[Max][maxNregions], Y[Max][maxNregions];

		typedef struct dataset{
			int steps[10];
			int bs[10];
			} dataset;

		dataset set; 
		int Msteps;

		char *name, *name0, *name2, *name3; 
		name=(char *)malloc(sizeof(char[CMAXI]));
		name2=(char *)malloc(sizeof(char[CMAXI]));
		name3=(char *)malloc(sizeof(char[CMAXI]));
		name0=(char *)malloc(sizeof(char[CMAXI]));

		snprintf(name0, CMAXI, "%s", nameIN); 
		if(d==1)		
			{snprintf(nameIN, CMAXI, "%s%s", name0, regextV);}
		else
			{snprintf(nameIN, CMAXI, "%s%s", name0, regextS);}
		
		snprintf(name, CMAXI, "%s", nameIN);
		
		for(i=1; i<=n; i++)
			{snprintf(name2, CMAXI, "%s%d_bs.dat", nameIN,i); 

			#if     _MSC_VER
			ifstream infile2(name2);		//MSVC
			#else
			ifstream infile2(name2);		//gcc
			#endif
			
			infile2.close();
			set.bs[i]=1;
			if(!infile2)
				{set.bs[i]=0;
				snprintf(name2, CMAXI, "%s%d.dat", nameIN, i); 				
				}
	
			#if     _MSC_VER
			ifstream infile(name2);		//MSVC
			#else
			ifstream infile(name2);		//gcc
			#endif
			j=0;	

			if(!infile)
			{	cout<<"\nFile not found.\n";
				return 0;}	
		
			while(!infile.eof())
				{infile>>X[j][i]>>Y[j][i];
				j++;}
				set.steps[i]=j-1;
			infile.close();
			}

		snprintf(name3, CMAXI, "%s.set.dat", nameIN);		
		ofstream outfile(name3);

		Msteps=0;
		for (i=1;i<=n;i++)	
			{if(set.bs[i]==0)
				{outfile<<"X"<<i<<"\tY"<<i<<"\t";}
			else
				{outfile<<"X"<<i<<"bs\tY"<<i<<"bs\t";}
			}
		outfile<<"\n";
		for (j=0;j<Max;j++)
			{for (i=1;i<=n;i++)				
				{
				if(j<set.steps[i])
					{outfile<<X[j][i]<<"\t"<<Y[j][i]<<"\t";}
				else
					{outfile<<"\t"<<"\t";}

				if(set.steps[i]>Msteps)					
					{Msteps=set.steps[i];}
				}
			outfile<<"\n";
			if(j>Msteps)
				{break;}
			}
		cout<<"\nSaved "<<n<<" regions in file: "<<name3<<"\n\n";
		free(name);
		free(name0);
		free(name2);
		free(name3);		
	return 0;	
	}
//************************************
// Sleep

void SleepP(int time)
{	
	#if     _MSC_VER
		Sleep(time);
	#else
		usleep(time*1000); 
	#endif
}

void clearscreen() {
	#if     _MSC_VER
		if (system( "clear" )) {system( "cls" );}
	#else
		int dm = 0;
		int dm1 = 0;
		dm=system( "clear" );
		if (dm==1) 
			{dm1=system( "cls" );}
	#endif
	}

//************************************
// Keyboard input I/O

int ReadKey()
{	char tkc[10];
	int tk;
	cin>>tkc;
		
	tk=(int) atof(tkc);
	if(tk<0)
		{return 10;}
	else
		{}

	return tk;
}

float ReadKeyF()
{	char tkc[10];
	float tk=0.0;
	cin>>tkc;
	if (strcmp(tkc,"m")==0 || strcmp(tkc,"m2")==0)
		{if (strcmp(tkc,"m")==0)
			{tk=(float) Maximum().y;}
		if (strcmp(tkc,"m2")==0)
			{tk=(float) Maximum().y/2;}
		}

	else
		{	
		tk= (float) atof(tkc);
		if(tk<0)
			{return 10;}
		else
			{}
	}

	return tk;
}

//***********************************

void PreferencePanel()

{	cout<<"\n****************************************************************************\n \"Preferences\" \n";
	cout<<"\n1)";
	if(outtype ==1)
		{cout <<" Output is saved in a file with extension \".dat\".\n";}		
	if (outtype ==2)
		{cout<<" The program asks for the name of the output file.\n";}
	
	cout<<"2)";
	if(infofile ==1)
		{cout <<" Info saved in a file with extension \".info.txt\". \n";}
	if(infofile ==2)
		{cout<<" Info file not saved.\n";}	
	
	cout<<"\n3) Automatic conversion of the intesity label to \"counts/s\": ";
	if(ctsconv == 1)
		{cout <<" Yes \n";}
	if(ctsconv == 2)
		{cout<<" No \n";}	

	cout<<"\n4) Automatic conversion of the x axis to \"binding energy\": ";
	if(bconv == 1)
		{cout <<" Yes \n";}
	if(bconv == 2)
		{cout<<" No \n";}
	
	cout<<"\n5) 5-point smoothing for: ";
	if(smoothT ==1)
		{cout<<" no spectra\n";}
	if(smoothT ==2)
		{cout <<" all but first region\n";}
	if(smoothT ==3)
		{cout <<" all regions\n";}
	
	cout<<"6) Shirley correction for: ";
	if (shirleyT ==1)
		{cout<<" no spectra\n";}
	if(shirleyT ==2)
		{cout <<" all but first region\n";}
	if(shirleyT ==3)
		{cout <<" all regions\n";}
	
	cout<<"7) Energy-dependence correction for: ";
	if (ecorrT ==1)
		{cout<<" no spectra\n";}
	if(ecorrT ==2)
		{cout <<" all but first region\n";}
	if(ecorrT ==3)
		{cout <<" all regions\n";}

	cout<<"\n8) Save regions in one ASCII datasheet: ";
	if(dsheet ==1)
		{cout <<" Yes \n";}
	if(dsheet == 2)
		{cout<<" No \n";}

	
	cout<<"\n9) Spectra are currently: ";
	if(vplot ==1)
		{cout <<" plotted. \n";}
	if (vplot ==2)
		{cout<<" NOT plotted.\n\n   Some features (fitting) may not be available if plotting is not enabled. \n";}	
	cout<<"\n*****************************************************************************\n";
}

