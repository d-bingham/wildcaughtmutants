#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include <random>
#include <cstdlib>
#include <sys/stat.h>

#include "mutins.h"
#include "options.h"

using namespace std;
using namespace mutins;


class Target
{
public:
	Target(string s, Line * l) : sFilename(s), pLine(l) {}
	string sFilename;
	Line * pLine;
};


class Match
{
public:
	Match(Target & tt, size_t i, MutantBase * m) : t(tt), iOffset(i), pM(m) {}
	Target t;
	size_t iOffset;
	MutantBase * pM;
};


// actually perform the mutation insertion
void insertMutant(Options & opt, Match * pMatch)
{
	ofstream out;
	out.open(pMatch->t.sFilename.c_str());


	size_t iPatchStart =
		pMatch->t.pLine->terms()[pMatch->iOffset].offset();

	size_t iTermCount = pMatch->pM->matchSize();

	size_t iPatchEnd =
		pMatch->t.pLine->terms()[pMatch->iOffset + iTermCount - 1].offset()
		+ pMatch->t.pLine->terms()[pMatch->iOffset + iTermCount - 1].size();


	stringstream ss;

	for (size_t i = 0; i < pMatch->pM->replaceSize(); i++)
	{
		const Line::Term * pTerm = &pMatch->pM->replaceTerm(i);

		switch (pTerm->type())
		{
		case tsKeyword:
		case tsOperator:
			ss << " " << pTerm->value() << " ";
			break;

		case tsExactIdentifier:
			ss << " " << pTerm->value() << " ";
			break;

		case tsLiteral:
		case tsIdentifier:
		case tsNumber:
		{
			size_t iIndex = string::npos;

			for (size_t j = 0; j < pMatch->pM->matchSize(); j++)
			{
				if (pMatch->pM->matchTerm(j).type() == pTerm->type()
					&& pMatch->pM->matchTerm(j).index()
					== pTerm->index())
				{
					iIndex = j;
					break;
				}
			}

			if (iIndex != string::npos)
			{
				ss << " " << pMatch->t.pLine->terms()[pMatch->iOffset + iIndex].value()
					<< " ";
			}
		}
			break;

		default:
			break;
		}
	}


	if (opt.verbose())
	{
		cout << "Mutant (and source patch):" << endl;
		cout << "   " << pMatch->pM->serialize() << endl << endl;

		cout << "Extracted code:" << endl;
		cout << "   " << pMatch->t.pLine->source().substr(iPatchStart,
			iPatchEnd - iPatchStart) << endl << endl;

		cout << "Un-patched code:" << endl;
		cout << "   " << ss.str() << endl;
	}


	if (iPatchStart > 0)
	{
		out << pMatch->t.pLine->originalSource().substr(0, iPatchStart);
	}

	out << ss.str();

	out << pMatch->t.pLine->originalSource().substr(iPatchEnd);

	out.close();

}


string stripComments(string sSource)
{
	bool bInMultilineComment = false;
	bool bInSingleLineComment = false;
	
	// // Check for a comment left over from
	// // a previous line
	// if( bInMultilineComment )
	// {
	// 	size_t iLoc = sTargetFile.find("*/");

	// 	if( iLoc == string::npos )
	// 	{
	// 		// If we're in a multiline and
	// 		// we don't see an end, ignore this
	// 		// line
	// 		continue;
	// 	}
	// 	else
	// 	{
	// 		// Continue the line after the
	// 		// multiline ends
	// 		sTargetFile = sTargetFile.substr(iLoc + 2);
	// 		bInMultilineComment = false;
	// 	}
	// }


	for( size_t i = 1; i < sSource.length(); i++ )
	{
		if( bInSingleLineComment )
		{
			if( sSource[i] == '\r' || sSource[i] == '\n')
			{
					
				bInSingleLineComment = false;
			}
			else
			{
				sSource[i] = ' ';
			}
		}
		else if( bInMultilineComment )
		{
			if( sSource[i] == '*' 
				&& i+1 < sSource.length() 
				&& sSource[i+1] == '/' )
			{
				sSource[i] = ' ';
				sSource[i+1] = ' ';
				bInMultilineComment = false;
			}
			else 
			{
				sSource[i] = ' ';
			}
		}
		else 
		{
			if( sSource[i] == '*' && sSource[i-1] == '/' )
			{
				sSource[i] = ' ';
				sSource[i-1] = ' ';
				bInMultilineComment = true;
			}
			else if( sSource[i] == '/' && sSource[i-1] == '/')
			{
				sSource[i] = ' ';
				sSource[i-1] = ' ';
				bInSingleLineComment = true;
			}
		}
	}

	return sSource;
}



