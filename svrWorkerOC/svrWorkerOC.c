// Module Name: svrWorkerOC.c
//
// Description:
//    Prototipo Servidor JDE...
//
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "stdafx.h"
#include <jde.h>

#pragma comment(lib, "Ws2_32.lib")

#include "F4101.h" 

#include "xt4311z1.h"	// F4311FSBeginDoc, F4311EditLine, F4311EndDoc,F4311ClearWorkFiles    
#include "b9800100.h"	// GetAuditInfo
#include "B4000370.h"	// F40095GetDefaultBranch
#include "B0000130.h"	// RetrieveCompanyFromBusUnit
#include "B4000150.h"	// GetBranchConstants
#include "xf41021.h"	// VerifyAndGetItemLocation
#include "b4000310.h"	// FormatLocation
#include "b4001050.h"	// GetCrossReferenceFields
#include "n0000563.h"	// F0010RetrieveCompanyConstant
#include "b4000610.h"	// GetLotMasterByLotNumber
#include "B4000520.h"	// GetItemUoMConversionFactor
#include "B1100007.h"	// DecimalsTriggerGetbyCOCRCD
#include "B9800420.h"	// GetDataDictionaryDefinition
#include "B0100039.h"	// IsColumnInAddressBook


	//Declaraciones UDP...
int   ilPort    = 0;	// Port to receive on - local...
int   irPort    = 0;	// Port to send on - remote...
#define DEFAULT_BUFFER_LENGTH   5500

//

void OWDCmp01 (int * primeraVez, int * opcionEscogida, char * pantallaTitulo, int opcionesCantidad, int inicioEtiquetas, 
				char * opcionesEtiqueta[], char * pantallaStatusLine, int iDbgFlg, FILE * dlg);//Manejo de la pantalla de menu...

void OWDCmp02 (int * primeraVez, char * pantallaTitulo, int camposOffset, int camposCantidad, 
				int statusOffset, int inicioEtiquetas, int inicioCampos, int ultimaLinea, int CReqTAB,
				int camposPosiciones[], char * camposEtiquetas[], char camposContenido[][128], int camposErrores[],
				char * pantallaStatusLine, int iDbgFlg, FILE * dlg); //Manejo de la pantalla de entrada datos...

void OWDCmp90 (int iDbgFlg, FILE * dlg); //Termina Manejo de la pantalla con curses...

//
// Function: usage -Print usage information and exit
//
void usage(int iDbgFlg, FILE * dlg){

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Inicio usage (ERROR!!!)...\n"));;
		jdeFflush(dlg);
	}

    printf("usage: svrWorkerOC [-r:int] [-l:int]\n\n");
    printf("       -r:int   Remote port\n");
    printf("       -l:int   Local port\n");
    ExitProcess(1);
}

//
// Function: ValidateArgs - Parse the command line arguments, and set some global flags to
//    indicate what actions to perform
//
void ValidateArgs(int argc, char **argv, int iDbgFlg, FILE * dlg){
    int                i;

	if (iDbgFlg > 0) {
		jdeFprintf(dlg,_J("***Inicio ValidateArgs...\n"));
		jdeFflush(dlg);
	}

    for(i = 1; i < argc; i++){
        if ((argv[i][0] == '-') || (argv[i][0] == '/')){
            switch (tolower(argv[i][1])){
                case 'r':   // Remote port...
                    if (strlen(argv[i]) > 3)
                        irPort = atoi(&argv[i][3]);
						if (iDbgFlg > 0) {
							fprintf(dlg,"*** ValidateArgs r:(%i)...\n",irPort);
							jdeFflush(dlg);
						}
                    break;
                case 'l':   // Local port
                    if (strlen(argv[i]) > 3)
                        ilPort = atoi(&argv[i][3]);
						if (iDbgFlg > 0) {
							jdeFprintf(dlg,_J("*** ValidateArgs l:(%i)...\n"),ilPort);
							jdeFflush(dlg);
						}
                    break;

                default:
                    usage(iDbgFlg, dlg);
                    break;
            }
        } else usage(iDbgFlg, dlg);
    }
}
        

