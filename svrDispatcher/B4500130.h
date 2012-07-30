
/*****************************************************************************
 *    Header File:  B4500130.h
 *
 *    Description:  F4076 Calculate Algebraic Expression Header File
 *
 *        History:
 *          Date        Programmer  SAR# - Description
 *          ----------  ----------  -------------------------------------------
 *   Author 02/14/1997  Kim S.      1414660 - Created  
 *          09/10/1997  S. Vahl     Added Variable UOM to DS
 *			   05/17/2005  Sharmini T. Formula based on Test Result Name
 *          06/09/2005  Sumitro P.  7582058 - Added support for functions (MIN, MAX, AVG)
 *          07/06/2005  Sumitro P.  7582058 - Added support for adj detail reference processing
 *          07/15/2005  Sumitro P.  7582058 - Support for force eval mode (price matrix)
 *          08/09/2005  Sumitro P.  7582058 - added UOM Conversion & DataItem validation 
 *                                            for adj detail reference
 *          08/16/2005  Sumitro P.  7582058 - removed hardcoded values to use user defined symbols(F4009)
 *          10/18/2005  Sumitro P.  7789198 - account for right justified format of MCU 
 *          04/02/2007  Rama		8429768 - Added Localization Plug & Play
 * Copyright (c) J.D. Edwards World Source Company, 1996
 *
 * This unpublished material is proprietary to J.D. Edwards World Source 
 * Company.  All rights reserved.  The methods and techniques described 
 * herein are considered trade secrets and/or confidential.  Reproduction
 * or distribution, in whole or in part, is forbidden except by express
 * written permission of J.D. Edwards World Source Company.
 ****************************************************************************/

#ifndef __B4500130_H
#define __B4500130_H

/*****************************************************************************
 * Table Header Inclusions
 ****************************************************************************/
#include <f4075.h>

#include <f4211.h>

#include <f4311.h>

#include <f43121.h>

#include <f4072.h>

#include <f4092.h>

#include <f40941.h>

#include <f40942.h>

#include <f40943.h>

/*****************************************************************************
 * External Business Function Header Inclusions
 ****************************************************************************/
#include <b0000027.h>

#include <b0000033.h>

#include <b0000500.h>

#include <b1100001.h>

#include <b4000260.h>

#include <b4000520.h>

#include <b4500040.h>

#include <b4500241.h>

#include <b9800420.h>   /* Get Data Dictionary Definition */

#include <x0005.h>

#include <b4504660.h>   /*  SAR 7834201  */

#include <b4504670.h>   /*  SAR 7834201  */

#include <b4504680.h>   /*  SAR 7834201  */

#include <b4504690.h>   /*  SAR 7834201  */

#include <b4504700.h>   /*  SAR 7834201  */

#include <n7003400.h>   /*  SAR 8429768  */

/*****************************************************************************
 * Global Definitions
 ****************************************************************************/
#define B4500130_CLOSE            _J(')')
#define B4500130_DIVISION         _J('/')
#define B4500130_MULTIPLICATION   _J('*')
#define B4500130_MINUS            _J('-')
#define B4500130_ADDITION         _J('+')
#define B4500130_OPEN             _J('(')
#define B4500130_MAX_EXP_LEN      501

/* 7582058 */
#define B4500130_FN_NAME_LEN       3         /* fn name limited to 3 exactly characters - don't change this*/
#define B4500130_FN_MIN           _J("MIN")  
#define B4500130_FN_MAX           _J("MAX")
#define B4500130_FN_AVG           _J("AVG")

/* 7582058 */
#define B4500130_ITEM_KEY_ID			    _J('I')  /* Item */
#define B4500130_CUSTOMER_KEY_ID		    _J('A')  /* AddressBook Number - Customer or Supplier */
#define B4500130_CUSTOMER_GROUP_TYPE    _J('C')  /* SAR 7794798  */
#define B4500130_ORDER_GROUP_KEY_ID     _J('O')  /* Order */
#define B4500130_INVALID_KEY_ID         _J('9')  /* Invalid */

