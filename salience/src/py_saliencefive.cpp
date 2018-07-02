#undef _DEBUG /* Link with python24.lib and not python24_d.lib */
#include <Python.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "py_saliencefive.h"

#ifndef WIN32
#  define _strdup strdup
#endif

using namespace std;

static int CallbackProxy(void* pParams, int nErrorCode, const char* acMessage)
{
	std::cerr << "Callback error level " << nErrorCode << ": " << acMessage << '\n';
	return 0;
}

static int CallbackProxySilent(void* pParams, int nErrorCode, const char* acMessage)
{
	return 0;
}


//If closesession hasn't been called we do it here to ensure that the memory
//is freed up
static void lxa_SalienceObject_dealloc(lxa_SalienceObject* self)
{
    if (self->pSalienceSession != NULL)
	{
		lxaCloseSalienceSession(self->pSalienceSession);
		lxaFreeLicense(&self->pLicense);
	}
	PyObject_Del(self);
}

static PyTypeObject lxa_SEType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "SalienceSession",
    sizeof(lxa_SalienceObject),
    0,
    (destructor)lxa_SalienceObject_dealloc, /*tp_dealloc*/
    0,                   /*tp_print*/
    0,                   /*tp_getattr*/
    0,                   /*tp_setattr*/
    0,                   /*tp_compare*/
    0,                   /*tp_repr*/
    0,                   /*tp_as_number*/
    0,                   /*tp_as_sequence*/
    0,                   /*tp_as_mapping*/
    0,                   /*tp_hash */
};

static void lxaPyDict_SetItemString(PyObject* pDictionary, const char* acKey, PyObject *pValue)
{
	PyDict_SetItemString(pDictionary,acKey,pValue);
	Py_DecRef(pValue);
}

static void lxaPyList_Append(PyObject* pList, PyObject* pValue)
{
	PyList_Append(pList,pValue);
	Py_DecRef(pValue);
}

static bool CheckErrorCode(lxa_SalienceObject* pSession, int nError)
{
	pSession->nLastError = nError;  //So we can return error codes
	if(nError == LXA_OK)
	{
		pSession->nLastWarning = 0;
		return true;
	}
	else if(nError == LXA_OK_WITH_WARNINGS)
	{
		lxaGetLastWarnings(pSession->pSalienceSession, &pSession->nLastWarning);
		return true;
	}
	else
	{
		pSession->nLastWarning = 0;
		char* acError;
		lxaGetSalienceErrorString(pSession->pSalienceSession,&acError);
		PyErr_SetString(PyExc_EnvironmentError,acError);
		lxaFreeString(acError);
		return false;
	}
}

static PyObject* MarshalPhrase(const SaliencePhrase& oPhrase)
{
		PyObject* pPhrase = PyDict_New();		
		lxaPyDict_SetItemString(pPhrase,"phrase",Py_BuildValue("s", oPhrase.acText));
		lxaPyDict_SetItemString(pPhrase,"document",Py_BuildValue("i", oPhrase.nDocument));
		lxaPyDict_SetItemString(pPhrase,"sentence",Py_BuildValue("i", oPhrase.nSentence));
		lxaPyDict_SetItemString(pPhrase,"word",Py_BuildValue("i", oPhrase.nWord));
		lxaPyDict_SetItemString(pPhrase,"length",Py_BuildValue("i", oPhrase.nLength));
		lxaPyDict_SetItemString(pPhrase,"byte_offset",Py_BuildValue("i", oPhrase.nByte));
		lxaPyDict_SetItemString(pPhrase,"byte_length",Py_BuildValue("i", oPhrase.nByteLength));
		return pPhrase;
}

static PyObject* ConvertPhraseList(SaliencePhraseList& oPhraseList)
{
	PyObject* pList = PyList_New(0);
	SaliencePhrase* pPhrase = oPhraseList.pPhrases;
	for(size_t i = 0; i < oPhraseList.nLength; i++, pPhrase++)
	{
		lxaPyList_Append(pList,MarshalPhrase(*pPhrase));
	}
	return pList;
}


static PyObject* ConvertTheme(SalienceTheme* pTheme)
{
	PyObject* pDictionary = PyDict_New();
	lxaPyDict_SetItemString(pDictionary, "theme", Py_BuildValue("s",pTheme->acTheme));
	lxaPyDict_SetItemString(pDictionary, "stemmed_theme", Py_BuildValue("s",pTheme->acStemmedTheme));
	lxaPyDict_SetItemString(pDictionary, "normalized_theme", Py_BuildValue("s",pTheme->acNormalizedTheme));
	lxaPyDict_SetItemString(pDictionary, "score",Py_BuildValue("f",pTheme->fScore));
	lxaPyDict_SetItemString(pDictionary, "sentiment",Py_BuildValue("f",pTheme->fSentiment));
	lxaPyDict_SetItemString(pDictionary, "evidence",Py_BuildValue("i",pTheme->nEvidence));
	lxaPyDict_SetItemString(pDictionary, "theme_type",Py_BuildValue("i",pTheme->nThemeType));
	lxaPyDict_SetItemString(pDictionary, "about", Py_BuildValue("i",pTheme->nAbout));
	lxaPyDict_SetItemString(pDictionary, "summary",Py_BuildValue("s",pTheme->acSummary));
	lxaPyDict_SetItemString(pDictionary, "mentions",ConvertPhraseList(pTheme->oMentions));
	return pDictionary;
}

static PyObject* ConvertThemeList(SalienceThemeList* pThemeList)
{
	PyObject* pResult = PyList_New(0);
	SalienceTheme* pTheme = pThemeList->pThemes;
	for (int i=0; i < pThemeList->nLength; i++, pTheme++)
	{
		PyObject* pDictionary = ConvertTheme(pTheme);

		if (PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pResult,pDictionary);
		}
	}
	return pResult;
}

static PyObject* ConvertTopic(SalienceTopic* pTopic)
{
	PyObject* pDictionary = PyDict_New();
	lxaPyDict_SetItemString(pDictionary,"topic",Py_BuildValue("s", pTopic->acTopic));
	lxaPyDict_SetItemString(pDictionary,"hits",Py_BuildValue("i", pTopic->nHits));
	lxaPyDict_SetItemString(pDictionary,"score",Py_BuildValue("f", pTopic->fScore));
	if(pTopic->acAdditional != (char*)NULL && pTopic->acAdditional != "")
	{
		PyObject* pList = PyList_New(0);
		char* pToken = strtok(pTopic->acAdditional,"|");
		while(pToken != NULL)
		{
			lxaPyList_Append(pList,Py_BuildValue("s",pToken));
			pToken = strtok(NULL,"|");
		}
		lxaPyDict_SetItemString(pDictionary,"documents",pList);
	}
	return pDictionary;
}

static PyObject* ConvertTopicList(SalienceTopicList* pTopicList)
{
	PyObject* pResult = PyList_New(0);
	SalienceTopic* pTopic = pTopicList->pTopics;
	for (int i=0; i < pTopicList->nLength; i++, pTopic++)
	{
		PyObject* pDictionary = ConvertTopic(pTopic);

		if (PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pResult,pDictionary);
		}
	}
	return pResult;
}
	
static PyObject* ConvertMentionList(SalienceMentionList* pMentionList)
{
	PyObject* pResult = PyList_New(0);
	SalienceMention* pMention = pMentionList->pMentions;
	for(int i = 0; i < pMentionList->nLength; i++, pMention++)
	{
		PyObject* pDictionary = PyDict_New();
		lxaPyDict_SetItemString(pDictionary,"type",Py_BuildValue("i", pMention->nType));
		lxaPyDict_SetItemString(pDictionary,"score",Py_BuildValue("f", pMention->fScore));
		
		PyObject* pPhrase = MarshalPhrase(pMention->oPhrase);

		if(PyDict_Check(pPhrase))
		{
			lxaPyDict_SetItemString(pDictionary,"phrase",pPhrase);
		}

		if(PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pResult,pDictionary);
		}
	}
	return pResult;
}