int _cdecl  main(int argc, char **argv){

	//Declaraciones JDE...


	HENV	hEnv		= NULL;   
	HUSER	hUser		= NULL;    

	FILE *	ini;
	FILE *	mnu; 
	FILE *	dlg;   
 
	JCHAR szLinea[80],szLin1[64],szLin2[64],szUsrEntry[128],
		 szUsrEnv[16],szUsrID[16],szUsrPass[16],szUsrLeng[2],szMenuFile[16], 
		 szDocTransDateBuf[16],szDocGLDateBuf[16],szDocBranchPlantBuf[16],szLocationBuf[21],szLineLocationBuf[21],
		 szReferenciaClienteBuf[26],szUbicacionDfltBuf[21],szItemDescriptionBuf[31],
		 szLineItemBuf[26],szLineUMBuf[3],szUOMdefaultBuf[3],szUOMstdConv[3],szLineLotBuf[31],szLotExpirationDate[16],
		 szPOOrderTypeBuf[3],szP4310VersionBuf[16],szBeginningStatusBuf[4], szLineUnitOfMeasureBuf[3],szCrossRefTypeCodeBuf[3],
		 szEtiquetaCodigo[64],

		 LszString01[128],LszString02[64],szTempBuf[128],* szDummy0,

		 cStandAloneFlg, cSeparadorEtiqueta[2], cUbicProcess, cValidarUbicacion, cLotProcess, cValidarLote;

	int	 iErrorCode,iDbgFlg,iOpcionSalir,i,iSalir,iEditLineLines,
		 iDocumentNumberBuf,iUPClenBuf,iSCClenBuf,iItemMinLenBuf,iGetItemMasterBy,
		 iCantLotesProc,iNumLotes,iCantDecQty,iLineQtyBuf,iLineIncrementBuf,
		 iCamposOffset, iCamposCantidad, iStatusOffset, iInicioEtiquetas, iInicioCampos, iUltimaLinea, iCReqTAB, 
		 iCamposPosiciones[64], iCamposErrores[64], iOpcionEscogida,
		 iPrimeraVez, iInitPhase;

	char * szCamposEtiquetas[64], szCamposContenido[64][128], szPantallaTitulo[64], szPantallaStatusLine[64], szDummy[128]; 
	 

#define INIwidth 80
#define NumFasesInit 1
#define CacheSize 110

	HREQUEST			hRequest	= (HREQUEST)NULL;
	LPJDEERROR_RECORD	ErrorRec	= NULL;
	INT					NumErrors	= 0;
	INT					NumWarnings	= 0;
	ID					idResult;
	LPCG_BHVR			lpVoid		= NULL;
	LPCG_BHVR			lpVoid1		= NULL;
	LPVOID				lpDS		= NULL;
	LPBHVRCOM			lpBhvrCom	= NULL;
	LPBHVRCOM			lpBhvrCom1	= NULL;
	ERROR_EVENT_KEY		EventKeyLocal;

	DSDF4311Z1A			dsF4311BeginDoc;
	DSDF4311Z1B			dsF4311EditLine;
	DSDF4311Z1D			dsF4311EndDoc;
	DSDF4311Z1E			dsF4311ClearWorkFiles;
	DSD4000150			dsGetBranchConstants;
	DSD4000232			dsF40095GetDefaultBranch;
	DSD0000130			dsRetrieveCompanyFromBusUnit;
	DSDXF41021C			dsVerifyAndGetItemLocation;
	DSD4001050			dsGetCrossReferenceFields;
	DSD4000310A			dsFormatLocation;
	DSD9800100			dsGetAuditInfo;
	DSD0000563			dsF0010RetrieveCompanyConstant;
	DSD4000610			dsGetLotMasterByLotNumber;
	DSD0000042B			dsFSCloseBatch;
	DSD4000520			dsGetItemUoMConversionFactor;
	DSD1100007			dsDecimalsTriggerGetbyCOCRCD;
	DSD9800420			dsGetDataDictionaryDefinition;
	LPFORMDSUDC			lpValidateUDC;
	FORMDSUDC			dsValidateUDC;
	DSD0100039			dsIsColumnInAddressBook;

	MATH_NUMERIC		mnTempBuf,mnTemp0Buf,mnItemShortIDBuf,mnCantIntroducidaBuf,mnCantIntroducida1Buf,
						mnCodigoProverdorBuff;


	typedef struct {
		MATH_NUMERIC	imitm;
		JCHAR           imlitm[26];		//Identifier2nd
		JCHAR           imdsc1[31];		//Description 1
	} stF4101Descr1;
    typedef struct {		
		char           codigo[21];		//Identifier2nd
		char           descripcion[31];	//Description 1
	} stF4101Descr2;

	stF4101Descr1		dsF4101Descr1;
	stF4101Descr2		dsF4101Descr2;
	char				dsProductosArr[CacheSize][50];

	HREQUEST			hRequestF4101 = (HREQUEST)NULL;
	NID					szF4101DescrColumnsArray[3] = { NID_ITM, NID_LITM, NID_DSC1 };
	KEY1_F4101			dsF4101Key1;
	SELECTSTRUCT		lpSelect[4];

	//Declaraciones UDP...
	//int   ilPort    = DEFAULT_PORT_LOCAL;   // Port to receive on
	//int   irPort    = DEFAULT_PORT_REMOTE;  // Port to send on
	DWORD dwLength = DEFAULT_BUFFER_LENGTH; // Length of receiving buffer
	DWORD dwLength1;

    WSADATA        wsd;
    SOCKET         s;
    char          *recvbuf = NULL;
	char          *sendbuf = NULL;
    int            ret;
    DWORD          dwRemoteSize;
    SOCKADDR_IN    remote, local;


	
	//*************************************************
	//*** Procesamiento login a JDE...
	//*************************************************

	//***Procesamiento Archivo INI                                          
	//Contruye nombre de archivo como nombre_Daammdd_Thhmmss.log
	char timebuf[9],datebuf[9];
	JCHAR Ltimebuf[9],Ldatebuf[9],szFileNameBuf[64];

	_strtime(timebuf);
	_strdate(datebuf);
	jdeToUnicode(Ltimebuf,timebuf,DIM(Ltimebuf),UTF8);
	jdeToUnicode(Ldatebuf,datebuf,DIM(Ldatebuf),UTF8);

	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCsvrWorkerOC_D"));
	Ldatebuf[2] = _J('\0');
	Ldatebuf[5] = _J('\0');

	jdeStrcpy(szTempBuf,Ldatebuf + 6);
	jdeStrcat(szFileNameBuf,szTempBuf);
	jdeStrcpy(szTempBuf,Ldatebuf);
	jdeStrcat(szFileNameBuf,szTempBuf);
	jdeStrcpy(szTempBuf,Ldatebuf + 3);
	jdeStrcat(szFileNameBuf,szTempBuf);

	Ltimebuf[2] = _J('\0');
	Ltimebuf[5] = _J('\0');
	jdeStrcat(szFileNameBuf,_J("_T"));
	jdeStrcpy(szTempBuf,Ltimebuf);
	jdeStrcat(szFileNameBuf,szTempBuf);
	jdeStrcpy(szTempBuf,Ltimebuf + 3);
	jdeStrcat(szFileNameBuf,szTempBuf);
	jdeStrcpy(szTempBuf,Ltimebuf + 6);
	jdeStrcat(szFileNameBuf,szTempBuf);
	jdeStrcat(szFileNameBuf,_J(".log"));

	dlg = jdeFopen(szFileNameBuf,_J("w"));

	ini = jdeFopen(_J("OWDCsvrWorkerOC.ini"),_J("r"));
	if (!ini){
		iErrorCode =  101;
		jdeFprintf(dlg,_J("***Error(%d) abriendo INI (OWDCsvrWorkerOC.ini)...\n"),iErrorCode);
        goto lbFIN1;
	} 

	while (jdeFgets(szLinea,INIwidth,ini) != NULL){ //***Mientras haya registros en ini file
		for(i=0; (szLinea[i] != _J('=') && i < INIwidth); i++);
		if (szLinea[i] == _J('=')){//Localiza el signo "="
			jdeStrncpy(szLin1, szLinea, i);
			szLin1[i] = _J('\0');//Divide el string en una porción antes "=" y otra porción despues "="
			i++;
			jdeStrncpy(szLin2,szLinea + i,jdeStrlen(szLinea) - i + 1 );
			szLin2[jdeStrlen(szLin2) - 1] = _J('\0');
		} 

		if(jdeStrcmp(szLin1,_J("Debug")) == 0){
			iDbgFlg = jdeAtoi(szLin2);
			if (iDbgFlg == 1){

			}
			else dlg = stderr;
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: Debug (%d)...\n"), iDbgFlg);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("UserID")) == 0){
			jdeStrcpy(szUsrID, szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: szUsrID (%ls)...\n"),szUsrID);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("UserPWD")) == 0){
			jdeStrcpy(szUsrPass, szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: szUsrPass (%ls)...\n"),szUsrPass);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("UserENV")) == 0){
			jdeStrcpy(szUsrEnv, szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: szUsrEnv (%ls)...\n"),szUsrEnv);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("Leng")) == 0){
			jdeStrcpy(szUsrLeng, szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: Leng (%ls)...\n"),szUsrLeng);
			continue;
		}		
		if(jdeStrcmp(szLin1,_J("StandAlone")) == 0){
			cStandAloneFlg = szLin2[0];
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: StandAlone (%lc)...\n"),cStandAloneFlg); 
			continue;
		}
		if(jdeStrcmp(szLin1,_J("camposOffset")) == 0){
			iCamposOffset = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: camposOffset (%d)...\n"),iCamposOffset); 
			continue;
		}
		if(jdeStrcmp(szLin1,_J("statusOffset")) == 0){
			iStatusOffset = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: statusOffset (%d)...\n"),iStatusOffset); 
			continue;
		}
		if(jdeStrcmp(szLin1,_J("inicioEtiquetas")) == 0){
			iInicioEtiquetas = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: inicioEtiquetas (%d)...\n"),iInicioEtiquetas); 
			continue;
		}
		if(jdeStrcmp(szLin1,_J("inicioCampos")) == 0){
			iInicioCampos = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: inicioCampos (%d)...\n"),iInicioCampos); 
			continue;
		}
		if(jdeStrcmp(szLin1,_J("ultimaLinea")) == 0){
			iUltimaLinea = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ultimaLinea (%d)...\n"),iUltimaLinea); 
			continue;
		}
		if(jdeStrcmp(szLin1,_J("CReqTAB")) == 0){
			iCReqTAB = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: CReqTAB (%d)...\n"),iCReqTAB); 
			continue;
		}
		if(jdeStrcmp(szLin1,_J("FechaGL")) == 0){
			jdeStrcpy(szDocGLDateBuf,szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: FechaGL (%ls)...\n"),szDocGLDateBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("FechaTran")) == 0){
			jdeStrcpy(szDocTransDateBuf,szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: FechaTran (%ls)...\n"),szDocTransDateBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("POOrderType")) == 0){
			jdeStrcpy(szPOOrderTypeBuf,szLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: POOrderType (%ls)...\n"),
										szPOOrderTypeBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("P4310Version")) == 0){
			jdeStrcpy(szP4310VersionBuf,szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: P4310Version (%ls)...\n"),szP4310VersionBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("BeginningStatus")) == 0){
			jdeStrcpy(szBeginningStatusBuf,szLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: BeginningStatus (%ls)...\n"),
										szBeginningStatusBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("LineUnitOfMeasure")) == 0){
			jdeStrcpy(szLineUnitOfMeasureBuf,szLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LineUnitOfMeasure (%ls)...\n"),
										szLineUnitOfMeasureBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("LineQty")) == 0){
			iLineQtyBuf = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LineQty (%d)...\n"),
										iLineQtyBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("LineIncrement")) == 0){
			iLineIncrementBuf = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LineIncrement (%d)...\n"),
										iLineIncrementBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("UPClen")) == 0){
			iUPClenBuf = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: UPClen (%d)...\n"),
										iUPClenBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("SCClen")) == 0){
			iSCClenBuf = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: SCClen (%d)...\n"),
										iSCClenBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("ItemMinLen")) == 0){
			iItemMinLenBuf = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ItemMinLen (%d)...\n"),
										iItemMinLenBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("GetItemMasterBy")) == 0){
			iGetItemMasterBy = jdeAtoi(szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: GetItemMasterBy (%d)...\n"),
										iGetItemMasterBy);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("CrossRefTypeCode")) == 0){
			jdeStrcpy(szCrossRefTypeCodeBuf, szLin2);			
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: CrossRefTypeCode (%ls)...\n"),
										szCrossRefTypeCodeBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("UbicProcess")) == 0){
			cUbicProcess = szLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: UbicProcess (%lc)...\n"),
										cUbicProcess);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("ValidarUbicacion")) == 0){
			cValidarUbicacion = szLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ValidarUbicacion (%lc)...\n"),
										cValidarUbicacion);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("UbicacionDflt")) == 0){
			jdeStrcpy(szUbicacionDfltBuf,szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: UbicacionDflt (%ls)...\n"),szUbicacionDfltBuf);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("LotProcess")) == 0){
			cLotProcess = szLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LotProcess (%lc)...\n"),
										cLotProcess);
			continue;
		}		
		if(jdeStrcmp(szLin1,_J("ValidarLote")) == 0){
			cValidarLote = szLin2[0];	
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: ValidarLote (%lc)...\n"),
										cValidarLote);
			continue;
		}
		if(jdeStrcmp(szLin1,_J("LotExpirationDate")) == 0){
			jdeStrcpy(szLotExpirationDate,szLin2);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: LotExpirationDate (%ls)...\n"),szLotExpirationDate);
			continue;
		}


		// ***
		if(jdeStrcmp(szLin1,_J("SeparadorEtiqueta")) == 0){
			memset(cSeparadorEtiqueta,'\0',sizeof(cSeparadorEtiqueta));
			cSeparadorEtiqueta[0] = szLin2[0];
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***INI: SeparadorEtiqueta (%lc)...\n"),cSeparadorEtiqueta[0]);
			continue;
		}
	}
	jdeFclose(ini);
	jdeFflush(dlg); 

	ValidateArgs(argc, argv, iDbgFlg, dlg);

	//***Fin Procesamiento INI   

	//***Sign In a JDE...
	iErrorCode = 0; //No hay Errores!!!

	if (cStandAloneFlg == _J('1')) {//Login caracter a OW, no compartido...

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));

		iPrimeraVez = 0;
		strcpy (szPantallaTitulo, "Servidor UDP JDE");
		strcpy (szPantallaStatusLine, "");

		szCamposEtiquetas[0] = "Ambiente...";
		szCamposEtiquetas[1] = "Usuario....";
		szCamposEtiquetas[2] = "Password...";

		jdeFromUnicode(szCamposContenido[0],szUsrEnv,127,UTF8);	
		iCamposPosiciones[0] = strlen(szCamposContenido[0]);
		jdeFromUnicode(szCamposContenido[1],szUsrID,127,UTF8);
		iCamposPosiciones[1] = strlen(szCamposContenido[1]);
		jdeFromUnicode(szCamposContenido[2],szUsrPass,127,UTF8);
		iCamposPosiciones[2] = strlen(szCamposContenido[2]);

		iCamposCantidad = 3;//tres campos a desplegar...

		OWDCmp02 (&iPrimeraVez, szPantallaTitulo, iCamposOffset, iCamposCantidad, iStatusOffset,iInicioEtiquetas, 
					iInicioCampos, iUltimaLinea, iCReqTAB,
					iCamposPosiciones, szCamposEtiquetas, szCamposContenido, iCamposErrores, szPantallaStatusLine,
					iDbgFlg, dlg);

		jdeToUnicode(szUsrEnv,szCamposContenido[0],127,UTF8);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szUsrEnv(%ls)...\n"), szUsrEnv);

		jdeToUnicode(szUsrID,szCamposContenido[1],127,UTF8);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szUsrID(%ls)...\n"), szUsrID);
		
		jdeToUnicode(szUsrPass,szCamposContenido[1],127,UTF8);
		if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szUsrPass(%ls)...\n"), szUsrPass);

		//Salir de curses...
		OWDCmp90 (iDbgFlg, dlg);
		jdeFflush(dlg);

		//Initialize Environment Handle
		if(JDB_InitEnvOvrExtended(&hEnv,szUsrEnv,szUsrID,szUsrPass, _J("*ALL"))!=JDEDB_PASSED){
			iErrorCode =  102;
			jdeFprintf(dlg,_J("***Error(%d): JDB_InitEnvOvrExtended failed...\n"),iErrorCode);
			goto lbFIN2;
		}
		//Initialize User Handle
		if(JDB_InitUser(hEnv, &hUser, NULL,JDEDB_COMMIT_AUTO)!=JDEDB_PASSED){
			iErrorCode =  103;		
			jdeFprintf(dlg,_J("***Error(%d): JDB_InitUser failed...\n"),iErrorCode);
			goto lbFIN3;
		}
		
	}
	else{//Login grafico a OW, compartido...
		if(JDB_InitEnv(&hEnv)!=JDEDB_PASSED){
			iErrorCode =  102;
			jdeFprintf(dlg,_J("***Error(%d): JDB_InitEnv failed...\n"),iErrorCode);
			goto lbFIN2;
		}
		//Initialize User Handle
		if(JDB_InitUser(hEnv, &hUser, NULL,JDEDB_COMMIT_AUTO)!=JDEDB_PASSED){
			iErrorCode =  103;		
			jdeFprintf(dlg,_J("***Error(%d): JDB_InitUser failed...\n"),iErrorCode);
			goto lbFIN3;
		}
	}

	jdeFflush(dlg);

	//***Fin Sign In a JDE...

	iErrorCode = 0; //No hay Errores!!!

	//***Set up the lpBhvrCom amd lpVoid objects                              
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("**Seteo de lpBhvrCom, lpVoid y estructuras...\n"));
	jdePrintf(_J("**Seteo de lpBhvrCom, lpVoid y estructuras...\n"));
	//F4311BeginDoc, F4311EditLine, F4311EndDoc
	jdeCreateBusinessFunctionParms(hUser, &lpBhvrCom1,(LPVOID*) &lpVoid1);
	lpVoid1->lpHdr = jdeErrorInitializeEx();
	lpVoid1->lpErrorEventKey = (LPERROR_EVENT_KEY) jdeAlloc(COMMON_POOL, 
										sizeof(ERROR_EVENT_KEY), MEM_ZEROINIT | MEM_FIXED);
	lpVoid1->lpHdr->nCurDisplayed = -1;
	lpBhvrCom1->lpObj->lpFormHdr = lpVoid1->lpHdr;
	EventKeyLocal.hwndCtrl = NULL;
	EventKeyLocal.iGridCol = 0;
	EventKeyLocal.iGridRow = 0;
	EventKeyLocal.wEvent = 1;
	lpBhvrCom1->lpEventKey = (LPVOID)&EventKeyLocal;

	//Otros
	jdeCreateBusinessFunctionParms(hUser, &lpBhvrCom,(LPVOID*) &lpVoid);
	lpVoid->lpHdr = jdeErrorInitializeEx();
	lpVoid->lpErrorEventKey = (LPERROR_EVENT_KEY) jdeAlloc(COMMON_POOL, 
										sizeof(ERROR_EVENT_KEY), MEM_ZEROINIT | MEM_FIXED);
	lpVoid->lpHdr->nCurDisplayed = -1;
	lpBhvrCom->lpObj->lpFormHdr = lpVoid->lpHdr;
	EventKeyLocal.hwndCtrl = NULL;
	EventKeyLocal.iGridCol = 0;
	EventKeyLocal.iGridRow = 0;
	EventKeyLocal.wEvent = 1;
	lpBhvrCom->lpEventKey = (LPVOID)&EventKeyLocal;

	memset((void *) &dsGetAuditInfo,(int) _J('\0'),sizeof(DSD9800100));
	memset((void *) &dsF40095GetDefaultBranch,(int) _J('\0'),sizeof(DSD4000232));
	memset((void *) &dsRetrieveCompanyFromBusUnit, (int) _J('\0'), sizeof(DSD0000130));
	memset((void *) &dsGetBranchConstants, (int) _J('\0'), sizeof(DSD4000150));
	memset((void *) &dsRetrieveCompanyFromBusUnit,(int) _J('\0'),sizeof(DSD0000130));
	memset((void *) &dsVerifyAndGetItemLocation,(int) _J('\0'),sizeof(DSDXF41021C));
	memset((void *) &dsGetCrossReferenceFields,(int) _J('\0'),sizeof(DSD4001050));
	memset((void *) &dsF0010RetrieveCompanyConstant,(int) _J('\0'),sizeof(DSD0000563));
	memset((void *) &dsGetLotMasterByLotNumber,(int)(_J('\0')),sizeof(DSD4000610));

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("**Determina Almacen, Compania, Maquina, Fecha y ProcOpt...\n"));
	jdePrintf(_J("**Determina Almacen, Compania, Maquina, Fecha y ProcOpt...\n"));

	//***Obtiene Uasuario, máquina, fecha
	idResult = jdeCallObject(_J("GetAuditInfo"),NULL,lpBhvrCom,lpVoid,&dsGetAuditInfo,
					(CALLMAP *) NULL,(int)(0),(JCHAR *) NULL,(JCHAR *) NULL,CALL_OBJECT_NO_ERRORS);
	if (idResult == ER_ERROR){
		iErrorCode = 601;
		jdeFprintf(dlg,_J("***Error(%d): GetAuditInfo:...\n"), iErrorCode);
		jdePrintf(_J("***Error(%d): GetAuditInfo:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 

	//***Busca Almacén por default del usuario
	idResult = jdeCallObject(_J("GetDefaultBranch"),NULL,lpBhvrCom,lpVoid,(LPVOID)&dsF40095GetDefaultBranch, 
						(CALLMAP *)NULL, (int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 602;
		jdeFprintf(dlg,_J("***Error(%d): GetDefaultBranch:...\n"), iErrorCode);
		jdePrintf(_J("***Error(%d): GetDefaultBranch:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 
	jdeStrcpy(szDocBranchPlantBuf,dsF40095GetDefaultBranch.szBranch);
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetDefaultBranch (%ls)...\n"),szDocBranchPlantBuf);
	jdePrintf(_J("***GetDefaultBranch (%ls)...\n"),szDocBranchPlantBuf);

	//***Busca Compañía de Centro de Costo	
	jdeStrcpy(dsRetrieveCompanyFromBusUnit.szCostCenter,szDocBranchPlantBuf);
	idResult = jdeCallObject(_J("RetrieveCompanyFromBusUnit"),NULL,lpBhvrCom,lpVoid,
						(LPVOID)&dsRetrieveCompanyFromBusUnit,(CALLMAP *)NULL,(int)0, 
						(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 603;
		jdeFprintf(dlg,_J("***Error(%d): RetrieveCompanyFromBusUnit:...\n"), iErrorCode);
		jdePrintf(_J("***Error(%d): RetrieveCompanyFromBusUnit:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***RetrieveCompanyFromBusUnit (%ls)...\n"),dsRetrieveCompanyFromBusUnit.szCompany);
	jdePrintf(_J("***RetrieveCompanyFromBusUnit (%ls)...\n"),dsRetrieveCompanyFromBusUnit.szCompany);

	//***Busca Decimales de la Cantidad	...
	jdeStrcpy(dsGetDataDictionaryDefinition.szDataDictionaryItem,_J("UORG"));
	idResult = jdeCallObject(_J("GetDataDictionaryDefinition"),NULL,lpBhvrCom,lpVoid,
						(LPVOID)&dsGetDataDictionaryDefinition,(CALLMAP *)NULL,(int)0, 
						(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 625;
		jdeFprintf(dlg,_J("***Error(%d): GetDataDictionaryDefinition:...\n"), iErrorCode);
		jdePrintf(_J("***Error(%d): GetDataDictionaryDefinition:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***GetDataDictionaryDefinition (%ls)...\n"),dsGetDataDictionaryDefinition.mnDisplayDecimals.String);
//	MathNumericToInt(&dsGetDataDictionaryDefinition.mnDisplayDecimals, &iCantDecQty); ***No funciona en Rattan
	iCantDecQty = atoi(dsGetDataDictionaryDefinition.mnDisplayDecimals.String);

	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Inicializando datos encabezado...\n"));
	jdePrintf(_J("***Inicializando datos encabezado...\n"));

	// Fecha Contable
	if (jdeStrcmp(szDocGLDateBuf,_J("0")) == 0) FormatDate(szDocGLDateBuf, &dsGetAuditInfo.jdDate,(JCHAR*) NULL);
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szDocGLDateBuf (%ls)...\n"),szDocGLDateBuf);
	jdePrintf(_J("***szDocGLDateBuf (%ls)...\n"),szDocGLDateBuf);
	// Fecha Transaccion
	if (jdeStrcmp(szDocTransDateBuf,_J("0")) == 0) FormatDate(szDocTransDateBuf,&dsGetAuditInfo.jdDate,(JCHAR*) NULL);
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szDocTransDateBuf (%ls)...\n"),szDocTransDateBuf);
	jdePrintf(_J("***szDocTransDateBuf (%ls)...\n"),szDocTransDateBuf);
	// Ubicación por defecto del usuario

	// Determina Símbolo para identificar 3rd Inv Number...
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Determina dsGetCrossReferenceFields...\n"));
	jdeStrcpy((JCHAR *)dsGetCrossReferenceFields.szSystemCode,(const JCHAR *)_J(" "));
	jdeStrcpy((JCHAR *)dsGetCrossReferenceFields.szBranchPlant,(const JCHAR *)szDocBranchPlantBuf);
	dsGetCrossReferenceFields.cSuppressErrorMsg = _J('1');
	idResult = jdeCallObject(_J("GetCrossReferenceFields"), NULL,lpBhvrCom,lpVoid,
	                      (LPVOID)&dsGetCrossReferenceFields,(LPCALLMAP)NULL,(int)0,
						  (JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if((idResult == ER_ERROR) || (jdeStrcmp(dsGetCrossReferenceFields.szErrorMsgID,_J(" ")) != 0)){
		iErrorCode = 607;
		jdeErrorSetToFirstEx(lpBhvrCom, lpVoid);
		while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom,lpVoid)){
			jdeFprintf(dlg,_J("***Error(%d): GetCrossReferenceFields: (%ls)...\n"),iErrorCode,ErrorRec->lpszShortDesc);
			jdeFflush(dlg);
		} 
		goto lbFIN;
	}
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***dsGetCrossReferenceFields (%lc)...\n"),dsGetCrossReferenceFields.c3rdItemNoSymbol);
	jdePrintf(_J("***dsGetCrossReferenceFields (%lc)...\n"),dsGetCrossReferenceFields.c3rdItemNoSymbol);

	// Obtiene Moneda y Conversión de la compañia
	jdeStrcpy(dsF0010RetrieveCompanyConstant.szCompany,_J("00000"));
	idResult = jdeCallObject(_J("F0010RetrieveCompanyConstant"),NULL,lpBhvrCom,lpVoid,
							(LPVOID)&dsF0010RetrieveCompanyConstant,(CALLMAP *)NULL,(int)0, 
							(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
	if (idResult == ER_ERROR){
		iErrorCode = 608;
		jdeFprintf(dlg,_J("***Error(%d): F0010RetrieveCompanyConstant:...\n"), iErrorCode);
		jdePrintf(_J("***Error(%d): F0010RetrieveCompanyConstant:...\n"), iErrorCode);
		jdeFflush(dlg);
		goto lbFIN;
	} 
	if (iDbgFlg > 0){
		jdeFprintf(dlg,_J("***F0010RetrieveCompanyConstant (%lc/%ls)...\n"),
							dsF0010RetrieveCompanyConstant.cCurrencyConverYNAR,
							dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom);
		jdeFflush(dlg);
	}
	jdePrintf(_J("***F0010RetrieveCompanyConstant (%lc/%ls)...\n"),
							dsF0010RetrieveCompanyConstant.cCurrencyConverYNAR,
							dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom);

	// Allocate and set data structures
	iErrorCode = 0;
	jdeErrorClearEx(lpBhvrCom1,lpVoid1);
	memset((void *) &dsF4311BeginDoc,(int) _J('\0'),sizeof(DSDF4311Z1A));
	lpValidateUDC = &dsValidateUDC;

	memset (lpValidateUDC,(int)_J('\0'),sizeof(dsValidateUDC));
	memset (szReferenciaClienteBuf,(int)_J('\0'),sizeof(szReferenciaClienteBuf));

	//*************************************************
	//*** Procesamiento UDP...
	//*************************************************
    // ***Parse arguments and load Winsock

    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0){
        printf("WSAStartup failed!\n");
		iErrorCode =  111;
		if (iDbgFlg > 0) fprintf(dlg,"***Error(%d): WSAStartup failed...\n", iErrorCode);
		jdeFflush(dlg);
        return 1;
    }

	recvbuf = (char *) malloc(dwLength);
	sendbuf = (char *) malloc(dwLength);	
	if (!recvbuf || !sendbuf){	
		printf("malloc() failed: %d\n", GetLastError());
		iErrorCode =  112;
		if (iDbgFlg > 0) fprintf(dlg,"***Error(%d): malloc() failed: %d...\n", iErrorCode, GetLastError());
		jdeFflush(dlg);
		return 1;
	}

	memset((char*) &local, 0, sizeof(local));
	memset((char*) &remote, 0, sizeof(remote));

	iSalir = 0;
	iPrimeraVez = 1;
	iInitPhase = NumFasesInit;

	//*** Procesamiento de la comunicación...
	while (1){
		// Creamos el socket y unimos a la interface/puerto local ...
		//
		s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (s == INVALID_SOCKET){		
			printf("socket() failed en s1; %d\n", WSAGetLastError());
			iErrorCode =  113;
			if (iDbgFlg > 0) fprintf(dlg,"***Error(%d): socket() failed en s1; %d...\n", iErrorCode, WSAGetLastError());
			jdeFflush(dlg);
			return 1;
		}
		local.sin_family = AF_INET;
		local.sin_port = htons((short)ilPort);
		local.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(s, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR){
			printf("bind() failed: %d\n", WSAGetLastError());
			iErrorCode =  114;
			if (iDbgFlg > 0) fprintf(dlg,"***Error(%d): bind() failed: %d...\n", iErrorCode, WSAGetLastError());
			jdeFflush(dlg);
			return 1;
		}

		// Leemos datagramas...
		//
        dwRemoteSize = sizeof(remote);
		printf("Escuchando puerto: %d\n", ilPort);
		remote.sin_port = htons((short)irPort);

        ret = recvfrom(s, recvbuf, dwLength, 0, (SOCKADDR *)&remote, (int *)&dwRemoteSize);
        if (ret == SOCKET_ERROR){			
            printf("recvfrom() failed; %d\n", WSAGetLastError());
			iErrorCode =  115;
			if (iDbgFlg > 0) fprintf(dlg,"***Error(%d): recvfrom() failed; %d...\n", iErrorCode, WSAGetLastError());
			printf("Recibido Error de IP/msg: %s/'%s'\n", inet_ntoa(remote.sin_addr), recvbuf);
            goto lbCommErr;
        }
        else if (ret == 0)
            break;
		else{
			recvbuf[ret] = '\0';
			printf("Recibido de IP/msg: %s/'%s'\n", inet_ntoa(remote.sin_addr), recvbuf);
			if (iDbgFlg > 0) fprintf(dlg,"***Recibido de IP/msg: %s/'%s'\n", inet_ntoa(remote.sin_addr), recvbuf);

			//Determiamos el Tipo de Transaccion...
			//T01 - Maestro Productos para la inicialización...
			//T11 - Campos para Transacción OC...

			if (strncmp(recvbuf,"T01:",4) == 0){

			}
			if (strncmp(recvbuf, "T11:", 4) == 0){			
				// Descomponemos el mensaje en cliente/producto/cantidad...
				jdeToUnicode(szEtiquetaCodigo,recvbuf + 4,sizeof(szEtiquetaCodigo)-1,UTF8);
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Etiqueta (%ls)...\n"),szEtiquetaCodigo);
				jdePrintf(_J("***Etiqueta (%ls)...\n"),szEtiquetaCodigo);
				
				szDummy0 = jdeStrtok(szEtiquetaCodigo,(JCHAR *) cSeparadorEtiqueta);
				ParseNumericString(&mnCodigoProverdorBuff,szDummy0);
				szDummy0 = jdeStrtok(NULL,(JCHAR *) cSeparadorEtiqueta);
				if (szDummy0 != NULL) {
					jdeStrncpy(szLineItemBuf,szDummy0,26);
					if (jdeStrcmp(szLineItemBuf, _J("999")) == 0)  goto lbEndDoc;
					szDummy0 = jdeStrtok(NULL,(JCHAR *) cSeparadorEtiqueta);
					if (szDummy0 != NULL) {
						ParseNumericString(&mnCantIntroducidaBuf,szDummy0);
					}
					else{
						iErrorCode = 308;
						jdeToUnicode(szTempBuf,szCamposContenido[0],sizeof(szEtiquetaCodigo)-1,UTF8);
						jdeFprintf(dlg,_J("***Error(%d): Etiqueta Invalida: %ls...\n"),szTempBuf);
						jdePrintf(_J("***Error(%d): Etiqueta Invalida: %ls...\n"),szTempBuf);
						jdeFflush(dlg);
					}
				}
				else {
					iErrorCode = 308;
					jdeToUnicode(szTempBuf,szCamposContenido[0],sizeof(szEtiquetaCodigo)-1,UTF8);
					jdeFprintf(dlg,_J("***Error(%d): Etiqueta Invalida: %ls...\n"),szTempBuf);
					jdePrintf(_J("***Error(%d): Etiqueta Invalida: %ls...\n"),szTempBuf);
					jdeFflush(dlg);
				}

				jdeToUnicode(LszString01,mnCodigoProverdorBuff.String,DIM(LszString01),UTF8);
				jdeToUnicode(LszString02,mnCantIntroducidaBuf.String,DIM(LszString02),UTF8);
				if (iDbgFlg > 0){
					jdeFprintf(dlg,_J("***Etiqueta ((%ls)%ls/%ls/%ls)...\n"),cSeparadorEtiqueta,
						LszString01,szLineItemBuf,LszString02);
					jdeFflush(dlg);
				}
				jdePrintf(_J("***Etiqueta ((%ls)%ls/%ls/%ls)...\n"),cSeparadorEtiqueta,
						LszString01,szLineItemBuf,LszString02);

				jdeFflush(dlg);
			} //if (strncmp(recvbuf, "T11:", 4)
		}

		if (strncmp(recvbuf, "T01:", 4) == 0){//Transacción construcción maestro productos...
			//*************************************************
			//*** Procesamiento Maestro Productos...
			//*************************************************

			if (iInitPhase == 1){//Inicializacion construcción maestro productos...
				memset((void *) &dsProductosArr, '\0', CacheSize * 50);
				iInitPhase = 0;

				if (iDbgFlg > 0) {
				jdeFprintf(dlg,_J("***Procesando Maestro Productos... \n"));
				jdeFflush(dlg);
				}

				idResult = JDB_OpenTable(hUser,NID_F4101,ID_F4101_ITEM_MASTER,szF4101DescrColumnsArray,(ushort)(3),
										(JCHAR *)NULL,&hRequestF4101);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 630;
					jdeFprintf (dlg,_J("***Error(%d): JDB_OpenTable(F4101) failed...\n"),iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}
				
				//Construye Where del select...
				ZeroMathNumeric(&dsF4101Key1.imitm); //inicializacmos la clave

				JDB_ClearSelection(hRequestF4101);
				jdeNIDcpy(lpSelect[0].Item1.szDict, NID_ITM);//Codigo UPC == szUsrEntry
				jdeNIDcpy(lpSelect[0].Item1.szTable, NID_F4101);
				lpSelect[0].Item1.idInstance = (ID)0;
				jdeNIDcpy(lpSelect[0].Item2.szDict, _J(""));
				jdeNIDcpy(lpSelect[0].Item2.szTable, _J(""));
				lpSelect[0].Item2.idInstance = (ID)0;
				lpSelect[0].lpValue = &dsF4101Key1.imitm;
				lpSelect[0].nValues = (short)1;
				lpSelect[0].nAndOr = JDEDB_ANDOR_AND;
				lpSelect[0].nCmp = JDEDB_CMP_GT;

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Construyendo Selection Tabla F4101..\n"));	
				idResult = JDB_SetSelection(hRequestF4101,lpSelect,(short)1,JDEDB_SET_REPLACE);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 631;
					jdeFprintf (dlg,_J("***Error(%d): JDB_SetSelection(F4101) failed...\n"),iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}				
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Ejecutando Select sobre Tabla F4101...\n"));
				idResult = JDB_SelectKeyed(hRequestF4101,(ID) 0,(void *)NULL,(short)0);
				if (idResult == JDEDB_FAILED){
					iErrorCode = 632;
					jdeFprintf (dlg,_J("***Error(%d): JDB_SelectKeyed(F4101) failed...\n"),iErrorCode);
					jdeFflush(dlg);
					goto lbFIN;
				}

			}

			//Se construye el arreglo y se manda...
			int i = 0;
			int finIndicador = 0;
			idResult = JDEDB_PASSED;				
			
			while ((idResult != JDEDB_FAILED) && (i < 100)){				
					
				idResult = JDB_Fetch(hRequestF4101,&dsF4101Descr1,(int)0);
				if (idResult != JDEDB_FAILED){
					jdeFromUnicode(dsF4101Descr2.codigo,dsF4101Descr1.imlitm,20,UTF8);
					jdeFromUnicode(dsF4101Descr2.descripcion,dsF4101Descr1.imdsc1,30,UTF8);
					dsF4101Descr2.codigo[20] = '\0';
					dsF4101Descr2.descripcion[30] = '\0';
				}
				if (iDbgFlg > 0) fprintf(dlg,"***Ejecutando Fetch sobre Tabla F4101...(%d) (%s)(%s)\n", i, 
										 dsF4101Descr2.codigo, dsF4101Descr2.descripcion);
				strncpy(dsProductosArr[i], dsF4101Descr2.codigo, 20);
				strncpy(dsProductosArr[i] + 20, dsF4101Descr2.descripcion, 30);
				strcpy(dsProductosArr[i] + 50, "");
				i++;

				jdeFflush(dlg);
			}
			i--;
			if (idResult == JDEDB_FAILED){
				finIndicador = 1;
			}

			// Ahora contestamos al cliente...
			//
			// Respondemos al remoto...
			sprintf(sendbuf,"T01:%d%02d%s\0",finIndicador,i,dsProductosArr);
			dwLength1 = strlen(sendbuf) + 1;

			ret = sendto(s, sendbuf, dwLength1, 0, (SOCKADDR *)&remote, sizeof(remote));


			printf("Enviando puerto/IP: %d/%s  (%s)\n", irPort, inet_ntoa(remote.sin_addr), sendbuf);
			
			char sendbuf1[128];
			strncpy(sendbuf1, sendbuf, 100);
			sendbuf1[100] = '\0';
			if (iDbgFlg > 0) fprintf(dlg,"***Enviando puerto/IP: %d/%s  (%s)\n", irPort, inet_ntoa(remote.sin_addr), sendbuf1);
				jdeFflush(dlg);
			if (ret == SOCKET_ERROR){
				printf("sendto() failed; %d\n", WSAGetLastError());
				iErrorCode =  117;
				if (iDbgFlg > 0) fprintf(dlg,"***Error(%d): sendto() failed; %d...\n", iErrorCode, WSAGetLastError());
				jdeFflush(dlg);
				break;
			}
			else if (ret == 0) break;

		}

		if (strncmp(recvbuf, "T11:", 4) == 0){
			//*************************************************
			//*** Procesamiento Pedidos JDE...
			//*************************************************
			if (iPrimeraVez == 1) {
				iPrimeraVez++;

				//***
				//***Comienza Procesamiento del Encabezado                            
				memset((void *) &dsF4311BeginDoc,(int) _J('\0'),sizeof(DSDF4311Z1A));
				memset((void *) &dsF4311EditLine,(int) _J('\0'),sizeof(DSDF4311Z1B));
				memset((void *) &dsF4311EndDoc,(int) _J('\0'),sizeof(DSDF4311Z1D));
				memset((void *) &dsF4311ClearWorkFiles,(int) _J('\0'),sizeof(DSDF4311Z1E));	
				memset((void *) &dsDecimalsTriggerGetbyCOCRCD,(int) _J('\0'),sizeof(DSD1100007));
				memset((void *) &dsDecimalsTriggerGetbyCOCRCD,(int) _J('\0'),sizeof(DSD1100007));
				memset((void *) &szReferenciaClienteBuf,(int) _J('\0'),sizeof(szReferenciaClienteBuf));
				memset((void *) &dsIsColumnInAddressBook,(int) _J('\0'),sizeof(dsIsColumnInAddressBook));

				//*************************************************************************
				//***Procesa F4311FSBeginDoc                                            ***
				//*************************************************************************

				//Asignaciones para F4311FSBeginDoc:
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**Asignado dsF431BeginDoc...\n"));
				jdePrintf(_J("**Asignado dsF431BeginDoc...\n"));
				jdeFflush(dlg);

				ZeroMathNumeric(&dsF4311BeginDoc.mnJobNumber);									// mnJobNumber;	    
				jdeStrcpy (dsF4311BeginDoc.szComputerID,dsGetAuditInfo.szWorkstation_UserId);	// szComputerID[16];           
				dsF4311BeginDoc.cHeaderActionCode = _J('A');											// cHeaderActionCode;
				dsF4311BeginDoc.cProcessEdits = _J('1');										// cProcessEdits;
				dsF4311BeginDoc.cUpdateOrWriteToWorkFile = _J('2');								// cUpdateOrWriteToWorkFile;
				dsF4311BeginDoc.cCurrencyProcessingFlag = _J('0');								// cRecordWrittenToWorkFile;
				//  JCHAR			cCurrencyProcessingFlag;										---            
				jdeStrcpy(dsF4311BeginDoc.szOrderCOmpany,dsRetrieveCompanyFromBusUnit.szCompany);// szOrderCOmpany[6];       
				//  MATH_NUMERIC	mnOrderNumber;												ok	NN
				jdeStrcpy(dsF4311BeginDoc.szOrderType,szPOOrderTypeBuf);						// szOrderType[3];
				jdeStrcpy(dsF4311BeginDoc.szOrderSuffix,_J("000"));								// szOrderSuffix[4];
				jdeStrcpy(dsF4311BeginDoc.szBranchPlant,dsRetrieveCompanyFromBusUnit.szCostCenter);// szBranchPlant[13];                   
				//  JCHAR			szOriginalOrderCompany[6];										           
				//  JCHAR			szOriginalOrderNumber[9];										            
				//  JCHAR			szOriginalOrderType[3];              
				//  JCHAR			szRelatedOrderCompany[6];            
				//  JCHAR			szRelatedOrderNumber[9];             
				//  JCHAR			szRelatedOrderType[3];               
				MathCopy(&dsF4311BeginDoc.mnSupplierNumber,&mnCodigoProverdorBuff);				// mnSupplierNumber;  
				//  MATH_NUMERIC	mnShipToNumber;
				DeformatDate(&dsF4311BeginDoc.jdRequestedDate,szDocTransDateBuf,(JCHAR*) NULL);	// jdRequestedDate;	                 
				//  JDEDATE			jdOrderDate;				Asignado por la aplicación		ok                         
				//  JDEDATE			jdPromisedDate;                      
				//  JDEDATE			jdCancelDate; 
				//jdeStrcpy(dsF4311BeginDoc.szReference01,szReferenciaClienteBuf);				// szReference01[26] - Referencia Cliente                  
				jdeStrcpy(dsF4311BeginDoc.szReference01,_J("Prueba UDPSvr"));				// szReference01[26] - Referencia Cliente                  
				//  JCHAR			szReference02[26];                   
				//  JCHAR			szDeliveryInstructions01[31];        
				//  JCHAR			szDeliveryInstructions02[31];        
				//  JCHAR			szPrintMessage[11];                  
				//  JCHAR			szSupplierPriceGroup[9];             
				//  JCHAR			szPaymentTerms[4];                   
				//  JCHAR			szTaxExplanationCode[3];             
				//  JCHAR			szTaxRateArea[11];                   
				//  JCHAR			szTaxCertificate[21];                
				//  JCHAR			cAssociatedText;                     
				//  JCHAR			szHoldCode[3];                       
				//  JCHAR			szFreightHandlingCode[4];            
				//  MATH_NUMERIC	mnBuyerNumber;                       
				//  MATH_NUMERIC	mnCarrierNumber;                     
				//  JCHAR			cEvaluatedReceiptsFlag;              
				//  JCHAR			cSendMethod;                         
				//  JCHAR			szLandedCostRule[4];                 
				//  JCHAR			szApprovalRouteCode[13];             
				//  MATH_NUMERIC	mnChangeOrderNumber; 
		//		dsF4311BeginDoc.cCurrencyMode = _J('D');											// cCurrencyMode - 'D'
		//		jdeStrcpy(dsF4311BeginDoc.szTransactionCurrencyCode,dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom);// szTransactionCurrencyCode[4]	        
				//  MATH_NUMERIC	mnCurrencyExchangeRate;              
				//  JCHAR			szOrderedPlacedBy[11];               
				//  JCHAR			szOrderTakenBy[11];
				jdeStrcpy(dsF4311BeginDoc.szProgramID,_J("EP4310"));								// szProgramID[11]                     
				//  JCHAR			szApprovalRoutePO[26];               
				jdeStrcpy(dsF4311BeginDoc.szPurchaseOrderPrOptVersion,szP4310VersionBuf);			// szPurchaseOrderPrOptVersion[11]
				jdeStrcpy(dsF4311BeginDoc.szBaseCurrencyCode,dsF0010RetrieveCompanyConstant.szCurrencyCodeFrom);// szBaseCurrencyCode[4]                
				jdeStrcpy(dsF4311BeginDoc.szUserID,dsGetAuditInfo.szUserName);						// szUserID[11]                        
				//  JCHAR			cAddNewLineToExistingOrder;          
				//  ID				idInternalVariables;                 
				//  JCHAR			cSourceOfData;                       
				//  MATH_NUMERIC	mnSODOrderNumber;                    
				//  JCHAR			szSODOrderType[3];                   
				//  JCHAR			szSODOrderCompany[6];                
				//  JCHAR			szSODOrderSuffix[4];                 
				//  MATH_NUMERIC	mnRetainage;                         
				//  JCHAR			szDescription[31];                   
				//  JCHAR			szRemark[31];                        
				//  JDEDATE			jdEffectiveDate;                     
				//  JDEDATE			jdPhysicalCompletionDate;            
				//  MATH_NUMERIC	mnTriangulationRateFromCurrenc;      
				//  MATH_NUMERIC	mnTriangulationRateToCurrency;       
				//  JCHAR			cCurrencyConversionMethod;           
				//  JCHAR			szPriceAdjustmentScheduleN[9];       
				//  JCHAR			cAIADocument;                        
				//  MATH_NUMERIC	mnProcessID;                         
				//  MATH_NUMERIC	mnTransactionID;                     
				//  MATH_NUMERIC	mnRMADocNumber;                      
				//  JCHAR			szRMADocType[3];                     
				//  JCHAR			szPOEMBFInternalFlags[11];           
				//  JDEDATE			jdUserReservedDate;                  
				//  MATH_NUMERIC	mnUserReservedAmount;                
				//  JCHAR			szUserReservedCode[3];               
				//  JCHAR			szUserReservedReference[16];           
				//  MATH_NUMERIC	mnUserReservedNumber;


				//***F4311BeginDoc  
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**F4311BeginDoc...\n"));	
				jdePrintf(_J("**F4311BeginDoc...\n"));
				idResult = jdeCallObject(_J("F4311FSBeginDoc"),NULL,lpBhvrCom1,lpVoid1, 
										(void *)&dsF4311BeginDoc,(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
					if (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ){		
						iErrorCode = 613;
						jdeFprintf(dlg,_J("***Error(%d): F4311BeginDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						jdeFflush(dlg);

						jdePrintf(_J("***Error(%d): F4311BeginDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					}
					else{
						jdeFprintf(dlg,_J("***Warn(%d): F4311BeginDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						jdeFflush(dlg);

						jdePrintf(_J("***Warn(%d): F4311BeginDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					}
				}
				if ((idResult == ER_ERROR) && (iErrorCode == 0)){
					iErrorCode = 613;
					jdeFprintf(dlg,_J("***Error(%d): F4311BeginDoc...\n"),iErrorCode);
					jdeFflush(dlg);

					jdePrintf(_J("***Error(%d): F4311BeginDoc...\n"),iErrorCode);
				}
				iEditLineLines = 0; // No hay líneas en el documento de salida...
			}
			jdeFflush(dlg);
			if (iErrorCode == 0) { // BeginDoc está OK, seguir con EditLine y tal vez EndDoc ...	
				//*************************************************************************
				//***Comienza Procesamiento del Detalle                                 ***
				//*************************************************************************

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**Comieza Procesamiento Detalle...\n"));
				jdePrintf(_J("**Comieza Procesamiento Detalle...\n"));
				jdeFflush(dlg);

				memset((void *) &szLineLotBuf,(int) _J('\0'),sizeof(szLineLotBuf));//Limpiamos por primera vez....
				memset((void *) &dsF4311EditLine,(int) _J('\0'),sizeof(DSDF4311Z1B));
				//memset((void *) &mnCantIntroducidaBuf,(int) _J('\0'),sizeof(MATH_NUMERIC));

				//***Codigo Producto...				
				jdeStrcpy(szTempBuf,_J("                         "));//ItemMasterItem Left Justified padded with blanks
				jdeStrcat(szLineItemBuf,szTempBuf + jdeStrlen(szLineItemBuf));
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szLineItemBuf(%ls)...\n"),szLineItemBuf);
				//Procesa Codigo de Barras
	//			szLineItemBuf[0] = '\0';
	//			szItemDescriptionBuf[0] = '\0';

				//***La cantidad...

				//***La UM...
				jdeStrcpy(szLineUMBuf,szLineUnitOfMeasureBuf);
	 
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("***szLineUMBuf(%s)...\n"),szLineUMBuf);		
				jdePrintf(_J("***szLineUMBuf(%s)...\n"),szLineUMBuf);

				//***Ubicación...
				if(jdeStrlen(szUbicacionDfltBuf) != 0){ //Procesamiento de Ubicacion Default...
					jdeStrcpy(szLineLocationBuf,szUbicacionDfltBuf);
				}
				else{
					jdeStrcpy(szLineLocationBuf,_J(""));
				}

				//***Lotes...


				//*************************************************************************
				//***F4311EditLine                                                      ***
				//*************************************************************************		
				//Asignaciones para F4311EditLine:
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**Asignado dsF4311EditLine...\n"));
				jdePrintf(_J("**Asignado dsF4311EditLine...\n"));
				
				MathCopy(&dsF4311EditLine.mnJobNumber,&dsF4311BeginDoc.mnJobNumber);		// mnJobNumber                         
				jdeStrcpy (dsF4311EditLine.szComputerID,dsF4311BeginDoc.szComputerID);		// szComputerID[16]                    
				//  MATH_NUMERIC      mnOrderLineNumber;                   
				dsF4311EditLine.cDetailActionCode = _J('A');								// cDetailActionCode;                   
				dsF4311EditLine.cProcessEdits = _J('1');									// cProcessEdits;                       
				dsF4311EditLine.cUpdateOrWriteWorkFile = _J('2');							// cUpdateOrWriteWorkFile;              
				//  JCHAR             cRecordWrittenToWorkFile;								// OK           
				//  JCHAR             cSuppressErrorMessages;              
				//  JCHAR             cCurrencyProcessingFlag;             
				jdeStrcpy(dsF4311EditLine.szPurchaseOrderPrOptVersion,dsF4311BeginDoc.szPurchaseOrderPrOptVersion);// szPurchaseOrderPrOptVersion[26];     
				//  JCHAR             szOrderCompany[6];                   
				//  MATH_NUMERIC      mnOrderNumber;                       
				//  JCHAR             szOrderType[3];                      
				//  JCHAR             szOrderSuffix[4];                    
				//  JCHAR             szBranchPlant[13];                   
				//  JCHAR             szOriginalOrderCompany[6];           
				//  JCHAR             szOriginalOrderNumber[9];            
				//  JCHAR             szOriginalOrderType[3];              
				//  MATH_NUMERIC      mnOriginalLineNumber;                
				//  JCHAR             szRelatedOrderCompany[6];            
				//  JCHAR             szRelatedOrderNumber[9];             
				//  JCHAR             szRelatedOrderType[3];               
				//  MATH_NUMERIC      mnRelatedOrderLine;                  
				//  MATH_NUMERIC      mnSupplierNumber;                    
				//  MATH_NUMERIC      mnShipToNumber;                      
				//  JDEDATE           jdRequestedDate;                     
				//  JDEDATE           jdTransactionDate;                   
				//  JDEDATE           jdPromisedDate;                      
				//  JDEDATE           jdCancelDate;                        
				//  JDEDATE           jdGLDate;                            
				jdeStrcpy(dsF4311EditLine.szUnformattedItemNumber,szLineItemBuf);			// szUnformattedItemNumber[27];         
				MathCopy(&dsF4311EditLine.mnQuantityOrdered,&mnCantIntroducidaBuf);			// mnQuantityOrdered;                   
				ZeroMathNumeric(&dsF4311EditLine.mnUnitPrice);								// mnUnitPrice;                         
				//  JCHAR             cPriceOverrideFlag;                  
				ZeroMathNumeric(&dsF4311EditLine.mnExtendedPrice);							// mnExtendedPrice;                     
				//  JCHAR             szLineType[3];                       
				//  JCHAR             szDescription1[31];                  
				//  JCHAR             szDescription2[31];                  
				//  JCHAR             szAssetID[26];                       
				//  JCHAR             szDetailLineBranchPlant[13];         
				jdeStrcpy(dsF4311EditLine.szLocation,szLineLocationBuf);					// szLocation[21];                      
				jdeStrcpy(dsF4311EditLine.szLotNumber,szLineLotBuf);						// szLotNumber[31];                     
				jdeStrcpy(dsF4311EditLine.szTransactionUoM,szLineUMBuf);					// szTransactionUoM[3];                 
				//  JCHAR             szPurchasingUoM[3];                  
				jdeStrcpy(dsF4311EditLine.szLastStatus,szBeginningStatusBuf);				// szLastStatus[4];                     
				//  JCHAR             szNextStatus[4];                     
				//  JCHAR             szUnformattedAccountNumber[30];      
				//  JCHAR             szSubledger[9];                      
				//  JCHAR             cSubledgerType;                      
				//  MATH_NUMERIC      mnDiscountFactor;                    
				//  JCHAR             szCatalogNumber[9];                  
				//  JCHAR             szInventoryPriceRule[9];             
				//  JCHAR             szPriceLevel[4];                     
				//  JCHAR             szPrintMessage[11];                  
				//  JCHAR             cTaxable;                            
				//  JCHAR             szTaxExplanationCode[3];             
				//  JCHAR             szTaxRateArea[11];                   
				//  JCHAR             szGLClassCode[5];                    
				//  MATH_NUMERIC      mnBuyerNumber;                       
				//  MATH_NUMERIC      mnCarrierNumber;                     
				//  JCHAR             szPurchasingCategoryCode1[4];        
				//  JCHAR             szPurchasingCategoryCode2[4];        
				//  JCHAR             szPurchasingCategoryCode3[4];        
				//  JCHAR             szPurchasingCategoryCode4[4];        
				//  JCHAR             szLandedCostRule[4];                 
				//  MATH_NUMERIC      mnWeight;                            
				//  JCHAR             szWeightUoM[3];                      
				//  MATH_NUMERIC      mnVolume;                            
				//  JCHAR             szVolumeUoM[3];                      
 				//  JCHAR             szReference1[26];                    
 				//  JCHAR             szReference2[26];                    
				//  JCHAR             cSendMethod;                         
				//  JCHAR             cFreezeCode;                         
				//  JCHAR             cEvaluatedReceipts;                  
				//  JCHAR             cInventoryInterface;                 
				//  JCHAR             cCurrencyMode;                       
				//  JCHAR             szTransactionCurrencyCode[4];        
				//  MATH_NUMERIC      mnCurrencyExchangeRate;              
				//  JCHAR             szBaseCurrencyCode[4];               
				//  JCHAR             cRelievePOBlanketOrder;              
				//  JCHAR             cAddNewLineToExistingOrder;          
				//  JCHAR             cSourceRequestingPOGeneration;       
				jdeStrcpy(dsF4311EditLine.szProgramID,dsF4311BeginDoc.szProgramID);			// szProgramID[11];                     
				jdeStrcpy(dsF4311EditLine.szUserID,dsF4311BeginDoc.szUserID);				// szUserID[11];                        
				//  JCHAR             cMultipleBlanketProcessing;          
				//  MATH_NUMERIC      mnUniqueKey;                         
				//  JCHAR             cSourceOfDataFlag;                   
				//  MATH_NUMERIC      mnSourceOfDataOrderNumber;           
				//  JCHAR             szSourceOfDataOrderType[3];          
				//  JCHAR             szSourceOfDataCompany[6];            
 				//  MATH_NUMERIC      mnSourceOfDataLineNumber;            
				//  MATH_NUMERIC      mnSourceOfDataUnitsOpen;             
				//  JCHAR             szPaymentTerms[4];                   
				//  JCHAR             szFreightHandlingCode[4];            
				//  JCHAR             szUserReservedCode[3];               
				//  JDEDATE           jdUserReservedDate;                  
				//  MATH_NUMERIC      mnUserReservedAmount;                
				//  MATH_NUMERIC      mnUserReservedNumber;                
				//  JCHAR             szUserReservedReference[16];         
				//  JCHAR             szAgreementNumber[13];               
 				//  MATH_NUMERIC      mnAgreementSupplement;               
 				//  MATH_NUMERIC      mnContractsFound;                    
 				//  MATH_NUMERIC      mnSequenceNumber1;                   
 				//  JCHAR             szProductSource[13];                 
 				//  JCHAR             szProductSourceType[3];              
				//  JDEDATE           jdEffectiveDate;                     
				//  JDEDATE           jdPhysicalCompletionDate;            
				//  JCHAR             szPurchasingCostCenter[13];          
				//  JCHAR             szObjectAccount[7];                  
				//  JCHAR             szSubsidiary[9];                     
				//  JCHAR             szOriginalOrderSuffix[4];            
				//  JCHAR             cMessageCde;                         
				//  JCHAR             szMaterialStatusCodeWo[3];           
 				//  JCHAR             cStockingType;                       
 				//  JCHAR             cKitsMethodofPriceCalculation;       
 				//  JCHAR             szItemNumberRelatedKit[9];           
 				//  MATH_NUMERIC      mnLineNum;                           
				//  JCHAR             szAdjustmentSchedule[9];             
				//  MATH_NUMERIC      mnAdjustmentRevisionLevel;           
				//  JCHAR             cFreeGoodIndicator;                  
				//  JCHAR             cAdvPriceCalledFlag;                 
 				//  JCHAR             szOriginalPoSoNumber_2[9];           
				//  JCHAR             szRelatedPoSoNumber_2[9];             
				//  JCHAR             cManagerialAnalysisType1;            
				//  JCHAR             szManagerialAnalysisCode1[13];       
 				//  JCHAR             cManagerialAnalysisType2;            
 				//  JCHAR             szManagerialAnalysisCode2[13];       
 				//  JCHAR             cManagerialAnalysisType3;            
				//  JCHAR             szManagerialAnalysisCode3[13];       
				//  JCHAR             cManagerialAnalysisType4;            
				//  JCHAR             szManagerialAnalysisCode4[13];       
				//  MATH_NUMERIC      mnLineNumberXREF;                    
				//  JCHAR             cTransferOrder;                      
				//  MATH_NUMERIC      mnShipmentNumber;                    
				MathCopy(&dsF4311EditLine.mnProcessID,&dsF4311BeginDoc.mnProcessID);		// mnProcessID;                         
				MathCopy(&dsF4311EditLine.mnTransactionID,&dsF4311BeginDoc.mnTransactionID);// mnTransactionID;                     
				//  JCHAR             szItemFlashMessage[3];               
				//  JCHAR             cAmountRecNotZero;                   
				//  MATH_NUMERIC      mnIdentifierShortItem;               
 				//  MATH_NUMERIC      mnRMADocNumber;                      
 				//  JCHAR             szRMADocType[3];                     
 				//  MATH_NUMERIC      mnSecondaryQty;                      
				//  JCHAR             szSecondaryUOM[3];                   
				//  JDEDATE           jdLotEffectivityDate;                
				//  JCHAR             szPromotionID[13];                   
				//  MATH_NUMERIC      mnMatrixControlLine;                 
				//  MATH_NUMERIC      mnProjectNumber;                     
				//  JCHAR             cActiveFlag;                         
 				//  JCHAR             szPOEMBFInternalFlags[11];           
 				//  JCHAR             szModeOfTransport[4];                
 				//  MATH_NUMERIC      mnTaskOrderNumber;                   
 				//  JCHAR             szTaskOrderType[3];                  
 				//  JCHAR             szPlan[5];                           
				//  JCHAR             szElevation[4];                      
				//  JCHAR             cPreservePOEManualAdj;               
 				//  JCHAR             cPOETriggerReprice;


				//***F4311EditLine 
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**dsF4311EditLine...\n"));
				jdePrintf(_J("**dsF4311EditLine...\n"));
				idResult = jdeCallObject( _J("F4311EditLine"),(LPFNBHVR)NULL,lpBhvrCom1,lpVoid1,&dsF4311EditLine,
										(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);
				jdeErrorSetToFirstEx(lpBhvrCom1, lpVoid1);

				if (iDbgFlg > 0) {
	//				FormateaDecimalesCantidad(LszString01,dsF4311EditLine.mnQuantityOrdered.String,iCantDecQty,iDbgFlg,dlg);
					jdeToUnicode(LszString02,dsF4311EditLine.mnQuantityOrdered.String,DIM(LszString02),UTF8);
					jdeFprintf(dlg,_J("***Producto (%ls) Cantidad (%ls/%ls) Ubicacion(%ls/%ls)...\n"),
						dsF4311EditLine.szUnformattedItemNumber,LszString02,dsF4311EditLine.szTransactionUoM,
						dsF4311EditLine.szLocation,dsF4311EditLine.szLotNumber);
					jdeFflush(dlg);
				}
				jdePrintf(_J("***Producto (%ls) Cantidad (%ls/%ls) Ubicacion(%ls/%ls)...\n"),
						dsF4311EditLine.szUnformattedItemNumber,LszString02,dsF4311EditLine.szTransactionUoM,
						dsF4311EditLine.szLocation,dsF4311EditLine.szLotNumber);

				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)) {
					if (jdeStrncmp(ErrorRec->lpszShortDesc, _J("Warning:"), 8) != 0 ){	
						iErrorCode = 616;
						jdeFprintf(dlg,_J("***Error(%d): F4311EditLine: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						jdeFflush(dlg);
						jdePrintf(_J("***Error(%d): F4311EditLine: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					}
					else{
						jdeFprintf(dlg,_J("***Warn(%d): dsF4311EditLine: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
						jdeFflush(dlg);
						jdePrintf(_J("***Warn(%d): dsF4311EditLine: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					}
				}
				if ((idResult == ER_ERROR) && (iErrorCode == 0)){
					iErrorCode = 616;
					jdeFprintf(dlg,_J("***Error(%d): F4311EditLine...\n"),iErrorCode);
					jdeFflush(dlg);
					jdePrintf( _J("***Error(%d): F4311EditLine...\n"),iErrorCode);
				}						
				if (iErrorCode == 0){

					iEditLineLines = 1; //Por lo menos hay una línea OK...
				}
			} //if (iErrorCode == 0) // BeginDoc está OK, seguir con EditLine y tal vez EndDoc ...
			else{// BeginDoc está MAL, repetir BeginDoc ...
				iPrimeraVez = 1;
			}

			// Ahora contestamos al cliente...
			//
			// Respondemos al remoto...
			strcpy(sendbuf, "RECEIVED");
			dwLength1 = strlen(sendbuf) + 1;

			ret = sendto(s, sendbuf, dwLength1, 0, (SOCKADDR *)&remote, sizeof(remote));
			printf("Enviando puerto/IP: %d/%s  RECEIVED\n", irPort, inet_ntoa(remote.sin_addr));
			if (iDbgFlg > 0) fprintf(dlg,"***Enviando puerto/IP: %d/%s  RECEIVED\n", irPort, inet_ntoa(remote.sin_addr));
			if (ret == SOCKET_ERROR){
				printf("sendto() failed; %d\n", WSAGetLastError());
				iErrorCode =  117;
				if (iDbgFlg > 0) fprintf(dlg,"***Error(%d): sendto() failed; %d...\n", iErrorCode, WSAGetLastError());
				jdeFflush(dlg);
				break;
			}
			else if (ret == 0) break;

	lbEndDoc:
			if ((jdeStrcmp(szLineItemBuf, _J("999")) == 0) && (iEditLineLines == 1)){ // Estamos procesando el EndDoc...

				//*************************************************************************
				//Procesa F4311EndDoc                                                   ***
				//*************************************************************************	

				//Asignaciones para F4311EndDoc:
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**Asignado F4311EndDoc...\n"));
				jdePrintf(_J("**Asignado F4311EndDoc...\n"));

				jdeStrcpy(dsF4311EndDoc.szComputerID,dsF4311BeginDoc.szComputerID);		// szComputerID[16];                    
				MathCopy(&dsF4311EndDoc.mnJobNumber,&dsF4311BeginDoc.mnJobNumber);		// mnJobNumber;                         
				//  JCHAR              szRelatedOrderCompany[6];            
				//  JCHAR              szRelatedOrderNumber[9];             
				//  JCHAR              szRelatedOrderType[3];               
				jdeStrcpy(dsF4311EndDoc.szCallingApplicationName,_J("EP4310"));			// szCallingApplicationName[11];        
				jdeStrcpy(dsF4311EndDoc.szVersion,dsF4311BeginDoc.szPurchaseOrderPrOptVersion);// szVersion[11];                       
				jdeStrcpy(dsF4311EndDoc.szUserID,dsF4311BeginDoc.szUserID);				// szUserID[11];                        
				//  MATH_NUMERIC       mnOrderNumberAssigned;							ok !!!              
				dsF4311EndDoc.cUseWorkFiles = _J('2');									// cUseWorkFiles;                       
				//  JCHAR              cConsolidateLines;                   
				//  JCHAR              cRetainHeaderWorkFile;               
				//  JCHAR              cSourceRequestingPOGeneration;       
				MathCopy(&dsF4311EndDoc.mnProcessID,&dsF4311BeginDoc.mnProcessID);		// mnProcessID;                         
				MathCopy(&dsF4311EndDoc.mnTransactionID,&dsF4311BeginDoc.mnTransactionID);// mnTransactionID;                     
				//  JCHAR              szPOEMBFInternalFlags[11];           
				//  MATH_NUMERIC       mnROSI;                              
				//  MATH_NUMERIC       mnTaskOrderNumber;                   
				//  JCHAR              szTaskOrderType[3];    
				
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**F4311EndDoc...\n"));
				jdePrintf(_J("**F4311EndDoc...\n"));
				idResult = jdeCallObject(_J("F4311EndDoc"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsF4311EndDoc,
								(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
				jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
					iErrorCode = 618;
					jdeFprintf(dlg,_J("***Error(%d): F4311EndDoc: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				} 				
				if ((idResult == ER_ERROR) && (iErrorCode == 0)){
					iErrorCode = 618;
					jdeFprintf(dlg,_J("***Error(%d): F4311EndDoc...\n"),iErrorCode);
					jdeFflush(dlg);
				}
				if (iErrorCode != 0) goto lbFIN;

				if (iDbgFlg > 0) {
					jdeToUnicode(LszString01,dsF4311EndDoc.mnOrderNumberAssigned.String,DIM(LszString01),UTF8);
					jdeFprintf(dlg,_J("***mnOrderNumberAssigned (%ls)...\n"),LszString01);
					jdeFflush(dlg);
				}
				jdePrintf(_J("***mnOrderNumberAssigned (%ls)...\n"),LszString01);

				//*************************************************************************
				//Procesa F4311ClearWorkFiles                                           ***
				//*************************************************************************				
				//Asignaciones para F4311ClearWorkFiles:
				//· szComputerID												ok GetAuditInfo
				//· mnJobNumber				No Asigndo							ok F4311BeginDoc
				//· cClearHeaderFile											ok 1
				//· cClearDetailFile											ok 1
				//· cErrorInClear			No Asignado							--
				//· mnLineNumber			No Asignado							--
				//· cUseWorkFiles												ok 2
				//· szOrderSuffix			No Asignado							--
				//· mnProcessID													¡!
				//· mnTransactionID												¡!

				jdeStrcpy(dsF4311ClearWorkFiles.szComputerID,dsF4311BeginDoc.szComputerID);
				MathCopy(&dsF4311ClearWorkFiles.mnJobNumber,&dsF4311BeginDoc.mnJobNumber);
				dsF4311ClearWorkFiles.cClearHeaderFile = _J('1');
				dsF4311ClearWorkFiles.cClearDetailFile = _J('1');
				//· cErrorInClear			No Asignado							
				//· mnLineNumber			No Asignado	
				dsF4311ClearWorkFiles.cUseWorkFiles = _J('1');
				//· szOrderSuffix
				MathCopy(&dsF4311ClearWorkFiles.mnProcessID,&dsF4311BeginDoc.mnProcessID);// mnProcessID;                         
				MathCopy(&dsF4311ClearWorkFiles.mnTransactionID,&dsF4311BeginDoc.mnTransactionID);//· mnTransactionID

				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**F4311ClearWorkFiles...\n"));
				jdePrintf(_J("**F4311ClearWorkFiles...\n"));
				idResult = jdeCallObject(_J("F4311ClearWorkFiles"),NULL,lpBhvrCom1,lpVoid1,(void *)&dsF4311ClearWorkFiles,
								(CALLMAP *)NULL,(int)0,(JCHAR *)NULL,(JCHAR *)NULL,(int)0);	
				jdeErrorSetToFirstEx(lpBhvrCom1,lpVoid1);
				while (ErrorRec = jdeErrorGetNextDDItemNameInfoEx(lpBhvrCom1,lpVoid1)){
					iErrorCode = 620;
					jdeFprintf(dlg,_J("***Error(%d): F4311ClearWorkFiles: %ls...\n"),iErrorCode,ErrorRec->lpszShortDesc);
					jdeFflush(dlg);
				} 
				if (((idResult == ER_ERROR) && (iErrorCode == 0)) || (dsF4311ClearWorkFiles.cErrorInClear == '1')){
					iErrorCode = 620;
					jdeFprintf(dlg,_J("***Error(%d): F4311ClearWorkFiles...\n"),iErrorCode);
					jdeFflush(dlg);
				}
				if (iErrorCode != 0) goto lbFIN;

				jdeFflush(dlg);

				//Queremos poder hacer otro BeginDoc...
				if (iDbgFlg > 0) jdeFprintf(dlg,_J("**\nComieza Procesamiento Encabezado...\n"));
				jdePrintf(_J("**Comieza Procesamiento Encabezado...\n"));

				iSalir = 0;
				iPrimeraVez = 1;

				// Codigo Proveedor
				//szCamposContenido[0]
				// Tipo Documento			
				// Referencia del Cliente
				//szCamposContenido[2]

				// Allocate and set data structures
				iErrorCode = 0;
				jdeErrorClearEx(lpBhvrCom1,lpVoid1);
				memset((void *) &dsF4311BeginDoc,(int) _J('\0'),sizeof(DSDF4311Z1A));
				lpValidateUDC = &dsValidateUDC;

				memset (lpValidateUDC,(int)_J('\0'),sizeof(dsValidateUDC));
				memset (szReferenciaClienteBuf,(int)_J('\0'),sizeof(szReferenciaClienteBuf));
			}

		} //if (strncmp(recvbuf, "T11:", 4)

lbCommErr:
		closesocket(s);
    } //while (1)

    closesocket(s);

    free(recvbuf);
	free(sendbuf);
    WSACleanup();
    return 0;

lbFIN:
	jdeFflush(dlg);
	iPrimeraVez = 9;
	if(hUser)JDB_FreeUser(hUser);
lbFIN3:
	if(hEnv)JDB_FreeEnv(hEnv);
lbFIN2:
	if (iDbgFlg > 0) jdeFprintf(dlg,_J("***exit main (%d)...\n"), iErrorCode);
	jdeFflush(dlg); 

	if (iDbgFlg == 1) jdeFclose(dlg); 
lbFIN1:
	return iErrorCode;
}