#define B4500130_ITEM_VALUE_MAX_LEN     6
#define B4500130_CUST_VALUE_MAX_LEN     3
#define B4500130_ORDER_VALUE_MAX_LEN    12

#define B4500130_DATAITEM_MAX_LEN       10
#define B4500130_VALUE_MAX_LEN          B4500130_ORDER_VALUE_MAX_LEN
#define B4500130_MAX_BUFFER_LEN         B4500130_VALUE_MAX_LEN + B4500130_DATAITEM_MAX_LEN + 4

#define B4500130_BLANK                  _J(' ')

/* error definitions */
#define B4500130_ERR_GEN_SYNTAX	      _J('1')   /* general syntax error - the catch all */
#define B4500130_ERR_FN_NESTED	      _J('2')   /* syntax error - nested function */
#define B4500130_ERR_FN_INVALID        _J('3')   /* syntax error - invalid function name */
#define B4500130_ERR_ARG_MIN_NUM       _J('4')   /* syntax error - fn. without atleast 2 arguments */
#define B4500130_ERR_ARG_ZEROLEN       _J('5')   /* syntax error - zero length argument */
#define B4500130_ERR_ADJ_SYNTAX	      _J('6')   /* general adj ref syntax error - the catch all */
#define B4500130_ERR_ADJ_BC_NOT_7      _J('7')   /* adj detail ref error - current adj not a formula */
#define B4500130_ERR_ADJ_BC_NOT_5      _J('8')   /* adj detail ref error - referred adj not an addon */
#define B4500130_ERR_ADJ_REF_NOT_FOUND _J('9')   /* adj detail ref error - unable to resolve adj detail ref */
#define B4500130_ERR_ADJ_INVALID_DD    _J('D')   /* adj detail ref error - not a valid dataitem */

/*****************************************************************************
 * Structure Definitions
 ****************************************************************************/
typedef struct
{	
   JCHAR          cVariableTableSymbol;      /* CP02 from F4009 */
   JCHAR          cUOMSymbol;                /* CP03 from F4009 */
   JCHAR          cCurrencySymbol;           /* CP04 from F4009 */
   JCHAR          cDBFieldSymbol;            /* CP05 from F4009 */
   JCHAR          cInventoryInterfaceFlag;   /* IVI from F40205 */
   JCHAR          cCurrencyControl;          /* N, Y, or Z */
   JCHAR          cCurrencyConversionMethod; /* EURO - From B0000033 */
   JCHAR          szExpression[B4500130_MAX_EXP_LEN]; /* Expression to evaluate */
   int            nFormulaLength;            /* Length of Formula using strlen() */
   int            nExpIndex;                 /* Index of the expression to evaluate */
   int            nFormulaIndex;             /* Index of the formula string */
   int            nFirstCharIndex;           /* Index of the first char */
   BOOL           bPrecededByMathOp;         /* */
   BOOL           bEvaluate;                 /* True if called to evaluate formula */
   MATH_NUMERIC   mnUOMConversionFactor;     /* Conversion factor from B4000520 */
   MATH_NUMERIC   mnExchangeRate;            /* Exchange rate from B0000033 */
   MATH_NUMERIC   mnTriangulationRateFrom;   /* EURO - From B0000033 */
   MATH_NUMERIC   mnTriangulationRateTo;     /* EURO - From B0000033 */
   LPF4211        lpdsF4211;                 /* Pointer to F4211 record */
   LPF4311        lpdsF4311;                 /* Pointer to F4311 record */
   LPDSD4500241   lpdsCsms;                  /* Pointer to CSMS record */
   LPF43121		   lpdsF43121;				      /* Pointer to Receipt record */
   JCHAR          cTestResultNameSymbol;     /* From F4009 */
   
   /* 7582058 - functions (min/max/avg) */
   JCHAR          cFunctionPrefixSymbol;	 /* From F4009 */
   JCHAR          cFunctionArgSeparator;   /* From F4009 */
   BOOL           bInFunc;					    /* */
   int			   nFunctionCount;			 /* Number of functions in expression */
   int			   nArgumentCount;			 /* Number of arguments in current function */
   int			   nFunctionOpen;           /* Number of opening brackets within function */
   int			   nFunctionClose;          /* Number of closing brackets within function */

   /*7582058 - adj detail reference*/
   JCHAR          cAdjustmentRefSymbol;   /* From F4009 */
   MATH_NUMERIC   mnFactorValue;
   JCHAR          szFactorUOM[4];
   JCHAR          szUOM[4];
} DS4500130A, *LPDS4500130A;

