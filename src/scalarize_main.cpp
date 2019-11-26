#include <iostream>
#include <string>
#include <sstream>
#include <map>

using namespace std;

int scalarize(map<string, int> & mScalar, string s) {

		int iIndex = 1;
		auto found = mScalar.find(s);
		if( found == mScalar.end() ) {
			iIndex = mScalar.size() + 1;
			mScalar[s] = iIndex;
		}
		else {
			iIndex = found->second;
		}

		return iIndex;
}


int main(int argc, char * argv[])
{
    map<string, int> mScalar;

    int iFill = -1;

    if( argc > 1 ) {
	    iFill = atoi(argv[1]);
    }

    while( cin.good() )
    {
        string sLine;
        getline(cin, sLine);

	if( sLine.length() < 3 ) {
		continue;
	}

        istringstream ss(sLine);

        int iLabel;
        ss >> iLabel;

        cout << iLabel;

	int iVectorCount = 0;

        while( ss.good() )
        {
		iVectorCount++;

		if( iFill > 0 && iVectorCount > iFill ) {
			break;
		}

            string s;
            ss >> s;

            int iIndex = scalarize(mScalar, s);

            cout << " " << iIndex;
        }

	if( iFill > 0 ) {
		int iIndex = scalarize(mScalar, "~~empty");

		while( iVectorCount < iFill ) {
			iVectorCount++;
			cout << " " << iIndex;
		}
	}

        cout << endl;
    }

    return 0;
}
