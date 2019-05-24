#include <iostream>
#include <string>

using namespace std;

int main()
{
    int iMutant, iIndex;
    cin >> iMutant;
    cin >> iIndex;

    int iCompilation;
    cin >> iCompilation;

    string sDiff;
    cin >> sDiff;

    int iBinaryEquiv;
    cin >> iBinaryEquiv;

    int iCount;
    cin >> iCount;


    cout << "-m " << iMutant << " -i " << iIndex << " --label " << iBinaryEquiv;


    return 0;
}
