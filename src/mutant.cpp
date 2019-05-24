#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include <algorithm>

#include "mutins.h"
#include "options.h"

using namespace std;
using namespace mutins;

map<string, size_t> Mutant::s_mIdioms;
map<string, size_t> Mutant::s_mIdentifierChange;



Mutant::Mutant(string sSerial, Options & opt)
	: m_bGood(true)
	, m_lBefore(opt)
	, m_lAfter(opt)
	, m_Options(opt)
{
	istringstream ss(sSerial);

	Line * pLine = &m_lBefore;

	while (ss.good())
	{
		string s;
		ss >> s;

		if (s.length() == 0)
			continue;

		if (s == "@")
		{
			getline(ss, m_sSource);
			break;
		}

		if (s == ":")
		{
			pLine = &m_lAfter;
			continue;
		}

		if (s.length() < 2)
			continue;

		int iIndex = atoi(s.substr(1).c_str());
		string sValue = s.substr(1);

		char c = s[0];

		switch (c)
		{
		case ' ':
			break;

		case '.':
			pLine->addTerm(tsOperator, sValue, 0, 0);
			break;

		case '_':
			pLine->addTerm(tsKeyword, sValue, 0, 0);
			break;

		case '=':
			pLine->addTerm(tsLiteral, sValue, 0, 0);
			break;

		case '$':
			pLine->addTerm(tsIdentifier, sValue, 0, 0);
			break;

		case '#':
			pLine->addTerm(tsNumber, sValue, 0, 0);
			break;

		default:
			break;
		}

		if (c == '=' || c == '#' || c == '$')
		{
			pLine->terms().back().setIndex(iIndex);
		}

	}
}

