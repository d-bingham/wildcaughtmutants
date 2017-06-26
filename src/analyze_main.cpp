#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>

using namespace std;


map<string,int> g_mPassCount;
map<string,int> g_mFailCount;
map<string,double> g_mAms;
vector<int> g_vMutantSet;
vector<int> g_vSuiteKills(5000, 0);
int g_iNonEquivalentMutants = 0;

vector<vector<int> > g_vvSuites;

mt19937 mt(time(NULL));


void testMutant(vector<int> & vPass, string sIndex)
{
   bool bEquivalent = true;

    for( size_t i = 0; i < vPass.size(); i++ )
    {
        if( vPass[i] == -1 )
        {
            cerr << "Missing data " << i << endl;
            exit(1);
        }
        else if( vPass[i] == 0 )
        {
            bEquivalent = false;
        }
    }

    if( bEquivalent )
    {
        //cout << "Equivalent mutant: " << sIndex << endl;

        for( size_t i = 0; i < vPass.size(); i++ )
        {
            vPass[i] = -1;
        }

        return;
    }

    istringstream ss(sIndex);
    string sM;
    int iMutant, iIndex;
    ss >> sM;
    ss >> iMutant;
    ss >> sM;
    ss >> iIndex;

    if( (g_vMutantSet[iMutant] & 2) == 0 )
    {
        return;
    }

    g_iNonEquivalentMutants++;
    
    //int iPassCount = 0;
    //int iFailCount = 0;

    // for( size_t i = 0; i < g_vvSuites.size(); i++ )
    // {
    //     bool bSuitePassed = true;
    //     for( size_t j = 0; j < g_vvSuites[i].size(); j++ )
    //     {
    //         //cout << vPass[g_vvSuites[i][j]] << " ";

    //         if( vPass[g_vvSuites[i][j]] == 0 )
    //         {
    //             bSuitePassed = false;
    //             break;
    //         }
    //     }

    //     //cout << endl;

    //     if( bSuitePassed )
    //     {
    //         iPassCount++;
    //     }
    //     else
    //     {
    //         iFailCount++;
    //     }
    // }

    for( size_t i = 0; i < g_vvSuites.size(); i++ )
    {
        vector<int> & vSuite = g_vvSuites[i];

        bool bKilled = false;

        for( size_t j = 0; j < vSuite.size(); j++ )
        {
            if( vPass[vSuite[j]] == 0 )
            {
                bKilled = true;
                break;
            }
        }

        if( bKilled )
        {
            g_vSuiteKills[i]++;
        }
    }

    for( size_t i = 0; i < vPass.size(); i++ )
    {
        vPass[i] = -1;
    }


}

void loadResultsFile(string sFilename)
{
    ifstream in;
    in.open(sFilename);

    string sIndex;

    vector<int> vPass(13496, -1);

    bool bFirst = true;

    size_t iCase = 0;

    while( in.good() )
    {
        string s;
        getline(in, s);

        if( s.length() < 1 )
        {
            continue;
        }

        if( !sIndex.empty() )
        {
            auto it = g_mPassCount.find(sIndex);
            if( it == g_mPassCount.end() )
            {
                g_mPassCount[sIndex] = 0;
            }

            it = g_mFailCount.find(sIndex);
            if( it == g_mFailCount.end() )
            {
                g_mFailCount[sIndex] = 0;
            }

        }

        if( s[0] == 'P')
        {
            g_mPassCount[sIndex]++;
            vPass[iCase] = 1;
            iCase++;
        }
        else if( s[0] == 'F' )
        {
            g_mFailCount[sIndex]++;
            vPass[iCase] = 0;
            iCase++;
        }
        else
        {
            // istringstream ss(s);

            // string sM;
            // ss >> sM;

            // int iMutant, iInsertion;
            // ss >> iMutant;

            // ss >> sM;

            // ss >> iInsertionl

            // stringstream ssIndex;
            // ssIndex << iMutant << "_" << iInsertion;

            if( !bFirst )
            {
                testMutant(vPass, sIndex);
            }

            

            bFirst = false;

            iCase = 0;
            sIndex = s;
            //cout << sIndex << ":" << endl;
        }
    }

    testMutant(vPass, sIndex);
}

int main(int argc, char * argv[])
{
    for( int i = 0; i < 5000; i++ )
    {
        g_vvSuites.push_back(vector<int>());
        for( int j = 0; j < 100; j++ )
        {
            g_vvSuites.back().push_back(mt() % 13496);
        }
    }


    {
        ifstream in;
        in.open("/home/bingham/mutants/msets.txt");

        int i = 0;

        while( in.good() )
        {
            int iIndex, iSet;
            in >> iIndex;
            in >> iSet;

            if( !in.good() )
            {
                break;
            }

            if( iIndex != i ) 
            {
                cout << "Error in mutant set file " << iIndex << " " <<  i  << endl;
                exit(1);
            }

            i++;

            g_vMutantSet.push_back(iSet);
        }
    }




    for( int i = 1; i < argc; i++ ) 
    {
        loadResultsFile(argv[i]);
    }

    vector<double> vAms;
    double dAmsTotal = 0.0;

    for( size_t i = 0; i < g_vvSuites.size(); i++ )
    {
        vAms.push_back((double) g_vSuiteKills[i] / (double) g_iNonEquivalentMutants);
        dAmsTotal += vAms.back();
    }

    sort(vAms.begin(), vAms.end());

    cerr << "Am(S) average: " << dAmsTotal / 5000 << endl;
    cerr << "Am(S) median: " << ((vAms[2499] + vAms[2500])/2.0) << endl;

    for( size_t i = 0; i < vAms.size(); i++ )
    {
        cout << vAms[i] << endl;
    }


    // for( auto it : g_mPassCount )
    // {
    //     string sIndex = it.first;
    //     istringstream ss(sIndex);

    //     string sM;
    //     int iMutant;
    //     int iIndex;
    //     ss >> sM;
    //     ss >> iMutant;
    //     ss >> sM;
    //     ss >> iIndex;

    //     cout << iMutant << "," << iIndex << ","
    //         << g_mPassCount[sIndex] << ","
    //         << g_mFailCount[sIndex] << ",";

    //     if( g_mFailCount[sIndex] == 0 ) {
    //         cout << "bad,";
    //     }
    //     else {
    //         cout << "good,";
    //     }

    //     if( g_vMutantSet[iMutant] == 0 ) {
    //         cout << "shift";
    //     }
    //     else if( g_vMutantSet[iMutant] == 1 ) {
    //         cout << "forward";
    //     }
    //     else if( g_vMutantSet[iMutant] == 2 ) {
    //         cout << "reverse";
    //     }
    //     else if( g_vMutantSet[iMutant] == 3 ) {
    //         cout << "both";
    //     }

    //     cout << "," << g_mAms[sIndex];


    //     cout << endl;
    // }
}