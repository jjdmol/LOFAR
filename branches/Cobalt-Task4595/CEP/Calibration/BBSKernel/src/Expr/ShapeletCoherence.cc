//# ShapeletCoherence.cc: Spatial coherence function of an elliptical gaussian
//# source.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: ShapeletCoherence.cc 16068 2010-07-23 19:53:58Z zwieten $

#include <lofar_config.h>

#include <BBSKernel/Expr/ShapeletCoherence.h>

#include <Common/lofar_complex.h>
#include <Common/lofar_math.h>
#include <Common/LofarLogger.h>

#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{
using LOFAR::exp;
using LOFAR::conj;

/******************* shapelet related code block **************************/
/* evaluate Hermite polynomial value using recursion
 */
static double
H_e(double x, int n) {
    if(n==0) return 1.0;
    if(n==1) return 2*x;
    return 2*x*H_e(x,n-1)-2*(n-1)*H_e(x,n-2);
}


/* struct for sorting coordinates */
typedef struct coordval_{
    double val;
    int idx;
} coordval;

/* comparison */
static int compare_coordinates(const void *a, const void *b) {
    const coordval *da=(const coordval *)a;
    const coordval *db=(const coordval *)b;
    return(da->val>=db->val?1:-1);
}

/* calculate mode vectors for each (u,v) point given by the arrays u, v
 * of equal length.
 *
 * in: u,v: arrays of the grid points
 *      N: number of grid points
 *      beta: scale factor
 *      n0: number of modes in each dimension
 * out:
 *      Av: array of mode vectors size N times n0.n0, in column major order
 *      cplx: array of integers, size n0*n0, if 1 this mode is imaginary, else real
 *
 */