static PyObject* ConvertSentimentPhraseList(const SalienceSentimentPhraseList& oPhrases)
{
	PyObject* pResult = PyList_New(0);
	SalienceSentimentPhrase* pPhrase = oPhrases.pPhrases;
	for(size_t i = 0; i < oPhrases.nLength; i++,pPhrase++)
	{
		PyObject* pDictionary = PyDict_New();

		PyObject* pPhraseDict = MarshalPhrase(pPhrase->oPhrase);
		if(PyDict_Check(pPhraseDict))
		{
			lxaPyDict_SetItemString(pDictionary,"phrase",pPhraseDict);
		}

		PyObject* pSupportDict = MarshalPhrase(pPhrase->oSupportingPhrase);
		if(PyDict_Check(pSupportDict))
		{
			lxaPyDict_SetItemString(pDictionary,"support_phrase", pSupportDict);
		}
		lxaPyDict_SetItemString(pDictionary,"score", Py_BuildValue("f",pPhrase->fScore));
		lxaPyDict_SetItemString(pDictionary,"negated", Py_BuildValue("i",pPhrase->nNegated));
		lxaPyDict_SetItemString(pDictionary,"type", Py_BuildValue("i",pPhrase->nType));
		lxaPyDict_SetItemString(pDictionary,"source", Py_BuildValue("s", pPhrase->acSource));
		if(PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pResult,pDictionary);
		}
	}
	return pResult;
}

static PyObject* ConvertEntity(SalienceEntity* pEntity)
{
	PyObject* pDictionary = PyDict_New();

	lxaPyDict_SetItemString(pDictionary,"type",Py_BuildValue("s", pEntity->acType));
	lxaPyDict_SetItemString(pDictionary,"label",Py_BuildValue("s", pEntity->acLabel));
	lxaPyDict_SetItemString(pDictionary,"normalized_form",Py_BuildValue("s", pEntity->acNormalizedForm));
	lxaPyDict_SetItemString(pDictionary,"score",Py_BuildValue("f", pEntity->fSentimentScore));
	lxaPyDict_SetItemString(pDictionary,"evidence",Py_BuildValue("i", pEntity->nEvidence));
	lxaPyDict_SetItemString(pDictionary,"summary",Py_BuildValue("s", pEntity->acSummary));
	lxaPyDict_SetItemString(pDictionary,"confident",Py_BuildValue("i", pEntity->nConfident));
	lxaPyDict_SetItemString(pDictionary,"about",Py_BuildValue("i", pEntity->nAbout));

	//Ok now deal with the themes
	PyObject* pThemeList = ConvertThemeList(&pEntity->oThemes);
	lxaPyDict_SetItemString(pDictionary,"themes",pThemeList);

	PyObject* pMentionList = ConvertMentionList(&pEntity->oMentions);
	lxaPyDict_SetItemString(pDictionary,"mentions",pMentionList);

	PyObject* pSentimentList = ConvertSentimentPhraseList(pEntity->oSentimentPhrases);
	lxaPyDict_SetItemString(pDictionary,"sentiment_phrases",pSentimentList);

	return pDictionary;
}

static PyObject* ConvertOpinionList(SalienceOpinionList* pOpinionList)
{
	PyObject* pResult = PyList_New(0);
	SalienceOpinion* pOpinion = pOpinionList->pOpinions;
	for(int i = 0; i < pOpinionList->nLength; i++, pOpinion++)
	{
		PyObject* pDictionary = PyDict_New();

		lxaPyDict_SetItemString(pDictionary,"quotation",Py_BuildValue("s", pOpinion->acQuotation));
		lxaPyDict_SetItemString(pDictionary,"sentiment",Py_BuildValue("f", pOpinion->fSentiment));
		lxaPyDict_SetItemString(pDictionary,"isTheme",Py_BuildValue("i",pOpinion->nHasTheme));

		PyObject* pSpeaker = ConvertEntity(&pOpinion->oSpeaker);
		if(PyDict_Check(pSpeaker))
		{
			lxaPyDict_SetItemString(pDictionary,"speaker",pSpeaker);
		}

		if(pOpinion->nHasTheme == 0)
		{
			PyObject* pTopic = ConvertEntity(&pOpinion->oEntityTopic);
			if(PyDict_Check(pTopic))
			{
				lxaPyDict_SetItemString(pDictionary,"topic",pTopic);
			}	
		}
		else
		{
			PyObject* pTopic = ConvertTheme(&pOpinion->oThemeTopic);
			if(PyDict_Check(pTopic))
			{
				lxaPyDict_SetItemString(pDictionary,"topic",pTopic);
			}
		}

		if(PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pResult,pDictionary);
		}
	}
	return pResult;
}


static PyObject* ConvertEntityList(SalienceEntityList* pEntityList)
{
	PyObject* pResult = PyList_New(0);
	SalienceEntity* pEntity = pEntityList->pEntities;
	for (int i=0; i < pEntityList->nLength; i++, pEntity++)
	{
		//Create a dictionary for this item
		PyObject* pDictionary = ConvertEntity(pEntity);

		//Add this dictionary to the list
		if (PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pResult,pDictionary);
		}
	}

	return pResult;
}

static PyObject* ConvertCollectionPhraseList(SaliencePhraseList* pPhraseList)
{
	PyObject* pMentionList = PyList_New(0);
	for(size_t j = 0; j < pPhraseList->nLength; j++)
	{
		PyObject* pMention = PyDict_New();
		lxaPyDict_SetItemString(pMention,"text",Py_BuildValue("s", pPhraseList->pPhrases[j].acText));
		lxaPyDict_SetItemString(pMention,"document",Py_BuildValue("i", pPhraseList->pPhrases[j].nDocument));
		lxaPyDict_SetItemString(pMention,"sentence",Py_BuildValue("i", pPhraseList->pPhrases[j].nSentence));
		lxaPyDict_SetItemString(pMention,"word",Py_BuildValue("i", pPhraseList->pPhrases[j].nWord));
		lxaPyList_Append(pMentionList,pMention);
	}
	return pMentionList;
}

static PyObject* ConvertFacetList(SalienceFacetList* pFacetList)
{
	PyObject* pResult = PyList_New(0);
	SalienceFacet* pFacet = pFacetList->pFacets;
	for(size_t i = 0; i < pFacetList->nLength; i++, pFacet++)
	{
		PyObject* pDictionary = PyDict_New();
		lxaPyDict_SetItemString(pDictionary,"facet",Py_BuildValue("s", pFacet->acFacet));
		lxaPyDict_SetItemString(pDictionary,"total_hits",Py_BuildValue("i", pFacet->nCount));
		lxaPyDict_SetItemString(pDictionary,"positive_hits",Py_BuildValue("i", pFacet->nPositiveCount));
		lxaPyDict_SetItemString(pDictionary,"neutral_hits",Py_BuildValue("i", pFacet->nNeutralCount));
		lxaPyDict_SetItemString(pDictionary,"negative_hits",Py_BuildValue("i", pFacet->nNegativeCount));

		
		lxaPyDict_SetItemString(pDictionary,"mentions",ConvertCollectionPhraseList(&pFacet->oMentions));

		PyObject* pAttributeList = PyList_New(0);
		for(size_t j = 0; j < pFacet->nSubjectLength; j++)
		{
			PyObject* pAttribute = PyDict_New();
			lxaPyDict_SetItemString(pAttribute,"attribute",Py_BuildValue("s",pFacet->pAttributes[j].acAttribute));
			lxaPyDict_SetItemString(pAttribute,"mentions",ConvertCollectionPhraseList(&pFacet->pAttributes[j].oMentions));
			lxaPyList_Append(pAttributeList,pAttribute);
		}
		lxaPyDict_SetItemString(pDictionary,"attributes",pAttributeList);

		lxaPyList_Append(pResult,pDictionary);
	}
	return pResult;
}


