#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include <random>
#include <sys/stat.h>

#include "mutins.h"
#include "options.h"

using namespace std;
using namespace mutins;

class Target
{
public:
	string sFilename;
	/*Mutant::*/Line * pLine;
};

class Match
{
public:
	Target t;
	size_t iOffset;
	Mutant * pM;
};


int main(int argc, char * argv[])
{
	Options opt;

	if (!opt.parse(argc, argv))
		return 1;

	vector<Mutant*> vMutants;
	vector<Target> vTargets;

	if (opt.extractFile().size() > 0)
	{
		ifstream in;
		in.open(opt.extractFile());

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
		}
	}

	for (string sTarget : opt.targetFiles())
	{
		ifstream in;
		in.open(sTarget);

		if (!in.good())
		{
			cout << "Unable to open target file " << sTarget << "." << endl;
			return 1;
		}

		string sTargetFile;

		sTargetFile.assign((istreambuf_iterator<char>(in)),
			(istreambuf_iterator<char>()));

		vTargets.push_back(
			Target{ sTarget, new /*Mutant::*/Line(sTargetFile, opt, true) });
	}

	vector<Match> vMatches;


	if (vMutants.size() == 0)
	{
		opt.displayOptions();

		cout << endl
			<< "No mutants to insert.  Please specify a valid extraction file."
			<< endl;
	}
	/*
	else if (vTargets.size() == 0)
	{
		opt.displayOptions();

		cout << endl
			<< "No targets to modify.  Please specify at least one target."
			<< endl;
	}
	*/

	vector<Mutant*> vRepOp;
	vector<Mutant*> vDelete;
	vector<Mutant*> vModify;
	int iOther = 0;

	if( vMutants.size() > 0 )
	{
		for( Mutant * pMutant : vMutants )
		{
			bool bOther = true;

			if( pMutant->isReplaceOperator() )
			{
				vRepOp.push_back(pMutant);
				bOther = false;
			}

			if( pMutant->isDeleteStatement() )
			{
				vDelete.push_back(pMutant);
				bOther = false;
			}

			if( pMutant->isModifyBranch() )
			{
				vModify.push_back(pMutant);
				bOther = false;
			}



			if( bOther )
			{
				iOther++;
			}
		}
	}

	if( opt.verbose() )
	{
		cout << "Replace operator:" << endl;
		for(Mutant * pMutant : vRepOp )
		{
			pMutant->display();
		}

		cout << endl << "Delete statement:" << endl;
		for(Mutant * pMutant : vDelete )
		{
			pMutant->display();
		}

		cout << endl << "Modify branch:" << endl;
		for( Mutant * pMutant : vModify )
		{
			pMutant->display();
		}
	}

	cout << endl << "Replace operator mutants: " << vRepOp.size() 
		<< endl << "Modify branch mutants: " << vModify.size()
		<< endl << "Delete statement mutants: " << vDelete.size() 
		<< endl << "Other mutants: " << iOther << endl;



	if( false )
	if (vMutants.size() > 0 && vTargets.size() > 0)
	{
		mt19937 mt(opt.randomSeed());


		if (opt.mutantIndex() == -1)
		{
			for (Mutant * pMutant : vMutants)
			{
				for (Target & t : vTargets)
				{
					for (size_t i = 0; i + pMutant->after().size()
						< t.pLine->size(); i++)
					{
						if (pMutant->testMatch(i, t.pLine))
						{
							vMatches.push_back(Match{ t, i, pMutant });
						}
					}
				}

			}
		}
		else
		{
			if (opt.mutantIndex() >= static_cast<int>(vMutants.size()))
			{
				cout << "Invalid mutant index." << endl;
				return 1;
			}

			Mutant * pMutant = vMutants[opt.mutantIndex()];

			for (Target & t : vTargets)
			{
				for (size_t i = 0; i + pMutant->after().size()
					< t.pLine->size(); i++)
				{
					if (pMutant->testMatch(i, t.pLine))
					{
						vMatches.push_back(Match{ t, i, pMutant });
					}
				}
			}


		}


		if( opt.verbose() || opt.countOnly() )
		{
			cout << "Loaded " << vMutants.size() << " mutants." << endl;
			cout << "Found " << vMatches.size() << " matches." << endl;
		}

		if (vMatches.size() == 0)
		{
			cout << "No matches found." << endl;
			return 1;
		}


		if (!opt.countOnly() && vMatches.size() > 0 )
		{
			size_t iIndex = mt() % vMatches.size();

			if (opt.matchIndex() > -1)
				iIndex = static_cast<size_t>(opt.matchIndex());

			if (iIndex >= vMatches.size())
			{
				cout << "Invalid match index." << endl;
				return 1;
			}

			Match & m = vMatches[iIndex];


			if (!opt.skipBackup())
			{
				string sOrigFilename = m.t.sFilename + ".orig";

				struct stat buffer;
				if (stat(sOrigFilename.c_str(), &buffer) != 0)
				{
					if (opt.verbose())
					{
						cout << "Creating " << sOrigFilename 
							<< "." << endl;
					}

					ofstream fOrig;
					fOrig.open(sOrigFilename);

					fOrig << m.t.pLine->source();

					fOrig.close();
				}
				else if (opt.verbose())
				{
					cout << sOrigFilename 
						<< " already exists, not overwriting." << endl;
				}
			}

			ofstream out;
			out.open(m.t.sFilename);



			size_t iPatchStart =
				m.t.pLine->terms()[m.iOffset].offset();

			size_t iTermCount = m.pM->after().size();

			size_t iPatchEnd =
				m.t.pLine->terms()[m.iOffset + iTermCount - 1].offset()
				+ m.t.pLine->terms()[m.iOffset + iTermCount - 1].size();


			stringstream ss;

			for (size_t i = 0; i < m.pM->before().size(); i++)
			{
				/*Mutant::*/Line::Term * pTerm = &m.pM->before().terms()[i];

				switch (pTerm->type())
				{
				case tsKeyword:
				case tsOperator:
					ss << pTerm->value() << " ";
					break;

				case tsLiteral:
				case tsIdentifier:
				case tsNumber:
				{
					size_t iIndex = string::npos;

					for (size_t j = 0; j < m.pM->after().size(); j++)
					{
						if (m.pM->after().terms()[j].type() == pTerm->type()
							&& m.pM->after().terms()[j].index()
							== pTerm->index())
						{
							iIndex = j;
							break;
						}
					}

					if (iIndex == string::npos)
					{
						ss << "?badindex? ";
					}
					else
					{
						ss << m.t.pLine->terms()[m.iOffset + iIndex].value() 
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
				cout << "   " << m.pM->serialize() << endl << endl;

				cout << "Extracted code:" << endl;
				cout << "   " << m.t.pLine->source().substr(iPatchStart,
					iPatchEnd - iPatchStart) << endl << endl;

				cout << "Un-patched code:" << endl;
				cout << "   " << ss.str() << endl;
			}


			if (iPatchStart > 0)
			{
				out << m.t.pLine->source().substr(0, iPatchStart - 1);
			}

			out << ss.str();

			out << m.t.pLine->source().substr(iPatchEnd + 1);

			out.close();
		}
	}


	for (Mutant * pMutant : vMutants)
		delete pMutant;

	for (Target & t: vTargets)
		delete t.pLine;
	

	return 0;
}