static int calculate_uv_mode_vectors_bi(double *u, double *v, int N,
    double beta, int n0, double **Av, int **cplx) {

    double *grid;
    int *xindex,*yindex;
    int xci,yci,zci,Ntot;
    int *neg_grid;

    double **shpvl, *fact;
    int n1,n2,start;
    double xval;

    /* for sorting */
    coordval *cx_val,*cy_val;
    int signval;

    /* image size: N pixels
    */
    /* allocate memory to store grid points: max unique points are 2N */
    if ((grid=(double*)calloc((size_t)(2*N),sizeof(double)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }

    if ((xindex=(int*)calloc((size_t)(N),sizeof(int)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }
    if ((yindex=(int*)calloc((size_t)(N),sizeof(int)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }

    /* sort coordinates */
    if ((cx_val=(coordval*)calloc((size_t)(N),sizeof(coordval)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }
    if ((cy_val=(coordval*)calloc((size_t)(N),sizeof(coordval)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }

    for (xci=0; xci<N; xci++) {
        cx_val[xci].idx=xci;
        cx_val[xci].val=u[xci];
        cy_val[xci].idx=xci;
        cy_val[xci].val=v[xci];
    }

#ifdef SBYDEBUG
    printf("Before sort id x, y\n");
    for (xci=0; xci<N; xci++) {
        printf("%d %lf %lf\n",xci,cx_val[xci].val,cy_val[xci].val);
    }
#endif
    qsort(cx_val,N,sizeof(coordval),compare_coordinates);
    qsort(cy_val,N,sizeof(coordval),compare_coordinates);
#ifdef SBYDEBUG
    printf("After sort id x, y\n");
    for (xci=0; xci<N; xci++) {
        printf("%d %lf %lf\n",xci,cx_val[xci].val,cy_val[xci].val);
    }
#endif

    /* merge axes coordinates */
    xci=yci=zci=0;
    while(xci<N && yci<N ) {
        if (cx_val[xci].val==cy_val[yci].val){
            /* common index */
            grid[zci]=cx_val[xci].val;
            /*xindex[xci]=zci;
            yindex[yci]=zci; */
            xindex[cx_val[xci].idx]=zci;
            yindex[cy_val[yci].idx]=zci;
            zci++;
            xci++;
            yci++;
        } else if (cx_val[xci].val<cy_val[yci].val){
            grid[zci]=cx_val[xci].val;
            //xindex[xci]=zci;
            xindex[cx_val[xci].idx]=zci;
            zci++;
            xci++;
        } else {
            grid[zci]=cy_val[yci].val;
            //yindex[yci]=zci;
            yindex[cy_val[yci].idx]=zci;
            zci++;
            yci++;
        }
    }
    /* copy the tail */
    if(xci<N && yci==N) {
        /* tail from x */
        while(xci<N) {
            grid[zci]=cx_val[xci].val;
            //xindex[xci]=zci;
            xindex[cx_val[xci].idx]=zci;
            zci++;
            xci++;
        }
    } else if (xci==N && yci<N) {
        /* tail from y */
        while(yci<N) {
            grid[zci]=cy_val[yci].val;
            //yindex[yci]=zci;
            yindex[cy_val[yci].idx]=zci;
            zci++;
            yci++;
        }
    }
    Ntot=zci;

    if (Ntot<2) {
        fprintf(stderr,"Error: Need at least 2 grid points\n");
        exit(1);
    }
#ifdef SBYDEBUG
    printf("Input coord points\n");
    for (xci=0; xci<N; xci++) {
        printf("[%d]=%lf %lf ",xci,u[xci],v[xci]);
    }
    printf("Grid\n");
    for (xci=0; xci<Ntot; xci++) {
        printf("[%d]=%lf ",xci,grid[xci]);
    }
    printf("Index x,y\n");
    for (xci=0; xci<N; xci++) {
        printf("[%d]=%d %d ",xci,xindex[xci],yindex[xci]);
    }
    printf("\n");
#endif
    /* wrap up negative values to positive ones if possible */
    if ((neg_grid=(int*)calloc((size_t)(Ntot),sizeof(int)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }
    for (zci=0; zci<Ntot; zci++) {
        neg_grid[zci]=-1;
    }
    zci=Ntot-1;
    xci=0;
    /* find positive values to all negative values if possible */
    while(xci<Ntot && grid[xci]<0) {
        /* try to find a positive value for this is possible */
        while(zci>=0 && grid[zci]>0 && -grid[xci]<grid[zci]) {
            zci--;
        }
        /* now we might have found a correct value */
        if (zci>=0 && grid[zci]>0 && -grid[xci]==grid[zci]) {
            neg_grid[xci]=zci;
        }
        xci++;
    }

#ifdef SBYDEBUG
    printf("Neg grid\n");
    for (xci=0; xci<Ntot; xci++) {
        printf("[%d]=%d ",xci,neg_grid[xci]);
    }
    printf("\n");
#endif

    /* set up factorial array */
    if ((fact=(double*)calloc((size_t)(n0),sizeof(double)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }
    fact[0]=1;
    for (xci=1; xci<(n0); xci++) {
        fact[xci]=(xci+1)*fact[xci-1];
    }

#ifdef SBYDEBUG
    printf("Factorials\n");
    for (xci=0; xci<(n0); xci++) {
        printf("[%d]=%lf ",xci,fact[xci]);
    }
    printf("\n");
#endif

    /* setup array to store calculated shapelet value */
    /* need max storage Ntot x n0 */
    if ((shpvl=(double**)calloc((size_t)(Ntot),sizeof(double*)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }
    for (xci=0; xci<Ntot; xci++) {
        if ((shpvl[xci]=(double*)calloc((size_t)(n0),sizeof(double)))==0) {
            fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
            exit(1);
        }
    }

    if ((*cplx=(int*)calloc((size_t)((n0)*(n0)),sizeof(int)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }

    /* start filling in the array from the positive values */
    for (zci=Ntot-1; zci>=0; zci--) {
    /* check to see if there are any positive values */
        if (neg_grid[zci] !=-1) {
            /* copy in the values from positive one, with appropriate sign change */
            for (xci=0; xci<(n0); xci++) {
                shpvl[zci][xci]=(xci%2==1?-shpvl[neg_grid[zci]][xci]:shpvl[neg_grid[zci]][xci]);
            }
        } else {
            for (xci=0; xci<(n0); xci++) {
                /*take into account the scaling */
                xval=grid[zci]*(beta);
                shpvl[zci][xci]=H_e(xval,xci)*exp(-0.5*xval*xval)/std::sqrt((2<<xci)*fact[xci]);
            }
        }
    }

#ifdef SBYDEBUG
    printf("x, shapelet val\n");
    for (zci=0; zci<Ntot; zci++) {
        printf("%lf= ",grid[zci]);
        for (xci=0; xci<(n0); xci++) {
            printf("%lf, ",shpvl[zci][xci]);
        }
        printf("\n");
    }
#endif

    /* now calculate the mode vectors */
    /* each vector is Nx x Ny length and there are n0*n0 of them */
    if ((*Av=(double*)calloc((size_t)(N*(n0)*(n0)),sizeof(double)))==0) {
        fprintf(stderr,"%s: %d: no free memory\n",__FILE__,__LINE__);
        exit(1);
    }
    for (n2=0; n2<(n0); n2++) {
        for (n1=0; n1<(n0); n1++) {
            (*cplx)[n2*n0+n1]=((n1+n2)%2==0?0:1) /* even (real) or odd (imaginary)*/;
            /* sign */
            if ((*cplx)[n2*n0+n1]==0) {
                signval=(((n1+n2)/2)%2==0?1:-1);
            } else {
                signval=(((n1+n2-1)/2)%2==0?1:-1);
            }
            /* fill in N*(zci) to N*(zci+1)-1 */
            start=N*(n2*(n0)+n1);
            if (signval==-1) {
                for (xci=0; xci<N; xci++) {
                    (*Av)[start+xci]=-shpvl[xindex[xci]][n1]*shpvl[yindex[xci]][n2];
                }
            } else {
            for (xci=0; xci<N; xci++) {
                    (*Av)[start+xci]=shpvl[xindex[xci]][n1]*shpvl[yindex[xci]][n2];
                }
            }
        }
    }

#ifdef SBYDEBUG
    printf("Matrix dimension=%d by %d\n",N,(n0)*(n0));
    for (n1=0; n1<(n0); n1++) {
        for (n2=0; n2<(n0); n2++) {
            /* fill in N*N*(zci) to N*N*(zci+1)-1 */
            start=N*(n1*(n0)+n2);
            for (xci=0; xci<N; xci++) {
                printf("%lf ",(*Av)[start+xci]);
            }
            printf("\n");
        }
    }
#endif

    free(grid);
    free(xindex);
    free(yindex);
    free(cx_val);
    free(cy_val);
    free(neg_grid);
    free(fact);
    for (xci=0; xci<Ntot; xci++) {
        free(shpvl[xci]);
    }
    free(shpvl);
    return 0;
}
/****************** end of shapelet related code block ********************/


ShapeletCoherence::ShapeletCoherence(const Expr<Vector<4> >::ConstPtr stokes,
    double scaleI, const casa::Array<double> &coeffI,
    const Expr<Vector<3> >::ConstPtr &uvwA,
    const Expr<Vector<3> >::ConstPtr &uvwB)
    :   BasicTernaryExpr<Vector<4>, Vector<3>, Vector<3>, JonesMatrix>(stokes,
            uvwA, uvwB),
        itsShapeletScaleI_(scaleI),
        itsShapeletCoeffI_(coeffI)
{
}

const JonesMatrix::View ShapeletCoherence::evaluateImpl(const Grid &grid,
    const Vector<4>::View &stokes, const Vector<3>::View &uvwA,
    const Vector<3>::View &uvwB) const
{
    return evaluateImplI(grid, stokes, itsShapeletScaleI_, itsShapeletCoeffI_,
        uvwA, uvwB);
}

const JonesMatrix::View ShapeletCoherence::evaluateImplI(const Grid &grid,
    const Vector<4>::View &stokes, double itsShapeletScaleI,
    const casa::Array<double> &itsShapeletCoeffI, const Vector<3>::View &uvwA,
    const Vector<3>::View &uvwB) const
{
    JonesMatrix::View result;

    // Compute baseline uv-coordinates in meters (1D in time).
    Matrix u = uvwA(0) - uvwB(0); //flip u sign
    Matrix v = uvwB(1) - uvwA(1);

    // Compute spatial coherence (2D).
    const unsigned int nFreq = grid[FREQ]->size();
    const unsigned int nTime = grid[TIME]->size();

    Matrix coherence;
    coherence.setDCMat(nFreq,nTime);
    double *realp,*imagp;
    coherence.dcomplexStorage(realp, imagp);
    //LOG_DEBUG_STR("Shapelet Domain: " << nFreq <<" "<<nTime);
    //create two arrays for scaled u, v coords
    double *ul,*vl;
    ul=new double[nTime];
    vl=new double[nTime];

    casa::IPosition cshape=itsShapeletCoeffI.shape();
    ASSERT(cshape(0)==cshape(1));

    double *Av=0; int *cplx=0;
    double *repsum=new double[nTime]; //array for partial sum
    double *impsum=new double[nTime]; //array for partial sum

    for(unsigned int ch = 0; ch < nFreq; ++ch)
    {
        const double lambda_inv = grid[FREQ]->center(ch) / casa::C::c;
        for(unsigned int ts = 0; ts < nTime; ++ts)
        {
            ul[ts]=u.getDouble(0,ts)*lambda_inv;
            vl[ts]=v.getDouble(0,ts)*lambda_inv;
        }
        //calculate mode vectors for this freq
        calculate_uv_mode_vectors_bi(ul, vl, nTime, itsShapeletScaleI,
            (int)cshape(0), &Av, &cplx);
        //calculate the partial sum, multiplied by coefficients
        memset(repsum,0,sizeof(double)*nTime);
        memset(impsum,0,sizeof(double)*nTime);
        //LOG_DEBUG_STR("Modes "<<cshape(0)<<" "<<cshape(1));
        for (unsigned int nx=0; nx<cshape(0); ++nx) {
            for (unsigned int ny=0; ny<cshape(0); ++ny) {
                //y=a*x+y
                if (cplx[ny*cshape(0)+nx]) {
                    for(unsigned int ts=0; ts<nTime; ++ts) {
                        impsum[ts]+=Av[(ny*cshape(0)+nx)*nTime+ts]*itsShapeletCoeffI(casa::IPosition(2,nx,ny));
                    }
                } else {
                    for(unsigned int ts=0; ts<nTime; ++ts) {
                        repsum[ts]+=Av[(ny*cshape(0)+nx)*nTime+ts]*itsShapeletCoeffI(casa::IPosition(2,nx,ny));
                    }
                }
            }
        }
        //assign the result to right values
        for(unsigned int ts=0; ts<nTime; ++ts) {
            (realp[nFreq*ts+ch])=repsum[ts];
            (imagp[nFreq*ts+ch])=impsum[ts];
        }
        free(Av); free(cplx);
    }
    delete ul;
    delete vl;
    delete repsum;
    delete impsum;

    //LOG_DEBUG_STR("Shapelet Stokes: " << stokes(0)<<" "<<stokes(1) <<" "<< stokes(2)<<" "<<stokes(3));

    if(stokes.bound(0) || stokes.bound(1))
    {
        result.assign(0, 0, coherence * (stokes(0) + stokes(1)));
        result.assign(1, 1, coherence * (stokes(0) - stokes(1)));
    }

    if(stokes.bound(2) || stokes.bound(3))
    {
        Matrix uv = coherence * tocomplex(stokes(2), stokes(3));
        result.assign(0, 1, uv);
        result.assign(1, 0, conj(uv));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
