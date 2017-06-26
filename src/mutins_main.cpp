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

int main(int argc, char * argv[])
{
	Options opt;

	if (!opt.parse(argc, argv))
		return 1;

	vector<MutantBase*> vMutants;
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

		vTargets.push_back(
		Target(  sTarget, new /*Mutant::*/Line(sTargetFile, opt, true)  ) );
	}

	vector<Match> vMatches;


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
		}
		else if( static_cast<size_t>(opt.mutantIndex()) >= vMutants.size()
						|| opt.mutantIndex() < 0 )
		{
			cout << endl
				<< "Invalid mutant index specified."
				<< endl;
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


		if (opt.mutantIndex() == -1)
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

					if( iCount > 0 && iCount < 20 )
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
							vMatches.push_back(Match( t, i, pMutant ));
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

			MutantBase * pMutant = vMutants[opt.mutantIndex()];

			for( size_t j = 0; j < vTargets.size(); j++)
			{
				Target & t = vTargets[j];

				for (size_t i = 0; i + pMutant->matchSize()
					< t.pLine->size(); i++)
				{
					if (pMutant->testMatch(i, t.pLine))
					{
						vMatches.push_back(Match( t, i, pMutant ) );
					}
				}
			}


		}


		if( opt.countOnly() )
		{
			cout << "Loaded " << vMutants.size() << " mutants." << endl;
			cout << "Found " << iCountOnly << " matches." << endl;
		}
		else if( opt.verbose() )
		{
			cout << "Loaded " << vMutants.size() << " mutants." << endl;
			cout << "Found " << vMatches.size() << " matches." << endl;
		}

		if (iCountOnly == 0 && vMatches.size() == 0)
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
					fOrig.open(sOrigFilename.c_str());

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
			out.open(m.t.sFilename.c_str());


			size_t iPatchStart =
				m.t.pLine->terms()[m.iOffset].offset();

			size_t iTermCount = m.pM->matchSize();

			size_t iPatchEnd =
				m.t.pLine->terms()[m.iOffset + iTermCount - 1].offset()
				+ m.t.pLine->terms()[m.iOffset + iTermCount - 1].size();


			stringstream ss;

			for (size_t i = 0; i < m.pM->replaceSize(); i++)
			{
				const Line::Term * pTerm = &m.pM->replaceTerm(i);

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

					for (size_t j = 0; j < m.pM->matchSize(); j++)
					{
						if (m.pM->matchTerm(j).type() == pTerm->type()
							&& m.pM->matchTerm(j).index()
							== pTerm->index())
						{
							iIndex = j;
							break;
						}
					}

					if (iIndex != string::npos)
					{
						ss << " " << m.t.pLine->terms()[m.iOffset + iIndex].value()
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


	for( size_t i = 0; i < vMutants.size(); i++ )
		delete vMutants[i];

	for( size_t i = 0; i < vTargets.size(); i++ )
		delete vTargets[i].pLine;

	return 0;
}