Mutant::Mutant(string sBefore, string sAfter, Options & opt)
	: m_bGood(true)
	, m_lBefore(sBefore, opt)
	, m_lAfter(sAfter, opt)
	, m_Options(opt)
{
	opt.markEvent("0,Potential mutant");

	m_sSource = sBefore + " : " + sAfter;

	if( m_lBefore.terms().size() == m_lAfter.terms().size() 
		&& m_lBefore.terms().size() > 0 )
	{
		bool bChanged = false;
		bool bBad = false;
		//size_t iIndex = 0;

		for( size_t i = 0; i < m_lBefore.terms().size(); i++ )
		{
			if( m_lBefore.terms()[i].type() != m_lAfter.terms()[i].type() )
			{
				bBad = true;
				break;
			}
			else if( m_lBefore.terms()[i].value() != m_lAfter.terms()[i].value() )
			{
				if( m_lBefore.terms()[i].type() == tsIdentifier )
				{
					if( bChanged )
					{
						bBad = true;
						break;
					}

					bChanged = true;
					//iIndex = i;
					//cout << m_lBefore.terms()[i].value() << " -> " 
					//	<< m_lAfter.terms()[i].value() << endl;

					stringstream ss;
					ss << m_lBefore.terms()[i].value() << " -> "
						<< m_lAfter.terms()[i].value();

					Mutant::addIdentifierChange(ss.str());
				}
				else
				{
					bBad = true;
					break;
				}
			}
		}

		if( !bBad )
		{
			// cout << m_lBefore.terms()[iIndex].value() << " -> " 
			// 	<< m_lAfter.terms()[iIndex].value() << endl;
		}
	}


	if( m_Options.verbose() )
	{
		cout << "Attempting to extract mutant..." << endl;
		cout << sBefore << endl << sAfter << endl;
	}

	if( static_cast<int>(m_lBefore.size()) > opt.maxTokens() 
		|| static_cast<int>(m_lAfter.size()) > opt.maxTokens() )
	{
		if( m_Options.verbose() )
		{
			cout << "Omitted due to too many tokens" << endl << endl;
		}

		opt.markEvent("1,Too many tokens");

		m_bGood = false;
		return;
	}

	if (m_lAfter.size() < 2)
	{
		if( m_Options.verbose() )
		{
			cout << "Omitted due to output being too small" << endl << endl;
		}

		opt.markEvent("2,Not enough tokens");
		
		m_bGood = false;
		return;
	}

	if (m_lBefore.size() < 1)
	{
		if( m_Options.verbose() )
		{
			cout << "Omitted due to input being too small" << endl << endl;
		}

		opt.markEvent("2,Not enough tokens");
		
		m_bGood = false;
		return;
	}
	
	

	if (m_sSource.find("---") != string::npos
		|| m_sSource.find("===") != string::npos
		|| m_sSource.find("^^^") != string::npos
		|| m_sSource.find("***") != string::npos
		|| m_sSource.find("###") != string::npos
		|| m_sSource.find("+++") != string::npos
		|| m_sSource.find("~~~") != string::npos)
	{
		if( m_Options.verbose() )
		{
			cout << "Omitted due to looking like ASCII art" << endl << endl;
		}

		opt.markEvent("3,ASCII art heuristic");

		m_bGood = false;
		return;
	}

	if (m_lAfter.termCount(tsLiteral) < m_lBefore.termCount(tsLiteral)
		|| m_lAfter.termCount(tsNumber) < m_lBefore.termCount(tsNumber)
		)
	{
		if( m_Options.verbose() )
		{
			cout << "Omitted due to synthesis requirement" << endl << endl;
		}

		opt.markEvent("5,Synthesis required");
		
		m_bGood = false;
		return;
	}

	int iIndex = 1;

	//for (Line::Term & tBefore : m_lBefore.terms())
	for( size_t ii = 0; ii < m_lBefore.terms().size(); ii++ )
	{
		Line::Term & tBefore = m_lBefore.terms()[ii];

		if (tBefore.type() == tsOperator)
		{
			if (tBefore.value() == "/*" || tBefore.value() == "*/")
			{
				if( m_Options.verbose() )
				{
					cout << "Omitted due to comment detection" << endl << endl;
				}

				opt.markEvent("4,Comment detected");
				
				m_bGood = false;
				return;
			}
		}

		if (tBefore.type() == tsIdentifier
			|| tBefore.type() == tsLiteral
			|| tBefore.type() == tsNumber)
		{
			bool bFound = false;

			//for (Line::Term & tAfter : m_lAfter.terms())
			for( size_t jj = 0; jj < m_lAfter.terms().size(); jj++)
			{
				Line::Term & tAfter = m_lAfter.terms()[jj];

				if (tAfter.value() == tBefore.value()
					&& tAfter.type() == tBefore.type() )
				{
					if (tAfter.index() == -1)
					{
						tAfter.setIndex(iIndex);
						tBefore.setIndex(iIndex);
						iIndex++;
					}
					else
					{
						tBefore.setIndex(tAfter.index());
					}

					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				if( m_Options.verbose() )
				{
					cout << "Omitted due to synthesis requirement" << endl << endl;
				}

				opt.markEvent("5,Synthesis required");
				
				m_bGood = false;
				return;
			}
		}
	}

	if (iIndex >= opt.maxLiterals() )
	{
		if( m_Options.verbose() )
		{
			cout << "Omitted due to too many literals (" << (iIndex + 1) << ")" << endl << endl;
		}

		opt.markEvent("6,Too many literals");
		
		m_bGood = false;
		return;
	}


	if( !opt.allowAdjacent() )
	{
		for (size_t i = 0; i + 1 < m_lBefore.terms().size(); i++)
		{
			if (m_lBefore.terms()[i].type() == tsIdentifier &&
				m_lBefore.terms()[i + 1].type() == tsIdentifier)
			{
				if( m_Options.verbose() )
				{
					cout << "Omitted due to adjacent identifiers" << endl << endl;
				}

				opt.markEvent("7,Too many adjacent identifiers");
				
				m_bGood = false;
				return;
			}
		}

		for (size_t i = 0; i + 1 < m_lAfter.terms().size(); i++)
		{
			if (m_lAfter.terms()[i].type() == tsIdentifier &&
				m_lAfter.terms()[i + 1].type() == tsIdentifier)
			{
				if( m_Options.verbose() )
				{
					cout << "Omitted due to adjacent identifiers" << endl << endl;
				}

				opt.markEvent("7,Too many adjacent identifiers");
				
				m_bGood = false;
				return;
			}
		}

	}	

	if (m_lBefore.displayString() == m_lAfter.displayString())
	{
		if( m_Options.verbose() )
		{
			cout << "Omitted due to tokenized streams being identical" << endl << endl;
		}

		opt.markEvent("8,Identical tokenized strings");

		m_bGood = false;
		return;
	}

	map<string, int> mBBraces;
	map<string, int> mABraces;

	for (size_t i = 0; i < m_lBefore.terms().size(); i++)
	{
		if( m_lBefore.terms()[i].type() == tsOperator )
		{
			mBBraces[m_lBefore.terms()[i].value()]++;
		}
	}
	for (size_t i = 0; i < m_lAfter.terms().size(); i++)
	{
		if( m_lAfter.terms()[i].type() == tsOperator )
		{
			mABraces[m_lAfter.terms()[i].value()]++;
		}
	}

	if( mABraces["("] != mBBraces["("] 
		|| mABraces[")"] != mBBraces[")"]
		|| mABraces["["] != mBBraces["["]
		|| mABraces["]"] != mBBraces["]"]
		|| mABraces["{"] != mBBraces["{"]
		|| mABraces["}"] != mBBraces["}"] )
		{
			if( opt.verbose() ) 
			{
				cout << "Omitting due to brace mismatch" << endl;
			}

			opt.markEvent("9,Brace mismatch");
			m_bGood = false;
			return;
		}
	


	if( m_bGood && opt.verbose() )
	{
		cout << "Storing mutant" << endl << endl;
	}

	if( m_bGood )
	{
		opt.markEvent("10,Stored mutant");
	}

	if( !m_bGood ) {
		cout << "Bad mutant" << endl;
	}
}

void /*Mutant::*/Line::tokenize(string /*sLine*/, vector</*Mutant::*/Line::Term> & /*vTerms*/)
{

}

void /*Mutant::*/Line::addTerm(tokenState ts, std::string sValue, 
	size_t iOffset, size_t iSize)
{
	if( ts == tsIdentifier || ts == tsLiteral || ts == tsNumber )
	{
		Mutant::addIdiom(sValue);
	}

	m_vTerms.push_back(Term(ts, sValue, iOffset, iSize));
}



/*Mutant::*/Line::Line(Options & /*opt*/)
	: m_sSource("Serialization")
{

}

/*Mutant::*/Line::Line(string sLine, Options & opt, bool bTarget, string sOriginalSource)
	: m_sSource(sLine)
	, m_sOriginalSource(sOriginalSource)	
{
	if (!bTarget)
	{
		//sLine = sLine.substr(1);

		size_t iComment = sLine.find("//");

		if (iComment != string::npos)
		{
			sLine = sLine.substr(0, iComment);
		}
	}

	string sWord;

	tokenState ts = tsSpace;
	char cLiteral = 0;
	bool bAddLB = false;
	size_t iLastStart = 0;

	for (size_t i = 0; i <= sLine.length(); i++)
	{
		char c;
		char cLast = ' ';
		if (i < sLine.length())
		{
			c = sLine[i];
		}
		else
		{
			c = ' ';
		}

		if (i > 0)
		{
			cLast = sLine[i - 1];
		}


		if (c == '\n' || c == '\x0a')
		{
			bAddLB = true;
		}

		if (c == '\t' || c == '\n' || c == '\x0d' || c == '\x0a')
			c = ' ';

		string s = " ";
		s[0] = c;

		bool bOp;
		bool bBreak;

		if (ts == tsLiteral)
		{
			bBreak = (c == cLiteral) && (cLast != '\\');
			bOp = false;
		}
		else if (ts == tsNumber && c == '.')
		{
			bBreak = false;
			bOp = false;
		}
		else
		{
			//bOp = g_sOperators.find(s) != g_sOperators.end();
			bOp = opt.isOperator(s);
			bBreak = bOp || (c == ' ');

			//cout << s << " : " << bOp << " " << bBreak << " " << sWord << endl;
		}

		if (!bBreak && bAddLB)
		{
			//addTerm(tsLineBreak, "", 0, 0);
			bAddLB = false;
		}

		if (bBreak)
		{
			if (sWord.length() == 0)
			{
				if (bAddLB)
				{
					//addTerm(tsLineBreak, "", 0, 0);
					bAddLB = false;
				}
			}

			if (sWord.length() > 0)
			{
				if (ts == tsLiteral)
				{
					//vRet.push_back(sWord + s);
					addTerm(tsLiteral, sWord + s, iLastStart, 1 + i - iLastStart);

					sWord = "";
					s = "";
				}
				else if (ts == tsNumber)
				{
					//if (m.size() > 0 && vRet.back() == "!-")
					//{
					//	vRet.pop_back();
					//	//vRet.push_back(string("#-") + sWord);
					//	m.terms().push_back(Mutant::Term(tsNumber, string("-") + sWord));
					//}
					//else
					{
						//vRet.push_back(string("#") + sWord);
						//cout << "number : " << sWord << endl;
						addTerm(tsNumber, sWord, iLastStart, i - iLastStart);
					}

					sWord = "";
				}
				//else if (g_sKeywords.find(sWord) != g_sKeywords.end())
				else if( opt.isKeyword(sWord) )
				{
					//vRet.push_back(string("@") + sWord);
					addTerm(tsKeyword, sWord, iLastStart, i - iLastStart);
					sWord = "";
				}
				else
				{
					//vRet.push_back(string("$") + sWord);
					addTerm(tsIdentifier, sWord, iLastStart, i - iLastStart);
					sWord = "";
				}
			}

			if (bOp && ts != tsLiteral)
			{
				//if (size() > 0 && terms().back().type() == tsOperator && g_sOperators.find(terms().back().value() + s) != g_sOperators.end())
				if (size() > 0 && terms().back().type() == tsOperator && opt.isOperator(terms().back().value() + s))
				{
					//string sOp = string("!") + vRet.back().substr(1) + s;
					//vRet.pop_back();
					//vRet.push_back(sOp);

					string sOp = terms().back().value() + s;
					terms().pop_back();

					addTerm(tsOperator, sOp, 1 + i - sOp.size(), 
							sOp.size());
				}
				else
				{
					//vRet.push_back(string("!") + s);
					addTerm(tsOperator, s, 1 + i - s.size(), s.size());
				}
				s = "";
			}

			ts = tsSpace;
		}
		else
		{
			if (sWord.length() == 0)
			{
				if (c >= '0' && c <= '9')
				{
					ts = tsNumber;
				}
				else if (c == '\'' || c == '"')
				{
					bool bValid = true;

					for (int j = terms().size() - 1; j >= 0; j--)
					{
						if (terms()[j].type() == tsOperator )
						{
							if (terms()[j].value() == "*/")
							{
								break;
							}
							else if (terms()[j].value() == "/*")
							{
								bValid = false;
								break;
							}
						}
					}

					if (bValid)
					{
						ts = tsLiteral;
						cLiteral = c;
					}
					else
					{
						ts = tsSpace;
					}
				}
				else
				{
					ts = tsIdentifier;
				}
			}

			if (ts != tsSpace && sWord.size() == 0)
			{
				iLastStart = i;
			}

			sWord += s;
		}

	}

	vector<pair<size_t, size_t> > vComments;

	for (size_t i = 0; i < m_vTerms.size(); i++)
	{
		if (m_vTerms[i].type() == tsOperator
			&& m_vTerms[i].value() == "/*")
		{
			for (size_t k = i + 1; k < m_vTerms.size(); k++)
			{
				if (m_vTerms[k].type() == tsOperator &&
					m_vTerms[k].value() == "*/")
				{
					vComments.push_back(pair<size_t, size_t>(i, k+1));
					i = k;
					break;
				}
			}
		}
	}

	while( vComments.size() > 0 )
	{
		m_vTerms.erase(m_vTerms.begin() + vComments.back().first,
			m_vTerms.begin() + vComments.back().second);

		vComments.pop_back();
	}
}

void Mutant::blah(Line * pTarget)
{
	if (pTarget == NULL)
		return;

	for (size_t i = 0; i + m_lAfter.size() < pTarget->size(); i++)
	{
		string s = matchAt(i, pTarget);

		//	if (s != "")
		//		return s;
	}
}

bool Mutant::testMatch(size_t iOffset, Line * pTarget)
{
	if( pTarget->terms().size() < iOffset + m_lAfter.size() )
		return false;

	for (size_t i = 0; i < m_lAfter.size(); i++)
	{
		if (m_lAfter.terms()[i].type() != pTarget->terms()[iOffset + i].type())
			return false;

		if (m_lAfter.terms()[i].type() == tsKeyword
			|| m_lAfter.terms()[i].type() == tsOperator)
		{
			if (m_lAfter.terms()[i].value() != pTarget->terms()[iOffset + i].value())
				return false;
		}
	}

	return true;
}

string Mutant::matchAt(size_t iOffset, Line * pTarget)
{
	for (size_t i = 0; i < m_lAfter.size(); i++)
	{
		if (m_lAfter.terms()[i].type() != pTarget->terms()[iOffset + i].type())
			return "";

		if (m_lAfter.terms()[i].type() == tsKeyword
			|| m_lAfter.terms()[i].type() == tsOperator)
		{
			if (m_lAfter.terms()[i].value() != pTarget->terms()[iOffset + i].value())
				return "";
		}
	}

	cout << m_lAfter.displayString() << " ==> ";

	for (size_t i = 0; i < m_lAfter.size(); i++)
	{
		cout << pTarget->terms()[i + iOffset].displayString(true) << " ";
	}


	cout << endl;

	return "";

}

string Mutant::match()
{
	//if (g_pTarget == nullptr)
	//	return "";

	//for (size_t i = 0; i + m_lAfter.size() < g_pTarget->size(); i++)
	//{
	//	string s = matchAt(i);

	//	if (s != "")
	//		return s;
	//}

	return "";
}


void Mutant::display(bool bRaw)
{
	std::cout << m_lAfter.displayString(bRaw)
		<< " ::: "
		<< m_lBefore.displayString(bRaw);

	std::cout << "// "
		<< m_lAfter.source()
		<< " :: "
		<< m_lBefore.source();

	std::cout << std::endl;
}

string Mutant::serialize()
{
	string sRet = "M " + m_lBefore.displayString() + " : " + m_lAfter.displayString();

	if( !m_Options.trimText() && !source().empty() )
	{
		sRet += " @ " + source(); 
	}

	return sRet;
}

string IdentifierShift::serialize() 
{ 
	return "I " + m_pBefore->displayString() + " " + m_pAfter->displayString();; 
}


string /*Mutant::*/Line::displayString(bool bRaw)
{
	stringstream ss;

	//for (Term & t : terms())
	for( size_t i = 0; i < terms().size(); i++ )
	{
		Term & t = terms()[i];

		ss << t.displayString(bRaw) << " ";
	}

	return ss.str();
}


string /*Mutant::*/Line::Term::displayString(bool bRaw) const
{
	stringstream ss;

	if (bRaw)
	{
		ss << value();
	}
	else switch (type())
	{
	case tsNumber:
		ss << "#" << index();
		break;

	case tsIdentifier:
		ss << "$" << index();
		break;

	case tsOperator:
		ss << "." << value();
		break;

	case tsKeyword:
		ss << "_" << value();
		break;

	case tsLiteral:
		//ss << "\"" << value() << "\"";
		ss << "=" << index();
		break;

	default:
		ss << "?" << value();
		break;

	}

	return ss.str();
}


bool Mutant::isReplaceOperator()
{
	if( m_lBefore.terms().size() != m_lAfter.terms().size())
		return false;

	bool bChanged = false;

	for( size_t i = 0; i < m_lBefore.terms().size(); i++ )
	{
		if( m_lBefore.terms()[i].type() != m_lAfter.terms()[i].type() )
			return false;

		if( m_lBefore.terms()[i].value() != m_lAfter.terms()[i].value() )
		{
			if( bChanged )
			{
				return false;
			}
			if( m_lBefore.terms()[i].type() != tsOperator )
			{
				return false;
			}
			else
			{
				bChanged = true;
			}
		}
	}

	return true;
}

bool Mutant::isDeleteStatement()
{
	if( m_lAfter.terms().size() >= m_lBefore.terms().size() )
	{
		return false;
	}

	vector<size_t> vStatements;
	size_t iLastSemi = 0;

	for( size_t i = 0; i < m_lBefore.terms().size(); i++ )
	{
		if( m_lBefore.terms()[i].value() == ";" )
		{
			vStatements.push_back(1 + (i - iLastSemi));
			iLastSemi = i;
		}
	}

	//for( size_t iSize : vStatements )
	for( size_t i = 0; i < vStatements.size(); i++ )
	{
		size_t iSize = vStatements[i];

		if( m_lAfter.terms().size() - m_lBefore.terms().size() == iSize )
			return true;
	}
	
	return false;
}

bool Mutant::isModifyBranch()
{
	map<string, bool> mBefore;
	map<string, bool> mAfter;

	for( size_t i = 0; i < m_lBefore.terms().size(); i++ )
	{
		mBefore[m_lBefore.terms()[i].value()] = true;
	}

	for( size_t i = 0; i < m_lAfter.terms().size(); i++ )
	{
		mAfter[m_lAfter.terms()[i].value()] = true;
	}

	vector<string> vMust;// = { "(", ")"};
	vMust.push_back("(");
	vMust.push_back(")");

	//for( string sMust : vMust )
	for( size_t i = 0; i < vMust.size(); i++)
	{
		string sMust = vMust[i];

		if( mBefore.find(sMust) == mBefore.end()
			|| mAfter.find(sMust) == mAfter.end() )
			{
				return false;
			}
	}

	vector<string> vFind; // = { "if", "for", "while", "do" };
	vFind.push_back("if");
	vFind.push_back("for");
	vFind.push_back("while");
	vFind.push_back("do");


	//for( string sFind : vFind )
	for( size_t i = 0; i < vFind.size(); i++ )
	{
		string sFind = vFind[i];
		
		if( mBefore.find(sFind) != mBefore.end()
			&& mAfter.find(sFind) != mAfter.end() )
			{
				return true;
			}
	}


	return false;
}

bool sortIdioms(string a, string b)
{
	return Mutant::s_mIdioms[a] > Mutant::s_mIdioms[b];
}

bool sortIS(string a, string b)
{
	return Mutant::s_mIdentifierChange[a] > Mutant::s_mIdentifierChange[b];
}



void Mutant::showIdioms()
{
	{
		size_t iCount = 0;

		vector<string> vKeys;

		//for( auto it : s_mIdioms )
		for( std::map<std::string, size_t>::iterator it = s_mIdioms.begin();
			it != s_mIdioms.end(); it++)
		{
			iCount += it->second;
			vKeys.push_back(it->first);
		} 

		cout << "Total of " << iCount << " identifiers/literals found." << endl;

		sort(vKeys.begin(), vKeys.end(), sortIdioms);
		// 	[&](string a, string b)
		// 	{

		// 		return s_mIdioms[a] > s_mIdioms[b];

		// });

		for( size_t i = 0; i < vKeys.size(); i++ )
		{
			if( s_mIdioms[vKeys[i]] > iCount / 2000 )
			{
				cout << vKeys[i] << " : " << s_mIdioms[vKeys[i]] << endl;
			}
		}
	}

	cout << endl;

	{
		size_t iCount = 0;

		vector<string> vKeys;

		//for( auto it : s_mIdentifierChange )
		for( std::map<std::string, size_t>::iterator it = s_mIdentifierChange.begin();
			it != s_mIdentifierChange.end(); it++ )
		{
			iCount += it->second;
			vKeys.push_back(it->first);
		} 

		cout << "Total of " << iCount << " identifier changes found." << endl;

		sort(vKeys.begin(), vKeys.end(), sortIS);
		// 	[&](string a, string b)
		// 	{

		// 		return s_mIdentifierChange[a] > s_mIdentifierChange[b];

		// });

		for( size_t i = 0; i < vKeys.size(); i++ )
		{
			if( s_mIdentifierChange[vKeys[i]] > 2 )
			{
				cout << vKeys[i] << " : " << s_mIdentifierChange[vKeys[i]] << endl;
			}
		}
	}
}

IdentifierShift::IdentifierShift(std::string sLine, Options & /*opt*/)
{
	istringstream ss(sLine);

	string sBefore, sAfter;

	ss >> sBefore;
	ss >> sAfter;

	m_pBefore = new Line::Term(tsExactIdentifier, sBefore, -1, -1);
	m_pAfter = new Line::Term(tsExactIdentifier, sAfter, -1, -1);
}

bool IdentifierShift::testMatch(size_t iOffset, Line * pTarget) 
{ 
	if( pTarget->terms()[iOffset].type() != tsIdentifier )
		return false;

	//cout << pTarget->terms()[iOffset].value() << " ";

	return pTarget->terms()[iOffset].value() == m_pBefore->value(); 
}
