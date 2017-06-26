#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char * argv[])
{
    vector<vector<string> > vvMutants;

    for( int i = 1; i < argc; i++ )
    {
        ifstream in;
        in.open(argv[i]);

        vvMutants.push_back(vector<string>());

        while( in.good() )
        {
            string sLine;

            getline(in, sLine);

            if( sLine.length() < 1 || (sLine[0] != 'M' && sLine[0] != 'I') )
            {
                continue;
            }

            vvMutants.back().push_back(sLine);

        }
    }

    for( size_t i = 0; i < vvMutants[0].size(); i++ )
    {
        string sBase = vvMutants[0][i];

        while( sBase.back() == ' ')
        {
            sBase.pop_back();
        }

        //cout << sBase << "|" << endl;
        int iCount = 0;
        //int iChecked = 0;

        for( size_t j = 1; j < vvMutants.size(); j++ )
        {
            for( size_t k = 0; k < vvMutants[j].size(); k++ )
            {
                string sCheck = vvMutants[j][k];

                while( sCheck.back() == ' ')
                {
                    sCheck.pop_back();
                }

                if( sBase == sCheck )
                {
                    iCount += j;
                    break;
                }
            }

            // cout << vvMutants[j].back() << "|" << endl;
            // cout << (vvMutants[j].back() == vvMutants[0][i]) << " "
            //     << vvMutants[j].back().size() << " "
            //     << vvMutants[0][i].size()
            //     << endl;
        }

        cout << i << " ";
        cout << iCount 
            //<< " " << iChecked 
            <<  endl;
    }
    
}