/* 7582058 - to capture function substring index info within szExpression*/
typedef struct
{
   JCHAR          szFunctionName[4];     /* MIN, MAX, AVG ... */
   int            nStartIndex;			  /* index of $ in 130A.szExpression  */        
   int            nEndIndex;             /* index of closing )of function in 130A.szExpression */ 
   int            nArgumentCount;        /* No or arguments in function */
   LPLINKLIST     lpArgumentInfoList;    /* Linked List of argument info */
} DS4500130B, *LPDS4500130B;

/* 7582058 - to capture argument substring index info within szExpression*/
typedef struct
{
   int            nArgIndex;			 /* index of | in 130A.szExpression  */        
   BOOL			   bIsExpression;     /* is this argument an expression */
} DS4500130C, *LPDS4500130C;

/* 7582058 */
typedef enum 
{
	MIN   = 0,
	MAX,                   
	AVG,
	INVALID
} FunctionTypeA; // Modificacion Luis Capriles 11/12/01

 /* 7582058 - DS for partial fetch of F4072 records */
 typedef struct 
 {
   /* primary key - we need 10 out of the 12 fields- 
      the remaining 2 are needed only for approvals
      mode
   */
   MATH_NUMERIC   aditm;               /* 0 to 48 */
   JCHAR          adast[9];            /* 49 to 66 */
   MATH_NUMERIC   adan8;               /* 67 to 115 */
   MATH_NUMERIC   adigid;              /* 116 to 164 */
   MATH_NUMERIC   adcgid;              /* 165 to 213 */
   MATH_NUMERIC   adogid;              /* 214 to 262 */
   JCHAR          adcrcd[4];           /* 263 to 270 */
   JCHAR          aduom[3];            /* 271 to 276 */
   MATH_NUMERIC   admnq;               /* 277 to 325 */
   JDEDATE        adexdj;              /* 326 to 331 */

   /* fetched fields */
   JCHAR          adbscd;              /* Basis Code       - 442 to 443 */
   MATH_NUMERIC   adfvtr;              /* Factor Value     - 472 to 520 */
   JCHAR          adfvum[3];           /* Factor Value UOM - 769 to 774 */

 } DSB4500130F4072, *LPDSB4500130F4072;


/*****************************************
 * TYPEDEF for Data Structure
 *    Template Name: F4076 Calculate Algebraic Expression
 *    Template ID:   D4500130
 *    Generated:     Thu Jul 14 17:07:06 2005
 *
 * DO NOT EDIT THE FOLLOWING TYPEDEF
 *    To make modifications, use the OneWorld Data Structure
 *    Tool to Generate a revised version, and paste from
 *    the clipboard.
 *
 **************************************/

#ifndef DATASTRUCTURE_D4500130
#define DATASTRUCTURE_D4500130

typedef struct tagDSD4500130
{
  JCHAR             cCallingMode;                        
  JCHAR             szCalculationFormula[161];           
  MATH_NUMERIC      mnNumericReturnValue;                
  JDEDATE           jdPricingDate;                       
  JCHAR             szCurrencyCode[4];                   
  JCHAR             szUom[3];                            
  ID                idF4211Pointer;                      
  JCHAR             szStandardUOMConv[3];                
  JCHAR             cErrorCode;                          
  JCHAR             cSuppressErrorMsg;                   
  JCHAR             szErrorMsgId[11];                    
  JCHAR             szUnitOfMeasureVariable[3];          
  JCHAR             szTestResultName[31];                
  JCHAR             szTestValue[16];                     
  ID                idF4072Pointer;                      
  JCHAR             cForceEval;                          
} DSD4500130, *LPDSD4500130;
 