void featureVector(int iContext, bool bAbstract, int iMaxTokens, Match * pMatch)
{
	for( int i = 0; i < iContext; i++ )
	{
		int iIndex = (static_cast<int>(pMatch->iOffset) - iContext) + i;

		if( iIndex < 0 ) 
		{
			cout << "~~empty ";
		}
		else 
		{
			cout << pMatch->t.pLine->terms()[static_cast<size_t>(iIndex)]
				.displayString(!bAbstract)
				<< " ";
		}
	}

	cout << "~~patch ";

	for( int i = 0; i < iMaxTokens; i++ )
	{
		if( static_cast<size_t>(i) < pMatch->pM->replaceSize() )
		{
			cout << pMatch->pM->replaceTerm(i).displayString(!bAbstract)
				<< " ";
		}
		else
		{
			cout << "~~empty ";
		}
	}


	cout << "~~patch ";


	for( int i = 0; i < iContext; i++ )
	{
		size_t iIndex = pMatch->iOffset + static_cast<size_t>(i);

		if( iIndex >= pMatch->t.pLine->terms().size() )
		{
			cout << "~~empty ";
		}
		else 
		{
			cout << pMatch->t.pLine->terms()[static_cast<size_t>(iIndex)]
				.displayString(!bAbstract)
				<< " ";
		}
	}

	cout << endl;

}

void generateFeatures(Options & opt, Match * pMatch)
{
	// cout << "*****" << endl;
	// cout << pMatch->t.pLine->displayString(true);
	// cout << endl << "*****" << endl;
	// cout << pMatch->t.pLine->displayString(false);
	// cout << "*****" << endl;

	if( opt.allFeatures() )
	{
		cout << "Concrete: ";
		featureVector(opt.context(), false, opt.maxTokens(), pMatch);		
		cout << "Abstract: ";
		featureVector(opt.context(), true, opt.maxTokens(), pMatch);		
	}
	else 
	{
		cout << opt.label() << " ";

		featureVector(opt.context(), opt.abstract(), opt.maxTokens(), pMatch);
	}


	return;

	ofstream out;
	out.open(pMatch->t.sFilename.c_str());


	size_t iPatchStart =
		pMatch->t.pLine->terms()[pMatch->iOffset].offset();

	size_t iTermCount = pMatch->pM->matchSize();

	size_t iPatchEnd =
		pMatch->t.pLine->terms()[pMatch->iOffset + iTermCount - 1].offset()
		+ pMatch->t.pLine->terms()[pMatch->iOffset + iTermCount - 1].size();


	stringstream ss;

	for (size_t i = 0; i < pMatch->pM->replaceSize(); i++)
	{
		const Line::Term * pTerm = &pMatch->pM->replaceTerm(i);

		switch (pTerm->type())
		{
		case tsKeyword:
		case tsOperator:
			ss << " " << pTerm->value() << " ";
			break;

		case tsExactIdentifier:
			ss << " " << pTerm->value() << " ";
			break;

		case tsLiteral:
		case tsIdentifier:
		case tsNumber:
		{
			size_t iIndex = string::npos;

			for (size_t j = 0; j < pMatch->pM->matchSize(); j++)
			{
				if (pMatch->pM->matchTerm(j).type() == pTerm->type()
					&& pMatch->pM->matchTerm(j).index()
					== pTerm->index())
				{
					iIndex = j;
					break;
				}
			}

			if (iIndex != string::npos)
			{
				ss << " " << pMatch->t.pLine->terms()[pMatch->iOffset + iIndex].value()
					<< " ";
			}
		}
			break;

		default:
			break;
		}
	}


	if (opt.verbose())
	{
		cout << "Mutant (and source patch):" << endl;
		cout << "   " << pMatch->pM->serialize() << endl << endl;

		cout << "Extracted code:" << endl;
		cout << "   " << pMatch->t.pLine->source().substr(iPatchStart,
			iPatchEnd - iPatchStart) << endl << endl;

		cout << "Un-patched code:" << endl;
		cout << "   " << ss.str() << endl;
	}


	if (iPatchStart > 0)
	{
		out << pMatch->t.pLine->originalSource().substr(0, iPatchStart - 1);
	}

	out << ss.str();

	out << pMatch->t.pLine->originalSource().substr(iPatchEnd);

	out.close();


}