static PyObject* ConvertCollectionEntityList(SalienceCollectionEntityList* pEntityList)
{
	PyObject* pResult = PyList_New(0);
	SalienceCollectionEntity* pEntity = pEntityList->pEntities;
	for(size_t i = 0; i < pEntityList->nLength; i++, pEntity++)
	{
		PyObject* pDictionary = PyDict_New();
		
		lxaPyDict_SetItemString(pDictionary,"label",Py_BuildValue("s",pEntity->acLabel));
		lxaPyDict_SetItemString(pDictionary,"normalized",Py_BuildValue("s",pEntity->acNormalizedForm));
		lxaPyDict_SetItemString(pDictionary,"type",Py_BuildValue("s",pEntity->acType));
		lxaPyDict_SetItemString(pDictionary,"hits",Py_BuildValue("i",pEntity->nCount));
		lxaPyDict_SetItemString(pDictionary,"positive_hits",Py_BuildValue("i",pEntity->nPositiveCount));
		lxaPyDict_SetItemString(pDictionary,"neutral_hits",Py_BuildValue("i",pEntity->nNeutralCount));
		lxaPyDict_SetItemString(pDictionary,"negative_hits",Py_BuildValue("i",pEntity->nNegativeCount));
		lxaPyDict_SetItemString(pDictionary,"mentions",ConvertCollectionPhraseList(&pEntity->oMentions));
		lxaPyList_Append(pResult,pDictionary);
	}
	return pResult;
}


static PyObject* lxa_OpenSession(PyObject* self, PyObject* args)
{
	lxa_SalienceObject* pSEObject;
	
	char *acError;
	char *acLicensePath;
    char *acDataPath;
    if (!PyArg_ParseTuple(args,"ss",&acLicensePath,&acDataPath))
		return NULL;

    pSEObject = PyObject_New(lxa_SalienceObject, &lxa_SEType);

	int nRet = lxaLoadLicense(acLicensePath, &pSEObject->pLicense, &acError);
	if (nRet != LXA_OK)
	{
		if(nRet == LXA_OK_WITH_WARNINGS)
		{
			lxaGetLastWarnings(pSEObject->pSalienceSession, &pSEObject->nLastWarning);
		}
		else
		{
			PyErr_SetString(PyExc_EnvironmentError,acError);
			lxaFreeString(acError);
			return NULL;
		}
	}
	SalienceSessionStartup oStartup;
	strcpy(oStartup.acDataDirectory, acDataPath);
	strcpy(oStartup.acUserDirectory,acDataPath);
	strcat(oStartup.acUserDirectory,"/user");
	nRet = lxaOpenSalienceSession(pSEObject->pLicense, &oStartup, &pSEObject->pSalienceSession);
	if (nRet != LXA_OK)
	{
		PyErr_SetString(PyExc_EnvironmentError,oStartup.acError);
		return NULL;
	}

	lxaSetSalienceCallback(pSEObject->pSalienceSession,
	                       CallbackProxy,
	                       static_cast<void*>(pSEObject));
	
		
	pSEObject->nCalculateConfidence = 0;
	pSEObject->nEntityThreshold = 55;
	pSEObject->nEntityTimeout = 5;
	Py_XINCREF(Py_None);

    return (PyObject*)pSEObject;
}

static PyObject* lxa_CloseSession(PyObject *self, PyObject *args)
{
	//OK lets get session
	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	//Free the license object
	LexalyticsLicense pLicense = pPythonSession->pLicense;
	lxaFreeLicense(&pPythonSession->pLicense);

	//And the session.
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	lxaCloseSalienceSession(pSession);
	pPythonSession->pSalienceSession = NULL;

	//return 0
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_GetVersion(PyObject *self, PyObject *args)
{
	char *acVersion;
	int nRet = lxaGetSalienceVersion(&acVersion);

	if (nRet != LXA_OK)
	{
		PyErr_SetString(PyExc_EnvironmentError,"Failed to extract Salience Version");
		return NULL;
	}

	PyObject* pReturn = Py_BuildValue("s", acVersion);
	lxaFreeString(acVersion);
	return pReturn;
}

static PyObject* lxa_DumpEnvironment(PyObject *self, PyObject *args)
{
	//OK lets get session
	lxa_SalienceObject* pPythonSession;

	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
	  return NULL;
	}

	char* acBuffer;
	lxaDumpEnvironment(pPythonSession->pSalienceSession, &acBuffer);
	PyObject* pReturn = Py_BuildValue("s", acBuffer);
	lxaFreeString(acBuffer);
	return pReturn;
}

static PyObject* lxa_GetLastWarnings(PyObject *self, PyObject *args)
{
	//OK lets get session
	lxa_SalienceObject* pPythonSession;

	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
	  return NULL;
	}

	return Py_BuildValue("i", pPythonSession->nLastWarning);
}

static PyObject* lxa_PrepareText(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	char* acText;

	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acText))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaPrepareText(pSession,acText)))
	{
		return NULL;
	}
	return Py_BuildValue("i", pPythonSession->nLastError);

}

static PyObject* lxa_PrepareTextFromFile(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	char* acFile;

	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acFile))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaPrepareTextFromFile(pSession,acFile)))
	{
		return NULL;
	}
	return Py_BuildValue("i", pPythonSession->nLastError);
}

static void convertTokenList(PyObject *pList, SalienceWord* pWordPointer, unsigned int nLength)
{
	for(unsigned int i = 0; i < nLength; i++)
	{
		SalienceWord* pWord = pWordPointer + i;
		PyObject* pWordDict = PyDict_New();
		lxaPyDict_SetItemString(pWordDict,"token",Py_BuildValue("s", pWord->acToken));
		lxaPyDict_SetItemString(pWordDict,"stem",Py_BuildValue("s", pWord->acStem));
		lxaPyDict_SetItemString(pWordDict,"tag",Py_BuildValue("s", pWord->acPOSTag));
		lxaPyDict_SetItemString(pWordDict,"invert",Py_BuildValue("i", pWord->nInvert));
		lxaPyDict_SetItemString(pWordDict,"sentiment",Py_BuildValue("f", pWord->fSentiment));
		lxaPyList_Append(pList,pWordDict);
	}
}

static PyObject* lxa_GetDocumentDetails(PyObject *self, PyObject *args)
{
	SalienceDocumentDetails oDetails;
	
	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetDocumentDetails(pSession,&oDetails)))
	{
		return NULL;
	}

	//Create the dictionary we are going to return
	PyObject* pResult = PyDict_New();
	lxaPyDict_SetItemString(pResult,"word_count",Py_BuildValue("i", oDetails.nWordCount));
	lxaPyDict_SetItemString(pResult,"sentence_count",Py_BuildValue("i", oDetails.nSentenceCount));
	lxaPyDict_SetItemString(pResult,"internal_version",Py_BuildValue("s", oDetails.acInternalVersion));
	lxaPyDict_SetItemString(pResult,"fingerprint",Py_BuildValue("s", oDetails.acFingerprint));
	lxaPyDict_SetItemString(pResult,"objective_count",Py_BuildValue("i", oDetails.nObjectiveCount));
	lxaPyDict_SetItemString(pResult,"subjective_count",Py_BuildValue("i", oDetails.nSubjectiveCount));


	//Add terms
	PyObject* pTermList = PyList_New(0);
	SalienceToken* pToken = oDetails.oTermFrequency.pTokens;
	for(int i = 0; i < oDetails.oTermFrequency.nLength; i++, pToken++)
	{
		PyObject* pTerm = PyDict_New();
		lxaPyDict_SetItemString(pTerm,"token",Py_BuildValue("s",pToken->acToken));
		lxaPyDict_SetItemString(pTerm,"token_count",Py_BuildValue("i", pToken->nCount));
		if (PyDict_Check(pTerm))
		{
			lxaPyList_Append(pTermList,pTerm);
		}
	}
	lxaPyDict_SetItemString(pResult,"document_terms",pTermList);

	PyObject* pBigramList = PyList_New(0);
	pToken = oDetails.oBiGrams.pTokens;
	for(int i = 0; i < oDetails.oBiGrams.nLength; i++, pToken++)
	{
		PyObject* pTerm = PyDict_New();
		lxaPyDict_SetItemString(pTerm,"token",Py_BuildValue("s",pToken->acToken));
		lxaPyDict_SetItemString(pTerm,"token_count",Py_BuildValue("i", pToken->nCount));
		if (PyDict_Check(pTerm))
		{
			lxaPyList_Append(pBigramList,pTerm);
		}
	}
	lxaPyDict_SetItemString(pResult,"document_bigrams",pBigramList);

	PyObject* pTrigramList = PyList_New(0);
	pToken = oDetails.oTriGrams.pTokens;
	for(int i = 0; i < oDetails.oTriGrams.nLength; i++, pToken++)
	{
		PyObject* pTerm = PyDict_New();
		lxaPyDict_SetItemString(pTerm,"token",Py_BuildValue("s",pToken->acToken));
		lxaPyDict_SetItemString(pTerm,"token_count",Py_BuildValue("i", pToken->nCount));
		if (PyDict_Check(pTerm))
		{
			lxaPyList_Append(pTermList,pTerm);
		}
	}
	lxaPyDict_SetItemString(pResult,"document_trigrams",pTrigramList);

	//Add sentences
	PyObject* pSentences = PyList_New(0);
	for(int i = 0; i < oDetails.nSentenceCount; i++)
	{
		SalienceSentence* pSentenceData = oDetails.pSentences + i;
		PyObject* pNextSentence = PyDict_New();
		lxaPyDict_SetItemString(pNextSentence,"subjective",Py_BuildValue("i",pSentenceData->nSubjective));
		//sentence tokens
		PyObject* pSentenceWords = PyList_New(0);
		convertTokenList(pSentenceWords,pSentenceData->pTokens, pSentenceData->nLength);
		lxaPyDict_SetItemString(pNextSentence, "tokens", pSentenceWords);
		PyObject* pChunkList = PyList_New(0);
		for(int j = 0; j < pSentenceData->nChunkCount; j++)
		{
			SalienceChunk* pChunk = pSentenceData->pChunks + j;
			PyObject* pNextChunk = PyDict_New();
			lxaPyDict_SetItemString(pNextChunk,"label",Py_BuildValue("s",pChunk->acLabel));
			lxaPyDict_SetItemString(pNextChunk,"sentence_num",Py_BuildValue("i",pChunk->nSentence));

			PyObject* pChunkTokens = PyList_New(0);
			convertTokenList(pChunkTokens,pChunk->pTokens,pChunk->nLength);
			lxaPyDict_SetItemString(pNextChunk,"tokens",pChunkTokens);
			lxaPyList_Append(pChunkList,pNextChunk);
		}
		lxaPyDict_SetItemString(pNextSentence, "chunks", pChunkList);
		lxaPyList_Append(pSentences, pNextSentence);
	}
	lxaPyDict_SetItemString(pResult,"sentences",pSentences);
	return pResult;
}

