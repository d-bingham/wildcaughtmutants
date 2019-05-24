#ifndef _options_included_
#define _options_included_

#include <map>
#include <string>
#include <vector>
#include <set>

namespace mutins
{

class Mutant;
class Line;

	
	class Options
	{
	public:
		Options(bool bMutGen = false);
		virtual ~Options();
		bool parse(int argc, char * argv[]);
		void displayOptions();
		bool good() { return m_bGood; }
		bool trimText() { return m_bTrimText; }
		//std::string keywordFile() { return m_sKeywordFile; }
		//std::string operatorFile() { return m_sOperatorFile; }
		std::string languageDefFile() { return m_sLanguageDefFile; }
		std::string targetFile() { return m_sTargetFile; }
		std::string extractFile() { return m_sExtractFile; }
		bool isKeyword(std::string sWord);
		bool isOperator(std::string sWord);
		/*Mutant::*/Line * target() { return m_pTarget; }
		std::vector<std::string> & inputFiles() { return m_vInputFiles; }
		std::vector<std::string> & targetFiles() { return m_vTargetFiles; }
		std::set<std::string> & keywords() { return m_sKeywords; }
		std::set<std::string> & operators() { return m_sOperators; }
		int randomSeed() { return m_iRandomSeed; }
		int matchIndex() { return m_iMatchIndex; }
		bool countOnly() { return m_bCountOnly; }
		bool verbose() { return m_bVerbose; }
		bool skipBackup() { return m_bSkipBackup; }
		bool allowAdjacent() { return m_bAllowAdjacent; }
		bool forwardMutants() { return m_bForwardMutants; }
		bool printOnly() { return m_bPrintOnly; }
		int mutantIndex() { return m_iMutantIndex; }
		int maxLiterals() { return m_iLiteralMax; }
		int maxTokens() { return m_iTokenMax; }
		bool CStyleComments() { return m_bCStyleComments; }
		void markEvent(std::string sEvent);
		void displayEvents();
		bool features() { return m_bFeatures; }
		int context() { return m_iContext; }
		int label() { return m_iLabel; }
		bool abstract() { return m_bAbstract; }

	private:
		bool m_bAbstract;
		bool m_bFeatures;
		int m_iContext;
		int m_iLabel;
		bool m_bGood;
		bool m_bTrimText;
		bool m_bCStyleComments;
		bool m_bMutGen;
		bool m_bCountOnly;
		bool m_bVerbose;
		bool m_bSkipBackup;
		bool m_bAllowAdjacent;
		bool m_bForwardMutants;
		bool m_bPrintOnly;
		int m_iRandomSeed;
		int m_iMatchIndex;
		int m_iMutantIndex;
		int m_iLiteralMax;
		int m_iTokenMax;
		//std::string m_sKeywordFile;
		//std::string m_sOperatorFile;
		std::string m_sLanguageDefFile;
		std::string m_sTargetFile;
		std::string m_sExtractFile;
		std::set<std::string> m_sKeywords;
		std::set<std::string> m_sOperators;
		std::set<std::string> m_sQuotes;
		std::set<std::string> m_sComments;
		std::vector<std::string> m_vInputFiles;
		std::vector<std::string> m_vTargetFiles;
		std::map<std::string, int> m_mEvents;

		bool loadLanguageFile(std::string sFilename);
		bool parseCommandString(std::string & sValue, int & i,
			int argc, char * argv[]);
		bool parseCommandInt(int & iValue, int & i, int argc, char * argv[]);
		bool parseCommandVector(std::vector<std::string> & vStrings, 
			int & i, int argc, char * argv[]);

		Line * m_pTarget;

	};


}

#endif