int main(int argc, char * argv[])
{
	Options opt;

	if (!opt.parse(argc, argv))
		return 1;

	vector<MutantBase*> vMutants;
	vector<int> vMutantCounts;

	vector<Target> vTargets;

	int iCountOnly = 0;

	if (opt.extractFile().size() > 0)
	{
		ifstream in;
		in.open(opt.extractFile().c_str());

		if (!in.good())
		{
			cout << "Unable to open mutant extract file " << opt.extractFile()
				<< "." << endl;
			return 1;
		}

		while (in.good())
		{
			string sLine;
			getline(in, sLine);

			size_t iStartLoc = sLine.find("/*");

			while( iStartLoc != string::npos )
			{
				size_t iEndLoc = sLine.find("*/", iStartLoc + 2);

				if( iEndLoc == string::npos )
				{
					sLine = sLine.substr(0, iStartLoc) +
						sLine.substr(iEndLoc + 2);
				}
			}

			if (sLine.size() < 3)
				continue;

			if (sLine[1] != ' ')
				continue;

			if (sLine[0] == 'O')
			{
				istringstream ss(sLine.substr(2));

				while (ss.good())
				{
					string s;
					ss >> s;

					if (s.size() > 0)
						opt.operators().insert(s);
				}
			}
			else if (sLine[0] == 'K')
			{
				istringstream ss(sLine.substr(2));

				while (ss.good())
				{
					string s;
					ss >> s;

					if (s.size() > 0)
						opt.keywords().insert(s);
				}
			}
			else if (sLine[0] == 'M')
			{
				vMutants.push_back(new Mutant(sLine.substr(2), opt));
			}
			else if( sLine[0] == 'I')
			{
				vMutants.push_back(new IdentifierShift(sLine.substr(2), opt));
			}
		}
	}

	for( size_t i = 0; i < opt.targetFiles().size(); i++ )
	{
		string sTarget = opt.targetFiles()[i];

		ifstream in;
		in.open(sTarget.c_str());

		if (!in.good())
		{
			cout << "Unable to open target file " << sTarget << "." << endl;
			return 1;
		}

		string sTargetFile;

		sTargetFile.assign((istreambuf_iterator<char>(in)),
			(istreambuf_iterator<char>()));

		string sOriginalFile = sTargetFile;


		if( opt.CStyleComments() )
		{
			sTargetFile = stripComments(sTargetFile);
		}		

		vTargets.push_back(
			Target( sTarget, new /*Mutant::*/Line(sTargetFile, opt, true, sOriginalFile) ) 
		);
	}

	if (vMutants.size() == 0)
	{
		opt.displayOptions();

		cout << endl
			<< "No mutants to insert.  Please specify a valid extraction file."
			<< endl;
	}
	else if( opt.printOnly() )
	{
		if( opt.mutantIndex() == -1 )
		{
			cout << endl
				<< "No mutant specified to print."
				<< endl;

			return 1;
		}
		else if( static_cast<size_t>(opt.mutantIndex()) >= vMutants.size()
						|| opt.mutantIndex() < 0 )
		{
			cout << endl
				<< "Invalid mutant index specified."
				<< endl;

			return 1;
		}
		else 
		{
			cout << vMutants[opt.mutantIndex()]->serialize()
				<< endl;
			
			return 0;
		}
	}
	else if (vTargets.size() == 0 )
	{
		opt.displayOptions();

		cout << endl
			<< "No targets to modify.  Please specify at least one target."
			<< endl;
	}

	if (vMutants.size() > 0 && vTargets.size() > 0)
	{
		mt19937 mt(opt.randomSeed());

		size_t iOrdinalMutantIndex = 0;
		size_t iOrdinalMutantMatch = 0;

		if (opt.mutantIndex() == -1 )
		{
			if( opt.countOnly() )
			{
				cout << "Counting..." << endl;

				//int iJob = -1;
				//int iOffset = -1;

				for( size_t i = 0; i < vMutants.size(); i++ )
				{
					MutantBase * pMutant = vMutants[i];

					int iCount = 0;

					for( size_t j = 0; j < vTargets.size(); j++ )
					{
						Target & t = vTargets[j];

						for (size_t i = 0; i + pMutant->matchSize()
							< t.pLine->size(); i++)
						{
							if (pMutant->testMatch(i, t.pLine))
							{
								iCount++;
								iCountOnly++;
							}
						}
					}

					//if( iCount > 0 && iCount < 20 )
					if( iCount > 0 )
					{
						cout << i << " " << iCount << endl;
					}

				}
			}
			else for( size_t j = 0; j < vMutants.size(); j++ )
			{
				MutantBase * pMutant = vMutants[j];

				for( size_t k = 0; k < vTargets.size(); k++ )
				{
					Target & t = vTargets[k];

					for (size_t i = 0; i + pMutant->matchSize()
						< t.pLine->size(); i++)
					{
						if (pMutant->testMatch(i, t.pLine))
						{
							//vMatches.push_back(Match( t, i, pMutant ));
							pMutant->found();

							if( opt.ordinalMutation() == iCountOnly ) 
							{
								cout << "Found ordinal match at " << iCountOnly << endl;
								iOrdinalMutantIndex = j;
								iOrdinalMutantMatch = k;
							}

							iCountOnly++;
						}
					}
				}

			}

			cout << "Saw " << iCountOnly << " mutants." << endl;
		}
		else
		{

			if (opt.mutantIndex() >= static_cast<int>(vMutants.size()))
			{
				cout << "Invalid mutant index." << endl;
				return 1;
			}

			MutantBase * pMutant = vMutants[opt.mutantIndex()];

			for( size_t j = 0; j < vTargets.size(); j++)
			{
				Target & t = vTargets[j];

				for (size_t i = 0; i + pMutant->matchSize()
					< t.pLine->size(); i++)
				{
					if (pMutant->testMatch(i, t.pLine))
					{
						//vMatches.push_back(Match( t, i, pMutant ) );
						pMutant->found();
						iCountOnly++;
					}
				}
			}
		}


		if( opt.countOnly() || opt.verbose() )
		{
			cout << "Loaded " << vMutants.size() << " mutants." << endl;
			cout << "Found " << iCountOnly << " matches." << endl;
		}

		if (iCountOnly == 0 )
		{
			cout << "No matches found." << endl;
			return 1;
		}


		if (!opt.countOnly() && iCountOnly > 0 )
		{
			size_t iIndex = mt() % iCountOnly;

			if (opt.matchIndex() > -1)
				iIndex = static_cast<size_t>(opt.matchIndex());

			if (iIndex >= static_cast<size_t>(iCountOnly) )
			{
				cout << "Invalid match index." << endl;
				return 1;
			}

			// Find the nth match out of the list...
			size_t iMutantIndex = 0;
			size_t iInsertionIndex = 0;

			if( opt.ordinalMutation() > -1 ) 
			{
				iMutantIndex = iOrdinalMutantIndex;
				iInsertionIndex = iOrdinalMutantMatch;
			}
			else if( opt.matchIndex() > -1 && opt.mutantIndex() > -1 ) 
			{
				iMutantIndex = opt.mutantIndex();
				iInsertionIndex = opt.matchIndex();
			}
			else 
			{
				for( size_t i = 0; i < vMutants.size(); i++ ) 
				{
					if( vMutants[i]->insertionPointCount() > iIndex )
					{
						iMutantIndex = i;
						iInsertionIndex = iIndex;
						break;
					}
					else
					{
						iIndex -= vMutants[i]->insertionPointCount();
					}
				}
			}

			if( iMutantIndex > vMutants.size() 
				|| iInsertionIndex >= vMutants[iMutantIndex]->insertionPointCount() )
			{
				cout << "Invalid mutant selection." << endl;
				return 1;
			}

			Match * pMatch = nullptr;

			MutantBase * pMutant = vMutants[iMutantIndex];

			if( opt.verbose() )
			{
				cout << "Applying mutant " << iMutantIndex
					<< " at insertion point " << iInsertionIndex << "."
					<< endl << endl;
			}

			for( size_t j = 0; j < vTargets.size(); j++)
			{
				Target & t = vTargets[j];

				for (size_t i = 0; i + pMutant->matchSize()
					< t.pLine->size(); i++)
				{
					if (pMutant->testMatch(i, t.pLine))
					{
						if( iInsertionIndex == 0 )
						{
							pMatch = new Match(t, i, pMutant);
						}
						else 
						{
							iInsertionIndex--;
						}
					}
				}
			}

			if (!opt.skipBackup())
			{
				string sOrigFilename = pMatch->t.sFilename + ".orig";

				struct stat buffer;
				if (stat(sOrigFilename.c_str(), &buffer) != 0)
				{
					if (opt.verbose())
					{
						cout << "Creating " << sOrigFilename
							<< "." << endl;
					}

					ofstream fOrig;
					fOrig.open(sOrigFilename.c_str());

					fOrig << pMatch->t.pLine->source();

					fOrig.close();
				}
				else if (opt.verbose())
				{
					cout << sOrigFilename
						<< " already exists, not overwriting." << endl;
				}
			}

			if( opt.features() )
			{
				generateFeatures(opt, pMatch);
			}
			else
			{
				if( opt.allFeatures() )
				{
					generateFeatures(opt, pMatch);
				}

				insertMutant(opt, pMatch);
			}
			


			delete pMatch;
		}
	}


	for( size_t i = 0; i < vMutants.size(); i++ )
		delete vMutants[i];

	for( size_t i = 0; i < vTargets.size(); i++ )
		delete vTargets[i].pLine;

	return 0;
}
