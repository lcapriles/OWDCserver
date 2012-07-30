// Module Name: svrDispatcher.c
//
// Description:
//    Prototipo Servidor Despachador conexiones...
//
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "stdafx.h"
#include <jde.h>

#pragma comment(lib, "Ws2_32.lib")

   
#include "b9800100.h"	// GetAuditInfo
#include "B4000370.h"	// F40095GetDefaultBranch
#include "B0000130.h"	// RetrieveCompanyFromBusUnit

#define DEFAULT_PORT_LOCAL      49153
#define DEFAULT_PORT_REMOTE     49152
#define DEFAULT_CLIENT_PORT     55500
#define INITIAL_SERVER_PORT     55501
#define DEFAULT_BUFFER_LENGTH   4096

//

  void OWDCmp02 (int * primeraVez, char * pantallaTitulo, int camposOffset, int camposCantidad, 
				int statusOffset, int inicioEtiquetas, int inicioCampos, int ultimaLinea, int CReqTAB,
				int camposPosiciones[], char * camposEtiquetas[], char camposContenido[][128], int camposErrores[],
				char * pantallaStatusLine, int iDbgFlg, FILE * dlg); //Manejo de la pantalla de entrada datos...
  void OWDCmp90 (int iDbgFlg, FILE * dlg); //Termina Manejo de la pantalla con curses...
        

int _cdecl  main(){

	//Declaraciones JDE...


	HENV	hEnv		= NULL;   
	HUSER	hUser		= NULL;    

	FILE *	ini;
	FILE *	mnu; 
	FILE *	dlg;   
 
	JCHAR szLinea[80],szLin1[64],szLin2[64],szUsrEntry[128],
		 szUsrEnv[16],szUsrID[16],szUsrPass[16],szUsrLeng[2],szMenuFile[16], 
		 szDocTransDateBuf[16],szDocGLDateBuf[16],szDocBranchPlantBuf[16],

		 szLineItemBuf[26], szNumeroCelCliente[64],

		 LszString01[128],LszString02[64],szTempBuf[128],* szDummy0,

		 cStandAloneFlg, cSeparadorEtiqueta[2];

	int	 iErrorCode,iDbgFlg,iOpcionSalir,i,
		 iPrimeraVez, iCamposOffset, iCamposCantidad, iStatusOffset, iInicioEtiquetas, iInicioCampos, iUltimaLinea, iCReqTAB, 
		 iCamposPosiciones[64], iCamposErrores[64], iOpcionEscogida;

	char * szCamposEtiquetas[64], szCamposContenido[64][128], szPantallaTitulo[64], szPantallaStatusLine[64], szDummy[128]; 
	 

#define INIwidth 80 

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

	DSD4000232			dsF40095GetDefaultBranch;
	DSD0000130			dsRetrieveCompanyFromBusUnit;
	DSD9800100			dsGetAuditInfo;


	//Declaraciones UDP...
	int   ilPort    = DEFAULT_PORT_LOCAL;   // Port to receive on
	int   irPort    = DEFAULT_PORT_REMOTE;  // Port to send on
	int   isvrPort	= INITIAL_SERVER_PORT;	// JDE Server Port...
	int   icltPort	= DEFAULT_CLIENT_PORT;	// Client Listening Port...	 
	DWORD dwLength	= DEFAULT_BUFFER_LENGTH; // Length of receiving buffer
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

	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCsvrDispatcher_D"));
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

	ini = jdeFopen(_J("OWDCsvrDispatcher.ini"),_J("r"));
	if (!ini){
		iErrorCode =  101;
		jdeFprintf(dlg,_J("***Error(%d) abriendo INI (OWDCsvrDispatcher.ini)...\n"),iErrorCode);
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

	}
	jdeFclose(ini);
	jdeFflush(dlg);            
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
		
		//Salir de curses...
		OWDCmp90 (iDbgFlg, dlg);
		jdeFflush(dlg);
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
	//...
	jdeCreateBusinessFunctionParms(hUser, &lpBhvrCom1,(LPVOID*) &lpVoid1); //TODO: Debe quedar un solo hvrCom...
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

	//***
	//***Comienza Procesamiento del Encabezado                              
	iPrimeraVez = 1; //TODO: El manejo de esto debe estar malo...


	// Allocate and set data structures
	iErrorCode = 0;
	jdeErrorClearEx(lpBhvrCom1,lpVoid1);
	jdeErrorClearEx(lpBhvrCom,lpVoid);




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
			
			// Descomponemos el mensaje en cliente...
			jdeToUnicode(szNumeroCelCliente,recvbuf,sizeof(szNumeroCelCliente)-1,UTF8);
			if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Cliente CelNro. (%ls)...\n"),szNumeroCelCliente);
			jdePrintf(_J("***Cliente CelNro. (%ls)...\n"),szNumeroCelCliente);
			
			jdeFflush(dlg);	

			// Ahora contestamos al cliente...
			//
			// Respondemos al remoto con los puertos para el servidor...

			sprintf(sendbuf,"%.5i%.5i",isvrPort, icltPort);
			dwLength1 = strlen(sendbuf) + 1;

			ret = sendto(s, sendbuf, dwLength1, 0, (SOCKADDR *)&remote, sizeof(remote));
			printf("Enviando puerto/IP: %d/%s  %s\n", irPort, inet_ntoa(remote.sin_addr), sendbuf);
			if (iDbgFlg > 0) fprintf(dlg,"***Enviando puerto/IP: %d/%s  %s\n", irPort, inet_ntoa(remote.sin_addr), sendbuf);
			if (ret == SOCKET_ERROR){
				printf("sendto() failed; %d\n", WSAGetLastError());
				iErrorCode =  117;
				if (iDbgFlg > 0) fprintf(dlg,"***Error(%d): sendto() failed; %d...\n", iErrorCode, WSAGetLastError());
				jdeFflush(dlg);
				break;
			}
			else if (ret == 0) break;


			isvrPort++; //TODO: Truco para buscar siguiente servidor...

			jdeFflush(dlg);
		}

lbCommErr:
		closesocket(s);
    } //while (1)

    closesocket(s);

    free(recvbuf);
	free(sendbuf);
    WSACleanup();
    return 0;

lbFIN:
	iPrimeraVez = 9;//Salir de curses
	OWDCmp90 (iDbgFlg,dlg);
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