#define IDERRcCallingMode_1                       1L
#define IDERRszCalculationFormula_2               2L
#define IDERRmnNumericReturnValue_3               3L
#define IDERRjdPricingDate_4                      4L
#define IDERRszCurrencyCode_5                     5L
#define IDERRszUom_6                              6L
#define IDERRidF4211Pointer_7                     7L
#define IDERRszStandardUOMConv_8                  8L
#define IDERRcErrorCode_9                         9L
#define IDERRcSuppressErrorMsg_10                 10L
#define IDERRszErrorMsgId_11                      11L
#define IDERRszUnitOfMeasureVariable_12           12L
#define IDERRszTestResultName_13                  13L
#define IDERRszTestValue_14                       14L
#define IDERRidF4072Pointer_15                    15L
#define IDERRcForceEval_17                        17L

#endif


/*****************************************************************************
 * Source Preprocessor Definitions
 ****************************************************************************/
#if defined (JDEBFRTN)
	#undef JDEBFRTN
#endif

#if defined (WIN32)
	#if defined (WIN32)
		#define JDEBFRTN(r) __declspec(dllexport) r
	#else
		#define JDEBFRTN(r) __declspec(dllimport) r
	#endif
#else
	#define JDEBFRTN(r) r
#endif

/*****************************************************************************
 * Business Function Prototypes
 ****************************************************************************/
JDEBFRTN (ID) JDEBFWINAPI F4076CalculateAlgebraicExpresn   (LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS);


/*****************************************************************************
 * Internal Function Prototypes
 ****************************************************************************/
ID I4500130_SetUp(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                  HUSER hUser, LPDS4500130A lpds4500130A);

ID I4500130_ProcessVariable(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                            LPDS4500130A lpds4500130A, HUSER hUser);

ID I4500130_ProcessUOM(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                       LPDS4500130A lpds4500130A);

ID I4500130_ProcessCurrency(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                            LPDS4500130A lpds4500130A);

ID I4500130_ProcessDataBaseField(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                                 LPDS4500130A lpds4500130A);

ID I4500130_GetF4075Record(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                           HUSER hUser, LPF4075 *lpdsF4075, JCHAR *szVariable);

ID I4500130_GetUOMConvFactor(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                             LPDS4500130A lpds4500130A, JCHAR *szUOM,
                             int *nSymbolIndex);

ID I4500130_GetExchangeRate(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                            LPDS4500130A lpds4500130A, JCHAR *szCurrencyCode,
                            int *nSymbolIndex);

ID I4500130_EvaluateStringFormula(LPBHVRCOM lpBhvrCom, LPVOID lpVoid,
                                  LPDSD4500130 lpDS, LPDS4500130A lpds4500130A);

ID I4500130_CopyF4211Field(LPDS4500130A lpds4500130A, JCHAR *lpszFieldName);

ID I4500130_ProcessMathOperands(LPDSD4500130 lpDS, LPDS4500130A lpds4500130A);

ID I4500130_ProcessSymbols(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS, 
                           LPDS4500130A lpds4500130A, HUSER hUser,
						   LPDS4500130B lpds4500130B, LPDS4500130C lpds4500130C); /* 7582058 */

ID I4500130A_ProcessConstant(LPDSD4500130 lpDS, LPDS4500130A lpds4500130A);

void I4500130_CallCurrencyConv(LPBHVRCOM lpBhvrCom, LPVOID lpVoid,
                               LPDS4500130A lpds4500130A,
                               LPMATH_NUMERIC lpmnDomesticAmt,
                               LPMATH_NUMERIC lpmnForeignAmt);

ID I4500130_CopyCsmsField(LPDS4500130A lpds4500130A, JCHAR *lpszFieldName);

