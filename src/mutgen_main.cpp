#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>

#include "mutins.h"
#include "options.h"

using namespace std;
using namespace mutins;


void extractMutants(Options & opt, ifstream & in, ofstream & out)
{
	string sPre;
	string sPost;

	while (in.good())
	{
		string sLine;

		getline(in, sLine);

		bool bCheck = true;

		if (sLine.length() < 3)
			bCheck = false;

		for (char c : sLine)
		{
			if (c < ' ' || c > 126)
			{
				bCheck = false;
				break;
			}
		}

		if( bCheck && sLine[0] == '-' )
		{
			sPost = "";
			sPre += sLine.substr(1);
		}
		else if( bCheck && sLine[0] == '+' )
		{
			sPost += " " + sLine.substr(1);
		}
		else if( !sPre.empty() && !sPost.empty() )
		{
			string sA, sB;

			if( opt.forwardMutants() )
			{
				sA = sPost;
				sB = sPre;
			}
			else
			{
				sA = sPre;
				sB = sPost;
			}

			Mutant m(sA, sB, opt);

			if( m.good() )
			{
				out << m.serialize() << endl;
			}

			sPre = "";
			sPost = "";
		}
	}


}

int main(int argc, char * argv[])
{
	Options opt(true);

	if (!opt.parse(argc, argv))
		return 1;

	ofstream out;
	out.open(opt.extractFile());

	if (!out.good())
	{
		cout << "Unable to open extraction output file " << opt.extractFile()
			<< "." << endl;
		return 1;
	}

	if (opt.keywords().size() > 0)
	{
		out << "K ";

		for (string s : opt.keywords())
		{
			out << s << " ";
		}

		out << endl;
	}

	if (opt.operators().size() > 0)
	{
		out << "O ";

		for (string s : opt.operators())
		{
			out << s << " ";
		}

		out << endl;
	}

	for (string sInput : opt.inputFiles())
	{
		ifstream in;
		in.open(sInput);

		if (!in.good())
		{
			cout << "Unable to open input file " << sInput << "." << endl;
			out.close();
			return 1;
		}

		extractMutants(opt, in, out);

		in.close();
	}

	out.close();

	if( !opt.extractFile().empty() )
	{
		string sCmd = "awk -F'@' '!seen[$1]++' ";
		sCmd += opt.extractFile();
		sCmd += " > ";
		sCmd += opt.extractFile() + ".tmp";
		system(sCmd.c_str());

		sCmd = "rm " + opt.extractFile();
		system(sCmd.c_str());

		sCmd = "mv " + opt.extractFile() + ".tmp " + opt.extractFile();
		system(sCmd.c_str());
	}

	return 0;
}
