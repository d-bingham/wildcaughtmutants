#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "options.h"
#include "mutins.h"

using namespace std;
using namespace mutins;

Options::Options(bool bMutGen)
	: m_bGood(false)
	, m_bTrimText(false)
	, m_bCStyleComments(false)
	, m_bMutGen(bMutGen)
	, m_bCountOnly(false)
	, m_bVerbose(false)
	, m_bSkipBackup(false)
	, m_bAllowAdjacent(false)
	, m_bForwardMutants(false)
	, m_bPrintOnly(false)
	, m_iRandomSeed(time(NULL))
	, m_iMatchIndex(-1)
	, m_iMutantIndex(-1)
	, m_iLiteralMax(4)
	, m_iTokenMax(10)
	, m_pTarget(NULL)
	, m_bFeatures(false)
	, m_iContext(20)
	, m_iLabel(0)
	, m_iOrdinalMutant(-1)
	, m_bAbstract(false)
	, m_bAllFeatures(false)
{
}

Options::~Options()
{
	if (m_pTarget != NULL)
	{
		delete m_pTarget;
		m_pTarget = NULL;
	}
}


bool Options::parse(int argc, char * argv[])
{
	if (argc < 2)
	{
		displayOptions();
		return false;
	}

	for (int i = 1; i < argc; i++)
	{
		string sArg(argv[i]);

		if (sArg == "--help")
		{
			displayOptions();
			return false;
		}
		if (sArg.length() > 1 && sArg[0] == '-')
		{
			m_bGood = true;

			if (sArg.length() != 2)
			{

				if( sArg == "--verbose")
				{
					m_bVerbose = true;
				}
				else if( sArg == "--allow-adjacent")
				{
					m_bAllowAdjacent = true;
				}
				else if( sArg == "--forward")
				{
					m_bForwardMutants = true;
				}
				else if( sArg == "--features" )
				{
					m_bFeatures = true;
				}
				else if( sArg == "--abstract" )
				{
					m_bAbstract = true;
				}
				else if( sArg == "--print-only" )
				{
					m_bPrintOnly = true;
				}
				else if( sArg == "--max-literals")
				{
					m_bGood = parseCommandInt(m_iLiteralMax, i, argc, argv);
				}
				else if( sArg == "--max-tokens")
				{
					m_bGood = parseCommandInt(m_iTokenMax, i, argc, argv);
				}
				else if( sArg == "--ordinal") 
				{
					m_bGood = parseCommandInt(m_iOrdinalMutant, i, argc, argv);
				}
				else if( sArg == "--c-comments")
				{
					m_bCStyleComments = true;
				}
				else if( sArg == "--trim-text")
				{
					m_bTrimText = true;
				}
				else if( sArg == "--context" )
				{
					m_bGood = parseCommandInt(m_iContext, i, argc, argv);
				}
				else if( sArg == "--label" )
				{
					m_bGood = parseCommandInt(m_iLabel, i, argc, argv);
				}
				else if( sArg == "--all-features" ) 
				{
					m_bAllFeatures = true;
				}
				else
				{
					m_bGood = false;
					return false;
				}
			}
			else switch (sArg[1])
			{
			case 'c':
				m_bCountOnly = true;
				break;

			case 'v':
				m_bVerbose = true;
				break;

			// case 'k':
			// 	m_bGood = parseCommandString(m_sKeywordFile, i, argc, argv);
			// 	break;

			case 'b':
				m_bSkipBackup = true;
				break;

			case 'd':
				m_bGood = parseCommandString(m_sLanguageDefFile, i, argc, argv);
				break;

			case 'x':
				m_bGood = parseCommandString(m_sExtractFile, i, argc, argv);
				break;

			case 't':
				m_bGood = !m_bMutGen && 
					parseCommandVector(m_vTargetFiles, i, argc, argv);
				break;
			
			case 'l':
				m_bGood = parseCommandInt(m_iLiteralMax, i, argc, argv);
				break;

			case 'i':
				if (m_bMutGen)
				{
					m_bGood = 
						parseCommandVector(m_vInputFiles, i, argc, argv);
				}
				else
				{
					m_bGood = parseCommandInt(m_iMatchIndex, i, argc, argv);
				}
				break;

			case 'r':
				m_bGood = !m_bMutGen
					&& parseCommandInt(m_iRandomSeed, i, argc, argv);
				break;

			case 'm':
				m_bGood = !m_bMutGen
					&& parseCommandInt(m_iMutantIndex, i, argc, argv);
				break;

			default:
				m_bGood = false;
				break;
			}

			if (!m_bGood)
			{
				displayOptions();
				return false;
			}
		}
	}

	//loadLanguageFile(operatorFile(), m_sOperators);
	//loadLanguageFile(keywordFile(), m_sKeywords);
	loadLanguageFile(languageDefFile());

	if (!m_bMutGen && targetFile().length() > 0)
	{
		ifstream in;
		in.open(targetFile().c_str());

		string sTarget;

		sTarget.assign((istreambuf_iterator<char>(in)),
			(istreambuf_iterator<char>()));

		m_pTarget = new /*Mutant::*/Line(sTarget, *this);
	}

	if (m_bMutGen)
	{
		if (extractFile().length() == 0)
		{
			displayOptions();
			cout << endl
				<< "You must specify an extract file." << endl;
			return false;
		}

		if (inputFiles().size() == 0)
		{
			displayOptions();
			cout << endl 
				<< "You must specify one or more output files." << endl;
			return false;
		}
	}


	return true;
}