ID I4500130_CopyF4311Field(LPDS4500130A lpds4500130A, JCHAR *lpszFieldName);

ID I4500130_CopyReceiptField(LPDS4500130A lpds4500130A, JCHAR *lpszFieldName);

ID I4500130_ProcessTestResult(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                              LPDS4500130A lpds4500130A);

/* 7582058 - start function processing */
ID I4500130_ProcessFunction(LPDSD4500130 lpDS, LPDS4500130A lpds4500130A, LPDS4500130B lpDS4500130B);

ID I4500130_EvaluateFunctions(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS, 
							  LPDS4500130A lpds4500130A, LPLINKLIST lpFunctionInfoList);

ID I4500130_GetFunctionValue(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS, 
							 LPDS4500130A lpds4500130A, LPLINKLIST lpArgumentInfoList, 
							 int * nArgStartIndex, int nFunctionType, BOOL bComma,
							 double * dFnValue);

ID I4500130_AddToFunctionInfoList(LPLINKLIST    lpFunctionInfoList, 
								  LPDS4500130B  lpds4500130B, 
								  LPLINKLIST    lpArgumentInfoList);
ID I4500130_AddToArgumentInfoList(LPLINKLIST    lpArgumentInfoList, 
								  LPDS4500130C  lpds4500130C);
ID I4500130_DestroyInfoList(LPLINKLIST	lpFunctionInfoList);

ID    I4500130_FastValidateString (JCHAR *szNumber, BOOL bComma);
void  I4500130_GetStringValue (JCHAR *szNumber, double *dValue, BOOL bComma);
void  I4500130_SetStringValue (JCHAR *szNumber, double *dValue, BOOL bComma);

ID I4500130_EvaluateArgExpression(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS,
                                  JCHAR* szArgExpression, double *dValue);

void  I4500130_SetError(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD4500130 lpDS);
/* 7582058 - end function processing */

/* 7582058 - start adj detail reference processing */
ID I4500130_ProcessAdjustmentRef(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, HUSER hUser, 
                                 LPDSD4500130 lpDS, LPDS4500130A lpds4500130A);

ID I4500130_ParseAdjustmentRef(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, HUSER hUser, 
                               LPDSD4500130 lpDS, LPDS4500130A lpds4500130A,
                               JCHAR* cKeyType, JCHAR* szDataItem, JCHAR* szRefValue);

ID I4500130_ResolveAdjustmentRef(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, HUSER hUser,
                                  LPDSD4500130 lpDS, LPDS4500130A lpds4500130A,
                                  JCHAR cKeyType, JCHAR* szDataItem, JCHAR* szRefValue);

void  I4500130_ProcessRefDot(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, HUSER hUser, 
                             LPDSD4500130 lpDS, LPDS4500130A lpds4500130A, 
                             JCHAR* cKeyType, JCHAR* szDataItem, JCHAR* szRefBuffer,
                             JCHAR* cFormulaChar, int* nDot1, int* nDot2, int* nDotCount, 
                             int* nBuffIndex, BOOL* bError);

BOOL I4500130_IsLegalRefChar(JCHAR cFormulaChar, LPDS4500130A lpds4500130A);

ID I4500130_ResolveF4072Reference(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, HUSER hUser,
                                  LPDSD4500130 lpDS, LPDS4500130A lpds4500130A,
                                  JCHAR cKeyType, JCHAR* szDataItem, JCHAR* szRefValue);

ID I4500130_ResolveItemGroupReference(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, HUSER hUser,
                                       LPDSD4500130 lpDS, LPDS4500130A lpds4500130A,
								               JCHAR cGroupType, JCHAR* szDataItem, JCHAR* szRefValue, 
                                       LPF4072 lpF4072);
ID I4500130_ResolveCustomerGroupReference(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, HUSER hUser,
                                       LPDSD4500130 lpDS, LPDS4500130A lpds4500130A,
								               JCHAR cGroupType, JCHAR* szDataItem, JCHAR* szRefValue, 
                                       LPF4072 lpF4072);