static PyObject* lxa_GetSummary(PyObject *self, PyObject *args)
{
	//OK lets get the text
	lxa_SalienceObject* pPythonSession;
	float fResult = 0;
	char* acBuffer;
	int nLength;
	PyObject* pRet;

	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession, &nLength))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;
	if(!CheckErrorCode(pPythonSession,lxaGetSummary(pSession, nLength, &acBuffer)))
	{
		return NULL;
	}

	pRet = Py_BuildValue("s", acBuffer);
	lxaFreeString(acBuffer);
	return pRet;
}

static PyObject* lxa_GetSentiment(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;
	

	SalienceSentimentResult oResults;

	//Get the sentiment
	if(!CheckErrorCode(pPythonSession,lxaGetSentiment(pSession,0,&oResults)))
	{
		return NULL;
	}

	PyObject* pRet = PyDict_New();
	lxaPyDict_SetItemString(pRet,"score",Py_BuildValue("f", oResults.fScore));

	PyObject* pModelList = PyList_New(0);
	SalienceSentimentModel* pModel = oResults.pModel;
	for(size_t i = 0; i < oResults.nModelCount; i++,pModel++)
	{
		PyObject* pSentiment = PyDict_New();
		if(pModel->nBest == 0)
		{
			lxaPyDict_SetItemString(pSentiment,"best_match",Py_BuildValue("s","positive"));
		} else if(pModel->nBest == 1)
		{
			lxaPyDict_SetItemString(pSentiment,"best_match",Py_BuildValue("s","negative"));
		} else if(pModel->nBest == 2)
		{
			lxaPyDict_SetItemString(pSentiment,"best_match",Py_BuildValue("s","mixed"));
		} else if(pModel->nBest == 3)
		{
			lxaPyDict_SetItemString(pSentiment,"best_match",Py_BuildValue("s","neutral"));
		}
		else
		{
			lxaPyDict_SetItemString(pSentiment,"best_match",Py_BuildValue("s","unknown"));
		}
		lxaPyDict_SetItemString(pSentiment,"positive",Py_BuildValue("f",pModel->fPositive));
		lxaPyDict_SetItemString(pSentiment,"negative",Py_BuildValue("f",pModel->fNegative));
		lxaPyDict_SetItemString(pSentiment,"mixed",Py_BuildValue("f",pModel->fMixed));
		lxaPyDict_SetItemString(pSentiment,"neutral",Py_BuildValue("f",pModel->fNeutral));
		lxaPyDict_SetItemString(pSentiment,"name",Py_BuildValue("s",pModel->acModelName));
		if (PyDict_Check(pSentiment))
		{
			lxaPyList_Append(pModelList,pSentiment);
		}			
	}
	lxaPyDict_SetItemString(pRet,"models",pModelList);

	PyObject* pPhraseList = PyList_New(0);
	SalienceSentimentPhrase* pPhrase = oResults.oPhrases.pPhrases;
	for (int i=0; i < oResults.oPhrases.nLength; i++, pPhrase++)
	{
		//Create a dictionary for this item
		PyObject* pDictionary = PyDict_New();

		lxaPyDict_SetItemString(pDictionary,"phrase",Py_BuildValue("s", pPhrase->oPhrase.acText));
		lxaPyDict_SetItemString(pDictionary,"score",Py_BuildValue("f", pPhrase->fScore));
		lxaPyDict_SetItemString(pDictionary,"sentence",Py_BuildValue("i", pPhrase->oPhrase.nSentence));
		lxaPyDict_SetItemString(pDictionary,"word",Py_BuildValue("i",pPhrase->oPhrase.nWord));
		lxaPyDict_SetItemString(pDictionary,"length",Py_BuildValue("i",pPhrase->oPhrase.nLength));
		lxaPyDict_SetItemString(pDictionary,"byte_offset",Py_BuildValue("i",pPhrase->oPhrase.nByte));
		lxaPyDict_SetItemString(pDictionary,"byte_length",Py_BuildValue("i",pPhrase->oPhrase.nByteLength));
		lxaPyDict_SetItemString(pDictionary,"is_negated",Py_BuildValue("i",pPhrase->nNegated));
		PyObject* pSupportingPhrase = MarshalPhrase(pPhrase->oSupportingPhrase);
		lxaPyDict_SetItemString(pDictionary, "supporting_phrase", pSupportingPhrase);
		lxaPyDict_SetItemString(pDictionary, "source",Py_BuildValue("s", pPhrase->acSource));
				
		//Add this dictionary to the list
		if (PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pPhraseList,pDictionary);
		}

	}

	lxaFreeSentimentResult(&oResults);

	lxaPyDict_SetItemString(pRet,"phrases",pPhraseList);
	return pRet;
}

static PyObject* lxa_GetThemes(PyObject *self, PyObject *args)
{
	SalienceThemeList oThemeList;
	
	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}
	
	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetThemes(pSession,&oThemeList)))
	{
		return NULL;
	}

	PyObject* pResult = ConvertThemeList(&oThemeList);

	lxaFreeThemeList(&oThemeList);
	return pResult;
}

static PyObject* lxa_GetNamedEntities(PyObject *self, PyObject *args)
{
	SalienceEntityList oResultSet;

	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetNamedEntities(pSession,&oResultSet)))
	{
		return NULL;
	}

	//Create the list we are going to return
	PyObject* pResult = ConvertEntityList(&oResultSet);

	lxaFreeEntityList(&oResultSet);

	return pResult;
}

static PyObject* lxa_GetUserEntities(PyObject *self, PyObject *args)
{
	SalienceEntityList oResultSet;

	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetUserDefinedEntities(pSession,&oResultSet)))
	{
		return NULL;
	}

	//Create the list we are going to return
	PyObject* pResult = ConvertEntityList(&oResultSet);

	lxaFreeEntityList(&oResultSet);

	return pResult;
}