bool Options::parseCommandInt(int & iValue, int & i, int argc, char * argv[])
{
	if (i < argc - 1)
	{
		iValue = atoi(argv[i + 1]);
		i++;
		return true;
	}
	else
	{
		return false;
	}
}


bool Options::parseCommandString(string & sValue, int & i,
	int argc, char * argv[])
{
	if (i < argc - 1)
	{
		sValue = argv[i + 1];
		i++;
		return true;
	}
	else
	{
		return false;
	}

}

bool Options::parseCommandVector(vector<string> & vStrings, int & i,
	int argc, char * argv[])
{
	if (i < argc - 1)
	{
		while (i < argc - 1 && argv[i + 1][0] != '-')
		{
			vStrings.push_back(argv[i + 1]);
			i++;
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool Options::loadLanguageFile(string sFilename)
{
	if( sFilename.length() == 0 )
		return false;

	ifstream in;
	in.open(sFilename.c_str());

	while (in.good())
	{
		string sLine;
		getline(in, sLine);

		istringstream ssLine(sLine);

		string sInitial;
		ssLine >> sInitial;

		while( ssLine.good() ) 
		{
			string sTerm;
			ssLine >> sTerm;

			if( sTerm.length() == 0 ) 
			{
				continue;
			}

			if( sInitial == "K" )
			{
				m_sKeywords.insert(sTerm);
			}
			else if( sInitial == "O" )
			{
				m_sOperators.insert(sTerm);
			}
			else if( sInitial == "C" )
			{
				m_sComments.insert(sTerm);
			}
			else if( sInitial == "c" )
			{
				m_sComments.insert(sTerm);
			}
			else if( sInitial == "Q" ) 
			{
				m_sQuotes.insert(sTerm);
			}

		}

	}

	in.close();

	return true;
}

void Options::displayOptions()
{
	m_bGood = false;

	if (m_bMutGen)
	{
		cout << "Usage: mutgen [options]" << endl;
	}
	else
	{
		cout << "Usage: mutins [options]" << endl;
	}

	cout << endl
		<< "Options:" << endl
		<< "  --help                  display this text" << endl;

	if (m_bMutGen)
	{
		cout
			<< "  -d LANGUAGE_FILE         load language definition from LANGUAGE_FILE" << endl
			<< "  -i INPUT_FILES...       extract mutants from INPUT_FILES" << endl
			<< "  -x EXTRACT_FILE         store extracted mutants in EXTRACT_FILE" << endl
			<< "  -v, --verbose           verbose output" << endl
			<< "  -l #, --max-literals #  set maximum literals to parse" << endl
			<< "  --max-tokens #          set maximum tokens allowed per mutant" << endl
			<< "  --c-comments            respect C/C++-style comments in parsing" << endl
			<< "  --trim-text             trim verbose text response from mutant file" << endl
			<< "  --allow-adjacent        allow adjacent literals" << endl
			<< "  --forward               do not reverse patch generation" << endl
			;
	}
	else
	{
		cout
			<< "  -v, --verbose       verbose output" << endl
			<< "  -c                  only count potential matches, do not insert" << endl
			<< "  --print-only        only display mutation operator" << endl
			<< "  --c-comments        respect C/C++-style comments in parsing" << endl
			<< "  --trim-text         trim verbose text response from mutant file" << endl
			<< "  -i MATCH_INDEX      insert the match at the specific index" << endl
			<< "  -r RANDOM_SEED      use RANDOM_SEED to initialize random number generator" << endl
			<< "                      (this is ignored if the -i option is used)" << endl
			<< "  -m MUTANT_INDEX     use only the mutant found at the (zero-based) " << endl
			<< "                      index MUTANT_INDEX" << endl
			<< "  -x EXTRACT_FILE     load mutants (and language data) from EXTRACT_FILE" << endl
			<< "  -t TARGET_FILES...  attempt to insert mutant into TARGET_FILE" << endl
			<< "  -b                  skip backup (by default, modified files are copied to " << endl
			<< "                      \"file.orig\" before mutant insertion)" << endl
			<< "  --features          generate feature vector from insertion" << endl
			<< "  --context #         how many tokens around the patch to featurize" << endl
			<< "  --label #           label to generate in feature vector" << endl
			<< "  --abstract          generate mutgen-style abstracted tokens in feature" << endl
			<< "                      vector" << endl
			<< "  --ordinal #         insert mutant based on its ordinal position" << endl
			;
	}

	cout << endl;
}


bool Options::isKeyword(string sWord)
{
	return m_sKeywords.find(sWord) != m_sKeywords.end();
}

bool Options::isOperator(string sWord)
{
	return m_sOperators.find(sWord) != m_sOperators.end();
}

void Options::markEvent(string sEvent)
{
	map<string,int>::iterator it = m_mEvents.find(sEvent);

	if( it == m_mEvents.end() )
	{
		m_mEvents[sEvent] = 1;
	}
	else
	{
		m_mEvents[sEvent]++;
	}
}

void Options::displayEvents()
{
	for( map<string,int>::iterator it = m_mEvents.begin();
			it != m_mEvents.end();
			it++ )
	{
		cout << it->first << "," << it->second << endl;
	}	
}
