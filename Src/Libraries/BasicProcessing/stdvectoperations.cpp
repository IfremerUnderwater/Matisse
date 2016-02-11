#include "stdvectoperations.h"


double doubleVectorMean( vector<double> & v_p )
{
        double sum = 0.0;
        int n = v_p.size();

        for ( int i=0; i < n; i++)
        {
            sum += v_p[i];
        }

        return ( sum / (double)n);
}


void doubleVectorScalarMult(vector<double> &v_p, double m_p)
{

    for (unsigned int i=0; i < v_p.size(); i++)
    {
        v_p[i] *= m_p;
    }

}