static PyObject* lxa_GetNamedEntityRelationships(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	SalienceRelationList oRelationSet;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetNamedEntityRelationships(pSession,&oRelationSet)))
	{
		return NULL;
	}
	
	PyObject* pResult = PyList_New(0);
	SalienceRelation* pRelation = oRelationSet.pRelations;
	for(int i = 0; i < oRelationSet.nLength; i++, pRelation++)
	{
		PyObject* pDictionary = PyDict_New();
		lxaPyDict_SetItemString(pDictionary,"type",Py_BuildValue("s",pRelation->acType));
		lxaPyDict_SetItemString(pDictionary,"confidence",Py_BuildValue("f",pRelation->fConfidence));
		lxaPyDict_SetItemString(pDictionary,"entities",ConvertEntityList(&pRelation->oEntities));
		lxaPyDict_SetItemString(pDictionary,"extra",Py_BuildValue("s",pRelation->acExtra));
		if(PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pResult, pDictionary);
		}
	}

	lxaFreeRelationList(&oRelationSet);
	return pResult;
}

static PyObject* lxa_GetNamedEntityOpinions(PyObject *self, PyObject *args)
{
	SalienceOpinionList oResultSet;
	
	lxa_SalienceObject* pPythonSession;
	int nThreshold = 55;

    if (!PyArg_ParseTuple(args, "O|i", &pPythonSession, &nThreshold))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetNamedEntityOpinions(pSession,&oResultSet)))
	{
		return NULL;
	}

	//Create the list we are going to return
	PyObject* pResult = ConvertOpinionList(&oResultSet);

	lxaFreeOpinionList(&oResultSet);

	return pResult;
}

static PyObject* lxa_GetUserEntityRelationships(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	SalienceRelationList oRelationSet;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetNamedEntityRelationships(pSession,&oRelationSet)))
	{
		return NULL;
	}
	
	PyObject* pResult = PyList_New(0);
	SalienceRelation* pRelation = oRelationSet.pRelations;
	for(int i = 0; i < oRelationSet.nLength; i++, pRelation++)
	{
		PyObject* pDictionary = PyDict_New();
		lxaPyDict_SetItemString(pDictionary,"type",Py_BuildValue("s",pRelation->acType));
		lxaPyDict_SetItemString(pDictionary,"confidence",Py_BuildValue("f",pRelation->fConfidence));
		lxaPyDict_SetItemString(pDictionary,"entities",ConvertEntityList(&pRelation->oEntities));
		lxaPyDict_SetItemString(pDictionary,"extra",Py_BuildValue("s",pRelation->acExtra));
		if(PyDict_Check(pDictionary))
		{
			lxaPyList_Append(pResult, pDictionary);
		}
	}

	lxaFreeRelationList(&oRelationSet);
	return pResult;
}

static PyObject* lxa_GetUserEntityOpinions(PyObject *self, PyObject *args)
{
	SalienceOpinionList oResultSet;
	
	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetNamedEntityOpinions(pSession,&oResultSet)))
	{
		return NULL;
	}

	//Create the list we are going to return
	PyObject* pResult = ConvertOpinionList(&oResultSet);

	lxaFreeOpinionList(&oResultSet);

	return pResult;
}

static PyObject* EntityMarkupHelper(SalienceDocument& oDoc)
{
	std::string sMarkup = "";
	int nPrimaryId = -1;
	int nSecondaryId = -1;
	std::string sType = "";
	for(size_t i = 0; i < oDoc.nSentenceCount; i++)
	{
		for(size_t j = 0; j < oDoc.pSentences[i].nLength; j++)
		{
			if(oDoc.pSentences[i].pTokens[j].nId == -1)
			{
				if(nPrimaryId != -1)
				{
					sMarkup += "</";
					sMarkup += sType;
					sMarkup += ">";
					nPrimaryId = -1;
					nSecondaryId = -1;
				}
				if(oDoc.pSentences[i].pTokens[j].nPostFixed == 0)
				{
					sMarkup += " ";
				}
				sMarkup += oDoc.pSentences[i].pTokens[j].acToken;
			}
			else
			{
				if(nPrimaryId == oDoc.pSentences[i].pTokens[j].nId && nSecondaryId == oDoc.pSentences[i].pTokens[j].nSecondaryId)
				{
					if(oDoc.pSentences[i].pTokens[j].nPostFixed == 0)
					{
						sMarkup += " ";
					}
					sMarkup += oDoc.pSentences[i].pTokens[j].acToken;
				}
				else
				{
					if(nPrimaryId != -1)
					{
						sMarkup += "</";
						sMarkup += sType;
						sMarkup += ">";
					}
					if(oDoc.pSentences[i].pTokens[j].nPostFixed == 0)
					{
						sMarkup += " ";
					}
					nPrimaryId = oDoc.pSentences[i].pTokens[j].nId;
					nSecondaryId = oDoc.pSentences[i].pTokens[j].nSecondaryId;
					sType = oDoc.pSentences[i].pTokens[j].acEntityType;
					sMarkup += "<";
					sMarkup += sType;
					sMarkup += ">";
					sMarkup += oDoc.pSentences[i].pTokens[j].acToken;				
				}
			}
		}
	}
	if(nPrimaryId != -1)
	{
		sMarkup += "</";
		sMarkup += sType;
		sMarkup += ">";
		nPrimaryId = -1;
		nSecondaryId = -1;
	}
	
	PyObject* pRet = Py_BuildValue("s",sMarkup.c_str());
	
	return pRet;
}

static PyObject* lxa_GetNamedEntityTaggedText(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;
	SalienceDocument oDoc;
	if(!CheckErrorCode(pPythonSession,lxaGetNamedEntityMarkup(pSession,&oDoc)))
	{
		return NULL;
	}

	return EntityMarkupHelper(oDoc);
}

static PyObject* lxa_GetUserEntityTaggedText(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;
	SalienceDocument oDoc;
	if(!CheckErrorCode(pPythonSession,lxaGetUserEntityMarkup(pSession,&oDoc)))
	{
		return NULL;
	}

	return EntityMarkupHelper(oDoc);
}


static PyObject* lxa_GetPOSTaggedText(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;
    SalienceDocument oDoc;

	if(!CheckErrorCode(pPythonSession,lxaGetPOSMarkup(pSession,&oDoc)))
	{
		return NULL;
	}

	std::string sMarkup = "";
	for (int i = 0; i < oDoc.nSentenceCount; i++)
	{
		for (int j = 0; j < oDoc.pSentences[i].nLength; j++)
		{
			if(oDoc.pSentences[i].pTokens[j].nPostFixed == 0)
			{
				sMarkup += " ";
			}
			std::string sTag(oDoc.pSentences[i].pTokens[j].acPOSTag);
			sMarkup += "<";
			sMarkup += sTag;
			sMarkup += ">";
			sMarkup += oDoc.pSentences[i].pTokens[j].acToken;
			sMarkup += "</";
			sMarkup += sTag;
			sMarkup += ">";
		}
	}

	PyObject* pRet = Py_BuildValue("s",sMarkup.c_str());

	return pRet;
}


