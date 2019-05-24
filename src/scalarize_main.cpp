#include <iostream>
#include <string>
#include <sstream>
#include <map>

using namespace std;

int main(int /*argc*/, char * /*argv*/[])
{
    map<string, int> mScalar;

    while( cin.good() )
    {
        string sLine;
        getline(cin, sLine);

        istringstream ss(sLine);

        int iLabel;
        ss >> iLabel;

        cout << iLabel;

        while( ss.good() )
        {
            string s;
            ss >> s;

            int iIndex = -1;

            auto found = mScalar.find(s);

            if( found == mScalar.end() )
            {
                iIndex = mScalar.size() + 1;
                mScalar[s] = iIndex;
            }
            else 
            {
                iIndex = found->second;
            }

            cout << " " << iIndex;
        }

        cout << endl;
    }

    return 0;
}
