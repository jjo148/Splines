/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  Copyright (C) 2016                                                      |
 |                                                                          |
 |         , __                 , __                                        |
 |        /|/  \               /|/  \                                       |
 |         | __/ _   ,_         | __/ _   ,_                                | 
 |         |   \|/  /  |  |   | |   \|/  /  |  |   |                        |
 |         |(__/|__/   |_/ \_/|/|(__/|__/   |_/ \_/|/                       |
 |                           /|                   /|                        |
 |                           \|                   \|                        |
 |                                                                          |
 |      Enrico Bertolazzi                                                   |
 |      Dipartimento di Ingegneria Industriale                              |
 |      Universita` degli Studi di Trento                                   |
 |      email: enrico.bertolazzi@unitn.it                                   |
 |                                                                          |
\*--------------------------------------------------------------------------*/

#include "Splines.hh"
#include <iomanip>

/**
 * 
 */

namespace Splines {

  using namespace std ; // load standard namespace

  static
  void
  knots_sequence( sizeType        n,
                  valueType const X[],
                  sizeType        order,
                  valueType     * knots ) {
                  /*
   knots[0] = 595 ;
   knots[1] = 595 ;
   knots[2] = 595 ;
   knots[3] = 595 ;
   knots[4] = 700 ;
   knots[5] = 738.759 ;
   knots[6] = 840.556 ;
   knots[7] = 872.888 ;
   knots[8] = 897.222 ;
   knots[9] = 921.032 ;
   knots[10] = 960.426 ;
   knots[11] = 1024.64 ;
   knots[12] = 1075 ;
   knots[13] = 1075 ;
   knots[14] = 1075 ;
   knots[15] = 1075 ;
   return ;
  /*
   knots[0] = 0.595000000000000e3 ;
   knots[1] = 0.595000000000000e3 ;
   knots[2] = 0.595000000000000e3 ;
   knots[3] = 0.595000000000000e3 ;
   knots[4] = 0.704596277936432e3 ;
   knots[5] = 0.772030235868696e3 ;
   knots[6] = 0.828886094973769e3 ;
   knots[7] = 0.868451908082036e3 ;
   knots[8] = 0.894798665375998e3 ;
   knots[9] = 0.919446984213767e3 ;
   knots[10] = 0.951204525959706e3 ;
   knots[11] = 0.992334500376403e3 ;
   knots[12] = 1.075000000000000e3 ;
   knots[13] = 1.075000000000000e3 ;
   knots[14] = 1.075000000000000e3 ;
   knots[15] = 1.075000000000000e3 ;
   return ;
*/

    std::fill( knots, knots+order, X[0] ) ;
    knots += order ;
    if ( n > order ) {
      /*
      #ifdef SPLINE_USE_ALLOCA
      valueType * i_nodes = (valueType*)alloca( n*sizeof(valueType) ) ;
      valueType * Xp      = (valueType*)alloca( n*sizeof(valueType) ) ;
      #else
      valueType i_nodes[n] ;
      valueType Xp[n] ;
      #endif
      for ( sizeType i = 0 ; i < n ; ++i ) i_nodes[i] = i ;
      pchip( i_nodes, X, Xp, n ) ;
      for ( sizeType j = 0 ; j < n-order ; ++j ) {
        valueType ss = valueType((n-1)*(j+1))/valueType(n-order+1) ;
        sizeType i_segment = sizeType(ss) ;
        valueType base[4] ;
        Hermite3( ss-i_segment, 1.0, base ) ;
        knots[j] = base[0] * X[i_segment] +
                   base[1] * X[i_segment+1] +
                   base[2] * Xp[i_segment]  +
                   base[3] * Xp[i_segment+1] ;
      }
      //*/
      ///*
      for ( sizeType j = 0 ; j < n-order ; ++j ) {
        valueType acc = 0 ;
        for ( sizeType k = 1 ; k < order ; ++k ) acc += X[j+k] ;
        knots[j] = acc / (order-1) ;
      }
      //*/
      knots += n-order ;
    }
    std::fill( knots, knots+order, X[n-1] ) ;
  }

  /*\
	  uses de Boor algorithm to compute one
	  coordinate on B-spline curve for param. value u in interval i.
    input:  degree:	polynomial degree of each piece of curve
    coeff:	B-spline control points
	knot:	knot sequence
	u:	evaluation abscissa
	i:	u's interval: u[i]<= u < u[i+1]
output:	coordinate value. 
  \*/

  template <int degree>
  class BSplineEval {
  public:

    static
    void
    eval_levels( valueType x, valueType const knot[], valueType y[] ) {
      sizeType j = degree ;
      do {
        valueType omega = (x-knot[j])/(knot[j+degree]-knot[j]) ;
        y[j] = (1-omega)*y[j-1]+omega*y[j] ;
      } while ( --j > 0 ) ;
      BSplineEval<degree-1>::eval_levels( x, knot+1, y+1 ) ;
    }

    static
    valueType
    eval( valueType x, valueType const knot[], valueType const y[] ) {
      valueType c[degree+1] ;
      sizeType j = degree ;
      do {
        valueType omega = (x-knot[j])/(knot[j+degree]-knot[j]) ;
        c[j] = (1-omega)*y[j-1]+omega*y[j] ;
      } while ( --j > 0 ) ;
      BSplineEval<degree-1>::eval_levels( x, knot+1, c+1 ) ;
      return c[degree] ;
    }

    static
    valueType
    eval_D( valueType x, valueType const knot[], valueType const y[] ) {
      valueType d[degree] ; // poligono derivata
      for ( sizeType j = 0 ; j < degree ; ++j )
        d[j] = degree*(y[j+1]-y[j])/(knot[j+degree+1] - knot[j+1]) ;
      return BSplineEval<degree-1>::eval( x, knot+1, d ) ;
    }

    static
    valueType
    eval_DD( valueType x, valueType const knot[], valueType const y[] ) {
      valueType d[degree] ; // poligono derivata
      for ( sizeType j = 0 ; j < degree ; ++j )
        d[j] = degree*(y[j+1]-y[j])/(knot[j+degree+1] - knot[j+1]) ;
      return BSplineEval<degree-1>::eval_D( x, knot+1, d ) ;
    }

    static
    valueType
    eval_DDD( valueType x, valueType const knot[], valueType const y[] ) {
      valueType d[degree] ; // poligono derivata
      for ( sizeType j = 0 ; j < degree ; ++j )
        d[j] = degree*(y[j+1]-y[j])/(knot[j+degree+1] - knot[j+1]) ;
      return BSplineEval<degree-1>::eval_DD( x, knot+1, d ) ;
    }

    // calcola il valore delle basi non zero quando
    // knot[degree] <= x <= knot[degree+1]
    static
    void
    eval_B( valueType x, valueType const knot[], valueType Bbase[] ) {
      valueType B[2*degree+2] ;
      std::fill( B, B+2*degree+2, 0.0 ) ;
      B[degree] = 1 ;
      for ( sizeType r = 1 ; r <= degree; ++r ) {
        for ( sizeType j = 0 ; j <= 2*degree-r ; ++j ) {
          valueType oma = 0 ;
          valueType omb = 0 ;
          if ( knot[j+r]   > knot[j]   ) oma = (x-knot[j])/(knot[j+r]-knot[j]) ;
          if ( knot[j+r+1] > knot[j+1] ) omb = (x-knot[j+1])/(knot[j+r+1]-knot[j+1]) ;
          B[j] = oma*B[j]+(1-omb)*B[j+1] ;
        }
      }
      std::copy( B, B+degree+1, Bbase ) ;
    }

  } ;

  template <>
  class BSplineEval<1> {
  public:

    static
    void
    eval_levels( valueType x, valueType const knot[], valueType y[] ) {
      valueType omega = (x-knot[1])/(knot[2]-knot[1]) ;
      y[1] = (1-omega)*y[0]+omega*y[1] ;
    }

    static
    valueType
    eval( valueType x, valueType const knot[], valueType const y[] ) {
      valueType omega = (x-knot[1])/(knot[2]-knot[1]) ;
      return (1-omega)*y[0]+omega*y[1] ;
    }

    static
    valueType
    eval_D( valueType, valueType const knot[], valueType const y[] ) {
      return (y[1]-y[0])/(knot[2] - knot[1]) ;
    }

    static
    valueType
    eval_DD( valueType, valueType const [], valueType const [] ) {
      return 0 ;
    }

    static
    valueType
    eval_DDD( valueType, valueType const [], valueType const [] ) {
      return 0 ;
    }

    static
    void
    eval_B( valueType, valueType const [], valueType Bbase[] ) {
      Bbase[0] = 1 ;
    }

  } ;
  
  /*
  // Solve banded linear system
  */
  static
  void
  solveBanded( valueType * rows,
               valueType * rhs,
               sizeType    n,
               sizeType    ndiag ) {
    // forward
    sizeType rsize = 1+2*ndiag ;
    sizeType i = 0 ;
    valueType * rowsi = rows + ndiag ;
    do {
      // pivot
      valueType pivot = rowsi[0] ; // elemento sulla diagonale
      
      // scala equazione
      for ( sizeType j = 0 ; j <= ndiag ; ++j ) rowsi[j] /= pivot ;
      rhs[i] /= pivot ;

      // azzera colonna
      sizeType nr = i+ndiag >= n ? n-i-1 : ndiag ;
      for ( sizeType k = 1 ; k <= nr ; ++k ) {
        valueType * rowsk = rowsi + k * (rsize-1) ;
        valueType tmp = rowsk[0] ;
        rowsk[0] = 0 ;
        for ( sizeType j = 1 ; j <= nr ; ++j ) rowsk[j] -= tmp*rowsi[j] ;
        rhs[i+k] -= tmp*rhs[i] ;
      }
      rowsi += rsize ;
      /*
      cout << "\n\n" ;
      for ( int ii = 0 ; ii < n ; ++ii ) {
        valueType * rowsi = rows + ii * rsize ;
        for ( int jj = 0 ; jj < rsize ; ++jj ) {
          cout << rowsi[jj] << ' ' ;
        }
        cout << "\n" ;
      }
      cout << "\n\n" ;
      */
    } while ( ++i < n ) ;
    // backward
    while ( i > 0 ) {
      --i ;
      rowsi -= rsize ;
      sizeType nr = i+ndiag >= n ? n-i-1 : ndiag ;
      for ( sizeType j = 1 ; j <= nr ; ++j ) rhs[i] -= rhs[i+j]*rowsi[j] ;
    }
  }

  // build spline without computation
  #ifdef SPLINES_USE_GENERIC_CONTAINER

  using GenericContainerNamespace::GC_VEC_REAL ;
  using GenericContainerNamespace::GC_VEC_INTEGER ;
  using GenericContainerNamespace::vec_int_type ;
  using GenericContainerNamespace::vec_real_type ;

  template <int _degree>
  void
  BSpline<_degree>::setup( GenericContainer const & gc ) {
    /*
    // gc["x"]
    // gc["y"]
    //
    */
    SPLINE_ASSERT( gc.exists("x"), "[" << type_name() << "[" << _name << "]::setup] missing `x` field!") ;
    SPLINE_ASSERT( gc.exists("y"), "[" << type_name() << "[" << _name << "]::setup] missing `y` field!") ;

    GenericContainer const & gc_x = gc("x") ;
    GenericContainer const & gc_y = gc("y") ;

    SPLINE_ASSERT( GC_VEC_REAL == gc_x.get_type() || GC_VEC_INTEGER == gc_x.get_type(),
                   "[" << type_name() << "[" << _name << "]::setup] field `x` expected to be of type `vec_real_type` found: `" <<
                   gc_x.get_type_name() << "`" ) ;

    SPLINE_ASSERT( GC_VEC_REAL == gc_y.get_type() || GC_VEC_INTEGER == gc_y.get_type(),
                   "[" << type_name() << "[" << _name << "]::setup] field `y` expected to be of type `vec_real_type` found: `" <<
                   gc_y.get_type_name() << "`" ) ;

    vec_real_type x, y ;
    vec_real_type const * px = nullptr ;
    vec_real_type const * py = nullptr ;
    if ( GC_VEC_INTEGER == gc_x.get_type() ) {
      vec_int_type const & vx = gc_x.get_vec_int() ;
      x.resize(vx.size()) ; std::copy( vx.begin(), vx.end(), x.begin() ) ;
    } else {
      px = &gc_x.get_vec_real() ;
    }
    if ( GC_VEC_INTEGER == gc_y.get_type() ) {
      vec_int_type const & vy = gc_y.get_vec_int() ;
      y.resize(vy.size()) ; std::copy( vy.begin(), vy.end(), y.begin() ) ;
    } else {
      py = &gc_y.get_vec_real() ;
    }

    build( *px, *py ) ;
  }
  #endif

  template <int _degree>
  void
  BSpline<_degree>::build(void) {
    #ifdef SPLINE_USE_ALLOCA
    valueType * band = (valueType*)alloca( npts*(2*_degree+1)*sizeof(valueType) ) ;
    #else
    valueType band[npts*(2*_degree+1)] ;
    #endif
    
    std::fill( band, band+npts*(2*_degree+1), 0 ) ;
    knots_sequence( npts, X, _degree+1, knots ) ;
    
    // costruzione sistema lineare

    // calcola il valore delle basi non zero quando
    // knot[degree] <= x <= knot[degree+1]
    sizeType nr = 2*_degree+1 ;
    for ( sizeType i = 0 ; i < npts ; ++i ) {
      valueType * rowi = band + nr * i ;
      sizeType ii = knot_search( X[i] ) ;
      BSplineEval<_degree>::eval_B( X[i], knots+ii, (rowi + _degree + ii) - i ) ;
      yPolygon[i] = Y[i] ;
      /*
      for ( sizeType ii = 0 ; ii < nr ; ++ii )
        cout << rowi[ii] << " " ;
      cout << '\n' ;
      */
    }
    solveBanded( band, yPolygon, npts, _degree ) ;
    // extrapolation
    valueType const * knots_R    = knots    + npts - _degree - 1 ;
    valueType const * yPolygon_R = yPolygon + npts - _degree - 1 ;
    valueType x_L = X[0] ;
    valueType x_R = X[npts-1] ;
    s_L   = BSplineEval<_degree>::eval(x_L,knots,yPolygon) ;
    s_R   = BSplineEval<_degree>::eval(x_R,knots_R,yPolygon_R) ;
    ds_L  = BSplineEval<_degree>::eval_D(x_L,knots,yPolygon) ;
    ds_R  = BSplineEval<_degree>::eval_D(x_R,knots_R,yPolygon_R) ;
    dds_L = BSplineEval<_degree>::eval_DD(x_L,knots,yPolygon) ;
    dds_R = BSplineEval<_degree>::eval_DD(x_R,knots_R,yPolygon_R) ;
  }

  // build spline using virtual constructor

  template <int _degree>
  void
  BSpline<_degree>::build ( valueType const x[], sizeType incx,
                            valueType const y[], sizeType incy,
                            sizeType n ) {
    reserve( n ) ;
    for ( sizeType i = 0 ; i < n ; ++i ) {
      X[i] = x[i*incx] ;
      Y[i] = y[i*incy] ;
    }
    knots_sequence( n, X, _degree+1, knots ) ;
    npts = n ;
    build() ;
  }

  template <int _degree>
  void
  BSpline<_degree>::build ( valueType const x[],
                            valueType const y[],
                            sizeType n ) {
    reserve( n ) ;
    std::copy( x, x+n, X );
    std::copy( y, y+n, Y );
    knots_sequence( n, X, _degree+1, knots ) ;
    npts = n ;
    build() ;
  }
  
  template <int _degree>
  void
  BSpline<_degree>::clear(void) {
    if ( !_external_alloc ) baseValue.free() ;
    npts = npts_reserved = 0 ;
    _external_alloc = false ;
    X = Y = knots = yPolygon = nullptr ;
  }

  template <int _degree>
  void
  BSpline<_degree>::reserve( sizeType n ) {
    if ( _external_alloc && n <= npts_reserved ) {
      // nothing to do!, already allocated
    } else {
      npts_reserved = n ;
      baseValue.allocate(4*n+_degree+1) ;
      X        = baseValue(n) ;
      Y        = baseValue(n) ;
      knots    = baseValue(n+_degree+1) ;
      yPolygon = baseValue(n) ;
      _external_alloc = false ;
    }
    npts         = 0 ;
    lastInterval = _degree ;
  }

  template <int _degree>
  void
  BSpline<_degree>::reserve_external( sizeType     n,
                                      valueType *& p_x,
                                      valueType *& p_y,
                                      valueType *& p_knots,
                                      valueType *& p_yPolygon ) {
    npts_reserved   = n ;
    X               = p_x ;
    Y               = p_y ;
    knots           = p_knots ;
    yPolygon        = p_yPolygon ;
    npts            = 0 ;
    lastInterval    = _degree ;
    _external_alloc = true ;
  }

  template <int _degree>
  sizeType
  BSpline<_degree>::knot_search( valueType x ) const {
    SPLINE_ASSERT( npts > 0, "\nknot_search(" << x << ") empty spline");
    if ( x < knots[lastInterval] || knots[lastInterval+1] < x ) {
      if ( _check_range ) {
        SPLINE_ASSERT( x >= X[0] && x <= X[npts-1],
                       "method search( " << x << " ) out of range: [" <<
                       X[0] << ", " << X[npts-1] << "]" ) ;
      }
      lastInterval = sizeType(lower_bound( knots, knots+npts+_degree+1, x ) - knots) ;
      if ( lastInterval < _degree ) lastInterval = _degree ;
      else                          --lastInterval ;
    }
    return lastInterval-_degree ;
  }

  template <int _degree>
  valueType
  BSpline<_degree>::operator () ( valueType x ) const {
    if ( x >= X[npts-1] ) {
      valueType dx = x - X[npts-1] ;
      return s_R + dx * ( ds_R + 0.5 * dds_R * dx ) ;
    } else if ( x <= X[0] ) {
      valueType dx = x - X[0] ;
      return s_L + dx * ( ds_L + 0.5 * dds_L * dx ) ;
    } else {
      sizeType i = knot_search( x ) ;
      return BSplineEval<_degree>::eval(x,knots+i,yPolygon+i) ;
    }
  }

  template <int _degree>
  valueType
  BSpline<_degree>::D( valueType x ) const {
    if ( x >= X[npts-1] ) {
      valueType dx = x - X[npts-1] ;
      return ds_R + dds_R * dx ;
    } else if ( x <= X[0] ) {
      valueType dx = x - X[0] ;
      return ds_L + dds_L * dx ;
    } else {
      sizeType i = knot_search( x ) ;
      return BSplineEval<_degree>::eval_D(x,knots+i,yPolygon+i) ;
    }
  }

  template <int _degree>
  valueType
  BSpline<_degree>::DD( valueType x ) const {
    if ( x >= X[npts-1] ) {
      return dds_R ;
    } else if ( x <= X[0] ) {
      return dds_L ;
    } else {
      sizeType i = knot_search( x ) ;
      return BSplineEval<_degree>::eval_DD(x,knots+i,yPolygon+i) ;
    }
  }

  template <int _degree>
  valueType
  BSpline<_degree>::DDD( valueType x ) const {
    if ( x >= X[npts-1] ) {
      return 0 ;
    } else if ( x <= X[0] ) {
      return 0 ;
    } else {
      sizeType i = knot_search( x ) ;
      return BSplineEval<_degree>::eval_DDD(x,knots+i,yPolygon+i) ;
    }
  }

  template <int _degree>
  sizeType // order
  BSpline<_degree>::coeffs( valueType cfs[], valueType nodes[], bool transpose ) const {
#if 0
    sizeType n = npts > 0 ? npts-1 : 0 ;
    for ( sizeType i = 0 ; i < n ; ++i ) {
      nodes[i] = X[i] ;
      valueType H  = X[i+1]-X[i] ;
      valueType DY = (Y[i+1]-Y[i])/H ;
      valueType a = Y[i] ;
      valueType b = Yp[i] ;
      valueType c = (3*DY-2*Yp[i]-Yp[i+1])/H;
      valueType d = (Yp[i+1]+Yp[i]-2*DY)/(H*H) ;
      if ( transpose ) {
        cfs[4*i+3] = a ;
        cfs[4*i+2] = b ;
        cfs[4*i+1] = c ;
        cfs[4*i+0] = d ;
      } else {
        cfs[i+3*n] = a ;
        cfs[i+2*n] = b ;
        cfs[i+1*n] = c ;
        cfs[i+0*n] = d ;
      }
    }
#endif
    return _degree+1 ;
  }

  // Implementation
  template <int _degree>
  void
  BSpline<_degree>::copySpline( BSpline const & S ) {
    BSpline::reserve(S.npts) ;
    npts = S.npts ;
    std::copy( S.X,        S.X+npts,               X ) ;
    std::copy( S.Y,        S.Y+npts,               Y ) ;
    std::copy( S.knots,    S.knots+npts+_degree+1, knots ) ;
    std::copy( S.yPolygon, S.yPolygon+npts,        yPolygon ) ;
  }

  //! change X-range of the spline
  template <int _degree>
  void
  BSpline<_degree>::setRange( valueType xmin, valueType xmax ) {
    Spline::setRange( xmin, xmax ) ;
    valueType recS = ( X[npts-1] - X[0] ) / (xmax - xmin) ;
    valueType * iy = Y ;
    while ( iy < Y + npts ) *iy++ *= recS ;
  }

  template <int _degree>
  void
  BSpline<_degree>::writeToStream( std::basic_ostream<char> & s ) const {
    sizeType nseg = npts > 0 ? npts - 1 : 0 ;
    for ( sizeType i = 0 ; i < nseg ; ++i )
      s << "segment N." << setw(4) << i
        << " X:[" << X[i] << ", " << X[i+1]
        << "] Y:[" << Y[i] << ", " << Y[i+1]
        << "] slope: " << (Y[i+1]-Y[i])/(X[i+1]-X[i])
        << '\n' ; 
  }

  template <int _degree>
  void
  BSpline<_degree>::bases( valueType x, valueType vals[] ) const {
    std::fill( vals, vals+npts, 0 ) ;
    sizeType i = knot_search( x ) ;
    BSplineEval<_degree>::eval_B( x, knots+i, vals+i ) ;
  }

  template <int _degree>
  sizeType
  BSpline<_degree>::bases_nz( valueType x, valueType vals[] ) const {
    sizeType i = knot_search( x ) ;
    BSplineEval<_degree>::eval_B( x, knots+i, vals ) ;
    return i ;
  }

  template class BSpline<1> ;
  template class BSpline<2> ;
  template class BSpline<3> ;
  template class BSpline<4> ;
  template class BSpline<5> ;
  template class BSpline<6> ;
  template class BSpline<7> ;
  template class BSpline<8> ;
  template class BSpline<9> ;
  template class BSpline<10> ;

}