static PyObject* lxa_GetSentimentTaggedText(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	float fPositive = 0.2f;
	float fNegative = -0.2f;

    if (!PyArg_ParseTuple(args, "O|ff", &pPythonSession, &fNegative, &fPositive))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;
	SalienceDocument oDoc;
	if(!CheckErrorCode(pPythonSession,lxaGetSentimentMarkup(pSession,&oDoc)))
	{
		return NULL;
	}
    std::string sMarkup = "";
	std::string sLastTag = "";
	for (int i = 0; i < oDoc.nSentenceCount; i++)
	{
		for (int j = 0; j < oDoc.pSentences[i].nLength; j++)
		{
			std::string sTag = "";
			int nType = oDoc.pSentences[i].pTokens[j].nSentimentType;
			if(nType != -1)
			{
				if(nType == 0)
				{
					sTag = "LXA_STOP";
				}
				else if(nType == 2)
				{
					sTag = "LXA_POSSIBLE";
				}
				else if(oDoc.pSentences[i].pTokens[j].nInvert != 0)
				{
					sTag = "LXA_INVERT";
				}
				else
				{
					if(nType == 1)
					{
						sTag = "LXA_HANDSCORE_";
					}
					else
					{
						sTag = "LXA_INTERNAL_";
					}
					if(oDoc.pSentences[i].pTokens[j].fSentiment < fNegative)
					{
						sTag += "NEGATIVE";
					}
					else if(oDoc.pSentences[i].pTokens[j].nSentimentType > fPositive)
					{
						sTag += "POSITIVE";
					}
					else
					{
						sTag += "NEUTRAL";
					}
				}
			}
			if(sTag.compare(sLastTag))
			{
				if(!sLastTag.empty())
				{
					sMarkup += "</";
					sMarkup += sLastTag;
					sMarkup += ">";
				}
				if(oDoc.pSentences[i].pTokens[j].nPostFixed == 0)
				{
					sMarkup += " ";
				}
				if(!sTag.empty())
				{
					sMarkup += "<";
					sMarkup += sTag;
					sMarkup += ">";
				}
				sMarkup += oDoc.pSentences[i].pTokens[j].acToken;
				sLastTag = sTag;
			}
			else
			{
				if(oDoc.pSentences[i].pTokens[j].nPostFixed == 0)
				{
					sMarkup += " ";
				}
				sMarkup += oDoc.pSentences[i].pTokens[j].acToken;
			}
		}
		if (!sLastTag.empty())
		{
			sMarkup += "</";
			sMarkup += sLastTag;
			sMarkup += ">";
		}
	}

	PyObject* pRet = Py_BuildValue("s",sMarkup.c_str());

	return pRet;
}


static PyObject* lxa_GetNamedOpinionTaggedText(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	char* acBuffer;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetNamedOpinionTaggedText(pSession,&acBuffer)))
	{
		return NULL;
	}

	PyObject* pRet = Py_BuildValue("s",acBuffer);

	lxaFreeString(acBuffer);

	return pRet;
}

static PyObject* lxa_GetUserOpinionTaggedText(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	char* acBuffer;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetUserOpinionTaggedText(pSession,&acBuffer)))
	{
		return NULL;
	}

	PyObject* pRet = Py_BuildValue("s",acBuffer);

	lxaFreeString(acBuffer);

	return pRet;
}

/*

static PyObject* lxa_GetGrammaticalTaggedText(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	char* acBuffer;

    if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetGrammaticalTaggedText(pSession,&acBuffer)))
	{
		return NULL;
	}

	PyObject* pRet = Py_BuildValue("s",acBuffer);

	lxaFreeString(acBuffer);

	return pRet;
}
*/
static PyObject* lxa_GetQueryTopics(PyObject *self, PyObject *args)
{
	SalienceTopicList oTopicList;
	
	lxa_SalienceObject* pPythonSession;
	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
		return NULL;
	}
	
	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetQueryDefinedTopics(pSession,&oTopicList)))
	{
		return NULL;
	}
	
	PyObject* pResult = ConvertTopicList(&oTopicList);

	lxaFreeTopicList(&oTopicList);
	return pResult;
}

static PyObject* lxa_GetConceptTopics(PyObject *self, PyObject *args)
{
	SalienceTopicList oTopicList;
	
	lxa_SalienceObject* pPythonSession;
	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
		return NULL;
	}
	
	SalienceSession* pSession = pPythonSession->pSalienceSession;

	if(!CheckErrorCode(pPythonSession,lxaGetConceptDefinedTopics(pSession,&oTopicList)))
	{
		return NULL;
	}
	
	PyObject* pResult = ConvertTopicList(&oTopicList);

	lxaFreeTopicList(&oTopicList);
	return pResult;
}

static PyObject* lxa_ExplainConceptMatches(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	char* acBuffer;
	PyObject* pRet;

	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}

	SalienceSession* pSession = pPythonSession->pSalienceSession;
	if(!CheckErrorCode(pPythonSession,lxaExplainConceptMatches(pSession, &acBuffer)))
	{
		return NULL;
	}

	pRet = Py_BuildValue("s", acBuffer);
	lxaFreeString(acBuffer);
	return pRet;
}

