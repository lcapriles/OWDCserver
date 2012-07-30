// owdcmenu.cpp
//
//Creado:		Luis Capriles,		Fecha:	16/12/2003
//Modificado:	Luis Capriles,		Fecha:	01/10/2008 - Conversion a UniCode
//Modificado:	Williams Ovalles,	Fecha:	08/06/2012 - Adaptar a Requerimiento de BSN/Medical

#include "stdafx.h"
#include <stdio.h>

#include <jde.h> 

int OWDCrc01(HENV hEnv,HUSER hUser); //Captura Recepcion Ordenes de Compras  
int OWDCce01(HENV hEnv,HUSER hUser); //Captura Confirmacion Embarques  

void OWDCmp01 (int * primeraVez, int * opcionEscogida, char * pantallaTitulo, int opcionesCantidad, int inicioEtiquetas, 
				char * opcionesEtiqueta[], char * pantallaStatusLine, int iDbgFlg, FILE * dlg);//Manejo de la pantalla de menu...

void OWDCmp02 (int * primeraVez, char * pantallaTitulo, int camposOffset, int camposCantidad, 
				int statusOffset, int inicioEtiquetas, int inicioCampos, int ultimaLinea, int CReqTAB,
				int camposPosiciones[], char * camposEtiquetas[], char camposContenido[][128], int camposErrores[],
				char * pantallaStatusLine, int iDbgFlg, FILE * dlg); //Manejo de la pantalla de entrada datos...

void OWDCmp90 (int iDbgFlg, FILE * dlg); //Termina Manejo de la pantalla con curses...
        

int _cdecl main()
{

	HENV	hEnv		= NULL;   
	HUSER	hUser		= NULL;    

	FILE *	ini;
	FILE *	mnu; 
	FILE *	dlg;   
 
	JCHAR szLinea[80],szLin1[64],szLin2[64],szUsrEntry[128],
		 szUsrEnv[16],szUsrID[16],szUsrPass[16],szUsrLeng[2],szMenuFile[16],  
		 szTempBuf[32],
		 
		 cStandAloneFlg;

	int	 iErrorCode,iDbgFlg,iOpcionSalir,i,
		 iPrimeraVez, iCamposOffset, iCamposCantidad, iStatusOffset, iInicioEtiquetas, iInicioCampos, iUltimaLinea, iCReqTAB, 
		 iCamposPosiciones[64], iCamposErrores[64], iOpcionEscogida;
	char * szCamposEtiquetas[64], szCamposContenido[64][128], szPantallaTitulo[64], szPantallaStatusLine[64], szDummy[128]; 
	 

#define INIwidth 80 


	//*************************************************************************
	//***Procesamiento Archivo INI                                          ***
	//*************************************************************************


	//Contruye nombre de archivo como nombre_Daammdd_Thhmmss.log
	char timebuf[9],datebuf[9];
	JCHAR Ltimebuf[9],Ldatebuf[9],szFileNameBuf[64];

	_strtime(timebuf);
	_strdate(datebuf);
	jdeToUnicode(Ltimebuf,timebuf,DIM(Ltimebuf),UTF8);
	jdeToUnicode(Ldatebuf,datebuf,DIM(Ldatebuf),UTF8);

	jdeStrcpy(szFileNameBuf,_J("../logs/OWDCmenu_D"));
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

	ini = jdeFopen(_J("OWDCmenu.ini"),_J("r"));
	if (!ini){
		iErrorCode =  101;
		jdeFprintf(dlg,_J("***Error(%d) abriendo INI (OWDCmenu.ini)...\n"),iErrorCode);
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
	}
	jdeFclose(ini);
	jdeFflush(dlg);            

	//*************************************************************************
	//***Fin Procesamiento INI                                              ***
	//*************************************************************************

	iErrorCode = 0; //No hay Errores!!!

	if (cStandAloneFlg == _J('1')) {//Login caracter a OW, no compartido...

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));
		memset(szCamposContenido,'\0',sizeof(szCamposContenido));
		memset(iCamposPosiciones,'\0',sizeof(iCamposPosiciones));
		memset(iCamposErrores,'\0',sizeof(iCamposErrores));

		iPrimeraVez = 0;

		strcpy (szPantallaTitulo, "Menu Captura Datos Inventario");
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

	//Salir de curses...

	OWDCmp90 (iDbgFlg, dlg);

	jdeFflush(dlg);

	//*************************************************************************
	//***Procesamiento Archivo MNU                                          ***
	//*************************************************************************
	do{ //while (iOpcionSalir == 2); Loop para reprocesar el menú 

		if (iDbgFlg > 0){
			jdeFprintf(dlg,_J("***Dibujando Ménu\n"));
			jdeFflush(dlg);
		}

		memset(szCamposEtiquetas,'\0',sizeof(szCamposEtiquetas));

		iPrimeraVez = 0;
		iOpcionEscogida = 0;

		strcpy (szPantallaTitulo, "Menu Captura Datos Inventario");
		strcpy (szPantallaStatusLine, "");

		iCamposCantidad = 0;   

		szCamposEtiquetas[iCamposCantidad++] = "1 - Captura Recepcion Orden de Compra";
		szCamposEtiquetas[iCamposCantidad++] = "2 - Captura Despachos Almacen";
		szCamposEtiquetas[iCamposCantidad++] = "0 - Salir";

		OWDCmp01 (&iPrimeraVez, &iOpcionEscogida, szPantallaTitulo, iCamposCantidad, iInicioEtiquetas, szCamposEtiquetas, 
					szPantallaStatusLine,iDbgFlg,dlg);
		
		iOpcionSalir = 0;  
		do {//while (iOpcionSalir == 0 );Loop para validar Opción Menú... 
			switch (iOpcionEscogida){
				case 1:
					iPrimeraVez = 9;//Salir de curses
					OWDCmp90 (iDbgFlg,dlg);
					iOpcionSalir = 2;
					jdeFflush(dlg);
					iErrorCode = OWDCrc01(hEnv,hUser);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Menu OWDCrc01 (%d)...\n"),iErrorCode);
					jdeFflush(dlg);
					iErrorCode = 0; 
					break;
				case 2:
					iPrimeraVez = 9;//Salir de curses
					OWDCmp90 (iDbgFlg,dlg);
					iOpcionSalir = 2;
					jdeFflush(dlg);
					iErrorCode = OWDCce01(hEnv,hUser);
					if (iDbgFlg > 0) jdeFprintf(dlg,_J("***Menu OWDCce01 (%d)...\n"),iErrorCode);
					jdeFflush(dlg);
					iErrorCode = 0; 
					break;

				case 0:
					iOpcionSalir = 1;
					goto lbFIN;
					break;
				default :
					iOpcionSalir = 0;
			}
		} while (iOpcionSalir == 0);
	}while (iOpcionSalir == 2);  
	//*************************************************************************
	//***Fin Procesamiento MNU                                              ***
	//*************************************************************************

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
