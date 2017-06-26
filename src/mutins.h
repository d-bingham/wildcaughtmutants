#ifndef _mutins_included_
#define _mutins_included_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <map>

namespace mutins
{
class Mutant;
class Line;

enum tokenState
{
    tsSpace,
    tsOperator,
    tsIdentifier,
    tsNumber,
    tsLiteral,
    tsKeyword,
    tsLineBreak,
    tsExactIdentifier
};

class Options;

class Line
{
  public:
    Line(std::string sLine, Options &opt, bool bTarget = false);
    Line(Options &opt);

    class Term
    {
      public:
	Term(tokenState ts, std::string sValue,
	     size_t iOffset, size_t iSize)
	    : m_tsType(ts), m_sValue(sValue), m_iIndex(-1), m_iOffset(iOffset), m_iSize(iSize)
	{
	}

	tokenState type() const { return m_tsType; }
	std::string value() const { return m_sValue; }
	int index() const { return m_iIndex; }
	void setIndex(int iIndex) { m_iIndex = iIndex; }
	std::string displayString(bool bRaw = false) const;
	size_t offset() const { return m_iOffset; }
	size_t size() const { return m_iSize; }

      private:
	tokenState m_tsType;
	std::string m_sValue;
	int m_iIndex;
	size_t m_iOffset;
	size_t m_iSize;
    };

    static void tokenize(std::string sLine, std::vector<Term> &vTerms);
    std::string displayString(bool bRaw = false);

    size_t size() const { return m_vTerms.size(); }

    void addTerm(tokenState ts, std::string sValue,
		 size_t iOffset, size_t iSize);

    size_t termCount(tokenState ts)
    {
	size_t iCount = 0;

	//for (Term &t : m_vTerms)
    for( size_t i = 0; i < m_vTerms.size(); i++)
	{
        Term & t = m_vTerms[i];

	    if (t.type() == ts)
		iCount++;
	}

	return iCount;
    }

    std::vector<Term> &terms() { return m_vTerms; }
    std::string source() { return m_sSource; }

  private:
    std::vector<Term> m_vTerms;
    std::string m_sSource;
};

class MutantBase
{
  public:
    virtual size_t matchSize() = 0;
    virtual const Line::Term &matchTerm(size_t iIndex) = 0;
    virtual size_t replaceSize() = 0;
    virtual const Line::Term &replaceTerm(size_t iIndex) = 0;
    virtual bool testMatch(size_t iOffset, Line *pLine) = 0;
    virtual ~MutantBase() {}
    virtual std::string serialize() = 0;
};

class IdentifierShift : public MutantBase
{
  public:
    virtual size_t matchSize() { return 1; }
    virtual const Line::Term &matchTerm(size_t /*iIndex*/) 
	{
		return *m_pBefore;
	}

    virtual size_t replaceSize() { return 1; }

    virtual const Line::Term &replaceTerm(size_t /*iIndex*/)
	{
		return *m_pAfter;
	}

    virtual bool testMatch(size_t iOffset, Line * pTarget);
    virtual ~IdentifierShift() {
		delete m_pBefore;
		delete m_pAfter;
	}
    virtual std::string serialize();

    IdentifierShift(std::string /*sLine*/, Options & /*opt*/);

  private:
  	Line::Term * m_pBefore;
	Line::Term * m_pAfter;
};

class Mutant : public MutantBase
{
  public:
    Mutant(std::string sBefore, std::string sAfter, Options &opt);
    Mutant(std::string sSerial, Options &opt);

    virtual size_t matchSize() { return m_lAfter.size(); }
    virtual const Line::Term &matchTerm(size_t iIndex) { return m_lAfter.terms()[iIndex]; }
    virtual size_t replaceSize() { return m_lBefore.size(); }
    virtual const Line::Term &replaceTerm(size_t iIndex) { return m_lBefore.terms()[iIndex]; }

    bool good() { return m_bGood; }
    void display(bool bRaw = false);
    bool isReplaceOperator();
    bool isDeleteStatement();
    bool isModifyBranch();
    virtual std::string serialize();

    void blah(Line *pTarget);
    std::string match();
    std::string matchAt(size_t iOffset, Line *pTarget);
    virtual bool testMatch(size_t iOffset, Line *pTarget);
    std::string source() { return m_sSource; }

    Line &before() { return m_lBefore; }
    Line &after() { return m_lAfter; }

    static void addIdiom(std::string sIdiom)
    {
	std::map<std::string, size_t>::iterator it = s_mIdioms.find(sIdiom);

	if (it == s_mIdioms.end())
	{
	    s_mIdioms[sIdiom] = 1;
	}
	else
	{
	    s_mIdioms[sIdiom]++;
	}
    }

    static void addIdentifierChange(std::string sIdiom)
    {
	std::map<std::string, size_t>::iterator it = s_mIdentifierChange.find(sIdiom);

	if (it == s_mIdentifierChange.end())
	{
	    s_mIdentifierChange[sIdiom] = 1;
	}
	else
	{
	    s_mIdentifierChange[sIdiom]++;
	}
    }

    static void showIdioms();

  private:
    bool m_bGood;
    Line m_lBefore;
    Line m_lAfter;
    Options &m_Options;
    std::string m_sSource;

public:
    static std::map<std::string, size_t> s_mIdioms;
    static std::map<std::string, size_t> s_mIdentifierChange;
};
};

#endif