static PyObject* lxa_PrepareCollectionFromList(PyObject *self, PyObject *args)
{	
	SalienceCollection oCollection;
	lxa_SalienceObject* pPythonSession;
	PyObject* pObject;
	char* acCollection;
	if (!PyArg_ParseTuple(args, "OsO", &pPythonSession,&acCollection,&pObject))
	{
		return NULL;
	}
	//user has to pass a list in.
	if(!PyList_Check(pObject))
	{		
		PyErr_SetString(PyExc_EnvironmentError,"You must pass in a List as the second argument to PrepareCollectionFromList()");
		return NULL;
	}
	int nSize = (int)PyList_Size(pObject);
	oCollection.nSize = nSize;
	oCollection.acCollectionName = acCollection;
	oCollection.pDocuments = (SalienceCollectionDocument*)malloc(sizeof(SalienceCollectionDocument) * nSize);
	for(size_t i = 0; i < nSize; i++)
	{
		PyObject* pString = PyList_GetItem(pObject,i);
		if(!PyString_Check(pString))
		{
			PyErr_SetString(PyExc_EnvironmentError,"The PrepareCollectionFromList() function must be given a list of only Strings");
			//clean up partially defined list
			for(size_t j = 0; j < i; j++)
			{
				free(oCollection.pDocuments[j].acText);
			}
			free(oCollection.pDocuments);
			return NULL;
		}
		oCollection.pDocuments[i].acText = PyString_AsString(pString);
		oCollection.pDocuments[i].nIsText = 1;
		oCollection.pDocuments[i].nSplitByLine = 0;
		
		std::stringstream oConverter;
		oConverter << i;
		oCollection.pDocuments[i].acIndentifier = _strdup(oConverter.str().c_str());

	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;

	lxaPrepareCollection(pSession,&oCollection);
	free(oCollection.pDocuments);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_PrepareCollectionFromFile(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acPath;
	char* acCollection;
	if (!PyArg_ParseTuple(args, "Oss", &pPythonSession, &acCollection, &acPath))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	lxaPrepareCollectionFromFile(pSession,acCollection, acPath);
	return Py_BuildValue("i", 0);
}

/*static PyObject* lxa_GetCollectionDetails(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
}*/

static PyObject* lxa_GetCollectionThemes(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	SalienceThemeList oThemes;
	lxaGetCollectionThemes(pSession,&oThemes);

	PyObject* pResult = ConvertThemeList(&oThemes);

	lxaFreeThemeList(&oThemes);
	return pResult;
}

static PyObject* lxa_GetCollectionFacets(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	SalienceFacetList oFacets;
	lxaGetCollectionFacets(pSession,&oFacets);
	PyObject* pResult = ConvertFacetList(&oFacets);
	lxaFreeFacetList(&oFacets);
	return pResult;
}

static PyObject* lxa_GetCollectionQueryDefinedTopics(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	SalienceTopicList oTopics;
	lxaGetCollectionQueryDefinedTopics(pSession,&oTopics);
	PyObject* pResult = ConvertTopicList(&oTopics);
	lxaFreeTopicList(&oTopics);
	return pResult;
}

static PyObject* lxa_GetCollectionConceptDefinedTopics(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	SalienceTopicList oTopics;
	lxaGetCollectionConceptDefinedTopics(pSession,&oTopics);
	PyObject* pResult = ConvertTopicList(&oTopics);
	lxaFreeTopicList(&oTopics);
	return pResult;
}

static PyObject* lxa_GetCollectionEntities(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	SalienceCollectionEntityList oEntities;
	lxaGetCollectionEntities(pSession,&oEntities);
	PyObject* pResult = ConvertCollectionEntityList(&oEntities);
	lxaFreeCollectionEntityList(&oEntities);
	return pResult;
}


static PyObject* lxa_SetOption_TaggingThreshold(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession, &nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 1000;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_MaxExecution(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 1002;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_FailLongSentences(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 1003;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_UserDirectory(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession,&acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 1000;
	oOption.acValue = acValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_ConceptSlop(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	float fValue;
	if (!PyArg_ParseTuple(args, "Of", &pPythonSession,&fValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 1005;
	oOption.fValue = fValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_DocumentSemantics(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 1006;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_MaxConceptTopicHits(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 2000;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_MinConceptTopicScore(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	float fValue;
	if (!PyArg_ParseTuple(args, "Of", &pPythonSession,&fValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 2001;
	oOption.fValue = fValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_ConceptTopicWindowSize(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 2002;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_ConceptTopicWindowJump(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 2003;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_NongrammaticalTopics(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 2004;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_ConceptTopicList(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 2005;
	oOption.acValue = acValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_RequiredEntities(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 3000;
	oOption.acValue = acValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_AnaphoraResolution(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession, &nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 3001;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_EntityModelSensitivity(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	float fValue;
	if (!PyArg_ParseTuple(args, "Of", &pPythonSession, &fValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 3002;
	oOption.fValue = fValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_EntityThreshold(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession, &nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 3003;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_SummaryLength(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession, &nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 3004;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_EntityOverlap(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession, &nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 3005;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_UserEntityList(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 3006;
	oOption.acValue = acValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_OverlapSentimentThemes(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 3007;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_SetSentimentDictionary(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 4000;
	oOption.nValue = 1;
	oOption.acValue = acValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_AddSentimentDictionary(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 4000;
	oOption.nValue = 0;
	oOption.acValue = acValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_AddSentimentModel(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 4001;
	oOption.acValue = acValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_ClearSentimentModel(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;


	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 4001;
	oOption.acValue = "";
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_LowerNeutralScore(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	float fValue;
	if (!PyArg_ParseTuple(args, "Of", &pPythonSession,&fValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 4003;
	oOption.fValue = fValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_UpperNeutralScore(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	float fValue;
	if (!PyArg_ParseTuple(args, "Of", &pPythonSession,&fValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 4002;
	oOption.fValue = fValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_QueryTopicStemming(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession, &nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 5000;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_QueryTopicList(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	char* acValue;
	if (!PyArg_ParseTuple(args, "Os", &pPythonSession, &acValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 5001;
	oOption.acValue = acValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_CollectionResultSize(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "On", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 6000;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_CollectionSemantics(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "On", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 6001;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_SetOption_ReturnAllThemeMentions(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	int nValue;
	if (!PyArg_ParseTuple(args, "Oi", &pPythonSession,&nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 6004;
	oOption.nValue = nValue;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}

static PyObject* lxa_Disable_Callback(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;
	int nValue = 1;

	if (!PyArg_ParseTuple(args, "O|i", &pPythonSession, &nValue))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	if(nValue == 0)
	{
		lxaSetSalienceCallback(pSession,
							   CallbackProxy,
							   static_cast<void*>(pSession));
	}
	else
		lxaSetSalienceCallback(pSession,
							   CallbackProxySilent,
							   static_cast<void*>(pSession));	{

	}

	return Py_BuildValue("i", 0);
}

static PyObject* lxa_Reset_Themes(PyObject *self, PyObject *args)
{
	lxa_SalienceObject* pPythonSession;

	if (!PyArg_ParseTuple(args, "O", &pPythonSession))
	{
        return NULL;
	}
	SalienceSession* pSession = pPythonSession->pSalienceSession;
	
	SalienceOption oOption;
	oOption.nOption = 1008;
	oOption.nValue = 1;
	lxaSetSalienceOption(pSession,&oOption);
	return Py_BuildValue("i", 0);
}
 

static PyMethodDef SalienceMethods[] = {
	{"openSession", (PyCFunction)lxa_OpenSession, METH_VARARGS,"Create a new SalienceSession object."},
	{"closeSession", (PyCFunction)lxa_CloseSession, METH_VARARGS,"Closes the session"},
	{"getVersion", (PyCFunction)lxa_GetVersion, METH_VARARGS,"Returns the version of Salience in use"},
	{"getLastWarnings", (PyCFunction)lxa_GetLastWarnings, METH_VARARGS,"Returns the warning bitmask (if any) from the last command"},
	{"dumpEnvironment", (PyCFunction)lxa_DumpEnvironment, METH_VARARGS,"Returns a string representation of Salience's state, specifically any options that have been set."},

	{"prepareText", (PyCFunction)lxa_PrepareText, METH_VARARGS,"Prepares the text for processing"},
	{"prepareTextFromFile", (PyCFunction)lxa_PrepareTextFromFile, METH_VARARGS,"Prepares the text in the file for processing"},
	{"getDocumentDetails", (PyCFunction)lxa_GetDocumentDetails, METH_VARARGS,"Gets details about the current document"},
	{"getSummary", (PyCFunction)lxa_GetSummary, METH_VARARGS,"Gets a summary of the document"},
	{"getDocumentSentiment", (PyCFunction)lxa_GetSentiment, METH_VARARGS,"Calculates the sentiment for the passed in document"},
	{"getDocumentThemes", (PyCFunction)lxa_GetThemes, METH_VARARGS,"Calculates the themes for the passed in document"},
	{"getNamedEntities", (PyCFunction)lxa_GetNamedEntities, METH_VARARGS,"Calculates the named entities for the passed in document"},
	{"getUserEntities", (PyCFunction)lxa_GetUserEntities, METH_VARARGS,"Calculates the user defined entities for the passed in document"},
	{"getNamedEntityRelationships", (PyCFunction)lxa_GetNamedEntityRelationships, METH_VARARGS,"Gets the relationships between named Entities in the text"},
	{"getNamedEntityOpinions", (PyCFunction)lxa_GetNamedEntityOpinions, METH_VARARGS,"Gets opinions attributable to named entities in the passed in document"},
	{"getUserEntityRelationships", (PyCFunction)lxa_GetUserEntityRelationships, METH_VARARGS,"Gets the relationships between user Entities in the text"},
	{"getUserEntityOpinions", (PyCFunction)lxa_GetUserEntityOpinions, METH_VARARGS,"Gets opinions attributable to user entities in the passed in document"},
	{"getNamedEntityTaggedText", (PyCFunction)lxa_GetNamedEntityTaggedText, METH_VARARGS,"Gets the Named Entity Tagged Text"},
	{"getUserEntityTaggedText", (PyCFunction)lxa_GetUserEntityTaggedText, METH_VARARGS,"Gets the User Entity Tagged Text"},
	{"getNamedOpinionTaggedText", (PyCFunction)lxa_GetNamedOpinionTaggedText, METH_VARARGS,"Gets the Named Opinion Tagged Text"},
	{"getUserOpinionTaggedText", (PyCFunction)lxa_GetUserOpinionTaggedText, METH_VARARGS,"Gets the User Opinion Tagged Text"},
	{"getPOSTaggedText", (PyCFunction)lxa_GetPOSTaggedText, METH_VARARGS,"Gets the POS Tagged Text"},
	{"getSentimentTaggedText", (PyCFunction)lxa_GetSentimentTaggedText, METH_VARARGS,"Gets the Sentiment Tagged Text"},
	{"getQueryDefinedTopics", (PyCFunction)lxa_GetQueryTopics, METH_VARARGS,"returns a list of the Query Defined Topics that matched the passed in document"},
	{"getConceptDefinedTopics", (PyCFunction)lxa_GetConceptTopics, METH_VARARGS,"returns a list of the Concept Defined Topics that match the passed in document"},
	{"explainConceptMatches", (PyCFunction)lxa_ExplainConceptMatches, METH_VARARGS,"returns a string representation of the top words in the document that contributed to each Concept Topic match"},

	{"prepareCollectionFromList", (PyCFunction)lxa_PrepareCollectionFromList, METH_VARARGS,"prepares a list of strings for processing as a collection"},
	{"prepareCollectionFromFile", (PyCFunction)lxa_PrepareCollectionFromFile, METH_VARARGS,"prepares a CSV file for processing as a collection"},
//  {"getCollectionDetails", (PyCFunction)lxa_GetCollectionDetails, METH_VARARGS,"returns basic count information about a document collection"},
	{"getCollectionThemes", (PyCFunction)lxa_GetCollectionThemes, METH_VARARGS,"returns the rolled up themes of a document collection"},
	{"getCollectionFacets", (PyCFunction)lxa_GetCollectionFacets, METH_VARARGS,"returns the rolled up facets of a document collection"},
	{"getCollectionQueryDefinedTopics", (PyCFunction)lxa_GetCollectionQueryDefinedTopics, METH_VARARGS,"returns matching topics in a collection defined by setOption_QueryTopicList"},
	{"getCollectionConceptDefinedTopics", (PyCFunction)lxa_GetCollectionConceptDefinedTopics, METH_VARARGS,"returns matching topics in a collection defined by setOption_ConceptDefinedTopics"},
	{"getCollectionEntities", (PyCFunction)lxa_GetCollectionEntities, METH_VARARGS,"returns the rolled up entities of a document collection"},

	{"resetThemes", (PyCFunction)lxa_Reset_Themes , METH_VARARGS, "Reinitializes the theme detection of Salience, to make use of any new data file changes" },
	{"disableCallback", (PyCFunction)lxa_Disable_Callback, METH_VARARGS, "Disables (or renables) the outputting of callback messages to standard output" },

	{"setOption_TaggingThreshold", (PyCFunction)lxa_SetOption_TaggingThreshold , METH_VARARGS, "Salience rejects documents with high density of symbols. Defaults to 80, lower values are less picky about symbols" },
	{"setOption_MaxExecutionTime", (PyCFunction)lxa_SetOption_MaxExecution , METH_VARARGS, "Maximum time in milliseconds certain functions are allowed to run for" },
	{"setOption_FailOnLongSentences", (PyCFunction)lxa_SetOption_FailLongSentences , METH_VARARGS, "When a document contains a very long sentence, should you process the rest of the document? (0=process)" },
	{"setOption_UserDirectory", (PyCFunction)lxa_SetOption_UserDirectory , METH_VARARGS, "Path to a user directory" },
	{"setOption_ConceptSlop", (PyCFunction)lxa_SetOption_ConceptSlop , METH_VARARGS, "" },
	{"setOption_DocumentSemantics", (PyCFunction)lxa_SetOption_DocumentSemantics , METH_VARARGS, "" },
	
	{"setOption_MaxConceptTopicHits", (PyCFunction)lxa_SetOption_MaxConceptTopicHits , METH_VARARGS, "Most hits a concept topic can contain (for collections)" },
	{"setOption_MinConceptTopicScore", (PyCFunction)lxa_SetOption_MinConceptTopicScore , METH_VARARGS, "Minimum score to accept a concept topic => document link (0-1)" },
	{"setOption_ConceptTopicWindowSize", (PyCFunction)lxa_SetOption_ConceptTopicWindowSize , METH_VARARGS, "Smaller values allow small, on topic text to match concepts for the whole document" },
	{"setOption_ConceptTopicWindowJump", (PyCFunction)lxa_SetOption_ConceptTopicWindowJump , METH_VARARGS, "Smooths out the window feature, at a slight performance hit. Should never be larger than window size" },
	{"setOption_NongrammaticalTopics", (PyCFunction)lxa_SetOption_NongrammaticalTopics , METH_VARARGS, "Set to 0 if your concept topics are defined by sentences, 1 if they are collections of words" },
	{"setOption_ConceptTopicList", (PyCFunction)lxa_SetOption_ConceptTopicList , METH_VARARGS, "List of concept topic definitions (or path to a file containing them)." },
	{"setOption_RequiredEntities", (PyCFunction)lxa_SetOption_RequiredEntities , METH_VARARGS, "List of entities you want reported (leave empty if you want all of them)" },
	{"setOption_AnaphoraResolution", (PyCFunction)lxa_SetOption_AnaphoraResolution , METH_VARARGS, "Whether to link pronouns and entities" },
	{"setOption_EntityModelSensitivity", (PyCFunction)lxa_SetOption_EntityModelSensitivity , METH_VARARGS, "0-100, controls tradeoff between getting good results on problematic documents or maintaining fast throughput (100 accepts the highest degredation)" },
	{"setOption_EntityThreshold", (PyCFunction)lxa_SetOption_EntityThreshold , METH_VARARGS, "0-100, controls how confident Salience must be to return an entity" },
	{"setOption_EntitySummaryLength", (PyCFunction)lxa_SetOption_SummaryLength , METH_VARARGS, "How many sentences to include in entity summaries (0 to turn the feature off)" },
	{"setOption_EntityOverlap", (PyCFunction)lxa_SetOption_EntityOverlap , METH_VARARGS, "Whether User Entities can overlap each other" },
	{"setOption_UserEntityList", (PyCFunction)lxa_SetOption_UserEntityList , METH_VARARGS, "List or path to file of User Entity Definitions" },
	{"setOption_OverlapSentimentThemes", (PyCFunction)lxa_SetOption_OverlapSentimentThemes , METH_VARARGS, "" },

	{"setOption_SetSentimentDictionary", (PyCFunction)lxa_SetOption_SetSentimentDictionary , METH_VARARGS, "All subsequent sentiment analysis will use the specified sentiment dictionary" },
	{"setOption_AddSentimentDictionary", (PyCFunction)lxa_SetOption_AddSentimentDictionary , METH_VARARGS, "Adds additional sentiment dictionaries to be used in sentiment analysis" },
	{"setOption_AddSentimentModel", (PyCFunction)lxa_SetOption_AddSentimentModel , METH_VARARGS, "Uses a different compiled sentiment model file for sentiment analysis" },
	{"setOption_ClearSentimentModel", (PyCFunction)lxa_SetOption_ClearSentimentModel , METH_VARARGS, "Turns off model based sentiment analysis" },
	{"setOption_UpperNeutralScore", (PyCFunction)lxa_SetOption_UpperNeutralScore , METH_VARARGS, "" },
	{"setOption_LowerNeutralScore", (PyCFunction)lxa_SetOption_LowerNeutralScore , METH_VARARGS, "" },

	{"setOption_QueryTopicStemming", (PyCFunction)lxa_SetOption_QueryTopicStemming , METH_VARARGS, "Whether queries should be stemmed or not (can alo be disabled at a per query level)" },
	{"setOption_QueryTopicList", (PyCFunction)lxa_SetOption_QueryTopicList , METH_VARARGS, "List or path to file of Query Topic Definitions" },
	{"setOption_CollectionResultSize", (PyCFunction)lxa_SetOption_CollectionResultSize , METH_VARARGS, "" },
	{"setOption_CollectionSemantics", (PyCFunction)lxa_SetOption_CollectionSemantics , METH_VARARGS, "" },
	{"setOption_ReturnAllThemeMentions", (PyCFunction)lxa_SetOption_ReturnAllThemeMentions , METH_VARARGS, "" },

	{NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC initsaliencefive(void)
{
  PyObject *m;

  lxa_SEType.ob_type = &PyType_Type;

  m = Py_InitModule("saliencefive", SalienceMethods);
  if (m == NULL) return;
  
  PyModule_AddIntConstant(m, "LXA_WARNING_NONE", 0);
  PyModule_AddIntConstant(m, "LXA_WARNING_LARGE_DOCUMENT", 1);
  PyModule_AddIntConstant(m, "LXA_WARNING_NON_UTF8", 2);
  PyModule_AddIntConstant(m, "LXA_WARNING_NO_WHITESPACE", 4);
  PyModule_AddIntConstant(m, "LXA_WARNING_NO_PUNCTUATION", 8);

  PyModule_AddIntConstant(m, "LXA_ERROR",  -1);
  PyModule_AddIntConstant(m, "LXA_OK", 0);
  PyModule_AddIntConstant(m, "LXA_NODOCUMENT", 1);
  PyModule_AddIntConstant(m, "LXA_INVALID_LICENSE", 2);
  PyModule_AddIntConstant(m, "LXA_ERROR_PREPROCESSING", 3);
  PyModule_AddIntConstant(m, "LXA_INVALID_PARAM", 4);
  PyModule_AddIntConstant(m, "LXA_INVALID_PATH", 5);
  PyModule_AddIntConstant(m, "LXA_OK_WITH_WARNINGS", 6);

}