ID I4500130_ResolveOrderGroupReference(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, HUSER hUser,
                                       LPDSD4500130 lpDS, LPDS4500130A lpds4500130A,
								               JCHAR cGroupType, JCHAR* szDataItem, JCHAR* szRefValue, 
                                       LPF4072 lpF4072);

ID I4500130_FetchF40941RecordByID(LPBHVRCOM lpBhvrCom,LPDSD4500130 lpDS, HUSER hUser, LPMATH_NUMERIC lpmnItemGroupKeyID, LPF40941 lpF40941);
ID I4500130_FetchF40942RecordByID(LPBHVRCOM lpBhvrCom,LPDSD4500130 lpDS, HUSER hUser, LPMATH_NUMERIC lpmnCustomerGroupKeyID, LPF40942 lpF40942);
ID I4500130_FetchF40943RecordByID(LPBHVRCOM lpBhvrCom,LPDSD4500130 lpDS, HUSER hUser, LPMATH_NUMERIC lpmnOrderGroupKeyID, LPF40943 lpF40943);

ID I4500130_FetchItemGroupKeyID(LPBHVRCOM lpBhvrCom,LPDSD4500130 lpDS, HUSER hUser, LPF40941 lpF40941, LPMATH_NUMERIC lpmnItemGroupKeyID);
ID I4500130_FetchCustomerGroupKeyID(LPBHVRCOM lpBhvrCom,LPDSD4500130 lpDS, HUSER hUser, LPF40942 lpF40942, LPMATH_NUMERIC lpmnCustomerGroupKeyID);
ID I4500130_FetchOrderGroupKeyID(LPBHVRCOM lpBhvrCom,LPDSD4500130 lpDS, HUSER hUser, LPF40943 lpF40943, LPMATH_NUMERIC lpmnOrderGroupKeyID);

ID I4500130_SubstituteF40941Value(LPF40941 lpF40941, JCHAR* szRefValue, int nIndex, BOOL bIsRightJustified);
ID I4500130_SubstituteF40942Value(LPF40942 lpF40942, JCHAR* szRefValue, int nIndex, BOOL bIsRightJustified);
ID I4500130_SubstituteF40943Value(LPF40943 lpF40943, JCHAR* szRefValue, int nIndex, BOOL bIsRightJustified);

ID I4500130_CopyValueRightPad(JCHAR* szValue, JCHAR* szRefValue, int nValueLen);

ID I4500130_FetchF4092Record(LPBHVRCOM lpBhvrCom,LPDSD4500130 lpDS, HUSER hUser, JCHAR cGroupType, JCHAR* szGroupCode, 
                              JCHAR szCategoryCd[][11], int * nCatCodeCnt);
void  I4500130_LoadF4072Key(LPKEY1_F4072 lpdsF4072Key1, LPF4072 lpF4072, LPMATH_NUMERIC lpmnKeyID, JCHAR cKeyType);
ID I4500130_FetchF4072PriceAdjustmentDetailRecord(HUSER hUser, LPBHVRCOM lpBhvrCom, LPVOID lpVoid,
                                                  LPDSD4500130 lpDS, 
                                                  LPF4072 lpF4072, LPKEY1_F4072 lpdsF4072Key1,
                                                  LPDS4500130A lpds4500130A);
ID I4500130_ConvertFactorValue(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, 
                               LPDSD4500130 lpDS, 
                               LPF4072 lpF4072, LPDS4500130A lpds4500130A);
void  I4500130_LoadCategoryCd(JCHAR cGroupType, JCHAR szCategoryCd[][11], int * nCatCodeCnt, LPF4092 lpdsF4092);

                                                 
/* 7582058 - end adj detail reference processing */

/* 7789198 - start */
ID I4500130_CopyValueLeftPad(JCHAR* szValue, JCHAR* szRefValue, int nValueLen);
ID I4500130_CopyValue(JCHAR* szValue, JCHAR* szRefValue, int nValueLen, BOOL bIsRightJustified);
/* 7789198 - end */

#endif    /* __B4500130_H */

