      subroutine bsplvd ( t, lent, k, x, left, a, dbiatx, nderiv )
c     --------   ------
c      implicit none

C calculates value and deriv.s of all b-splines which do not vanish at x
C calls bsplvb
c
c******  i n p u t  ******
c  t     the knot array, of length left+k (at least)
c  k     the order of the b-splines to be evaluated
c  x     the point at which these values are sought
c  left  an integer indicating the left endpoint of the interval of
c        interest. the  k  b-splines whose support contains the interval
c               (t(left), t(left+1))
c        are to be considered.
c  a s s u m p t i o n  - - -  it is assumed that
c               t(left) .lt. t(left+1)
c        division by zero will result otherwise (in  b s p l v b ).
c        also, the output is as advertised only if
c               t(left) .le. x .le. t(left+1) .
c  nderiv   an integer indicating that values of b-splines and their
c        derivatives up to but not including the  nderiv-th  are asked
c        for. ( nderiv  is replaced internally by the integer in (1,k)
c        closest to it.)
c
c******  w o r k   a r e a  ******
c  a     an array of order (k,k), to contain b-coeff.s of the derivat-
c        ives of a certain order of the  k  b-splines of interest.
c
c******  o u t p u t  ******
c  dbiatx   an array of order (k,nderiv). its entry  (i,m)  contains
c        value of  (m-1)st  derivative of  (left-k+i)-th  b-spline of
c        order  k  for knot sequence  t , i=m,...,k; m=1,...,nderiv.
c
c******  m e t h o d  ******
c  values at  x  of all the relevant b-splines of order k,k-1,...,
c  k+1-nderiv  are generated via  bsplvb  and stored temporarily
c  in  dbiatx .  then, the b-coeffs of the required derivatives of the
c  b-splines of interest are generated by differencing, each from the
c  preceding one of lower order, and combined with the values of b-
c  splines of corresponding order in  dbiatx  to produce the desired
c  values.

C Args
      integer lent,k,left,nderiv
      double precision t(lent),x, dbiatx(k,nderiv), a(k,k)
C Locals
      double precision factor,fkp1mm,sum
      integer i,ideriv,il,j,jlow,jp1mid, kp1,kp1mm,ldummy,m,mhigh

      mhigh = max0(min0(nderiv,k),1)
c     mhigh is usually equal to nderiv.
      kp1 = k+1
      call bsplvb(t,lent,kp1-mhigh,1,x,left,dbiatx)
      if (mhigh .eq. 1)                 go to 99
c     the first column of  dbiatx  always contains the b-spline values
c     for the current order. these are stored in column k+1-current
c     order  before  bsplvb  is called to put values for the next
c     higher order on top of it.
      ideriv = mhigh
      do 15 m=2,mhigh
         jp1mid = 1
         do 11 j=ideriv,k
            dbiatx(j,ideriv) = dbiatx(jp1mid,1)
   11       jp1mid = jp1mid + 1
         ideriv = ideriv - 1
         call bsplvb(t,lent,kp1-ideriv,2,x,left,dbiatx)
   15    continue
c
c     at this point,  b(left-k+i, k+1-j)(x) is in  dbiatx(i,j) for
c     i=j,...,k and j=1,...,mhigh ('=' nderiv). in particular, the
c     first column of  dbiatx  is already in final form. to obtain cor-
c     responding derivatives of b-splines in subsequent columns, gene-
c     rate their b-repr. by differencing, then evaluate at  x.
c
      jlow = 1
      do 20 i=1,k
         do 19 j=jlow,k
   19       a(j,i) = 0e0
         jlow = i
   20    a(i,i) = 1e0
c     at this point, a(.,j) contains the b-coeffs for the j-th of the
c     k  b-splines of interest here.
c
      do 40 m=2,mhigh
         kp1mm = kp1 - m
         fkp1mm = dble(kp1mm)
         il = left
         i = k
c
c        for j=1,...,k, construct b-coeffs of  (m-1)st  derivative of
c        b-splines from those for preceding derivative by differencing
c        and store again in  a(.,j) . the fact that  a(i,j) = 0  for
c        i .lt. j  is used.sed.
         do 25 ldummy=1,kp1mm
            factor = fkp1mm/(t(il+kp1mm) - t(il))
c           the assumption that t(left).lt.t(left+1) makes denominator
c           in  factor  nonzero.
            do 24 j=1,i
   24          a(i,j) = (a(i,j) - a(i-1,j))*factor
            il = il - 1
   25       i = i - 1
c
c        for i=1,...,k, combine b-coeffs a(.,i) with b-spline values
c        stored in dbiatx(.,m) to get value of  (m-1)st  derivative of
c        i-th b-spline (of interest here) at  x , and store in
c        dbiatx(i,m). storage of this value over the value of a b-spline
c        of order m there is safe since the remaining b-spline derivat-
c        ive of the same order do not use this value due to the fact
c        that  a(j,i) = 0  for j .lt. i .
   30    do 40 i=1,k
            sum = 0.
            jlow = max0(i,m)
            do 35 j=jlow,k
   35          sum = a(j,i)*dbiatx(j,m) + sum
   40       dbiatx(i,m) = sum
   99 return
      end

      subroutine bsplvb ( t, lent,jhigh, index, x, left, biatx )
c      implicit none
c     -------------

calculates the value of all possibly nonzero b-splines at  x  of order
c
c               jout  =  dmax( jhigh , (j+1)*(index-1) )
c
c  with knot sequence  t .
c
c******  i n p u t  ******
c  t.....knot sequence, of length  left + jout  , assumed to be nonde-
c        creasing.  a s s u m p t i o n . . . .
c                       t(left)  .lt.  t(left + 1)   .
c   d i v i s i o n  b y  z e r o  will result if  t(left) = t(left+1)
c  jhigh,
c  index.....integers which determine the order  jout = max(jhigh,
c        (j+1)*(index-1))  of the b-splines whose values at  x  are to
c        be returned.  index  is used to avoid recalculations when seve-
c        ral columns of the triangular array of b-spline values are nee-
c        ded (e.g., in  bvalue  or in  bsplvd ). precisely,
c                     if  index = 1 ,
c        the calculation starts from scratch and the entire triangular
c        array of b-spline values of orders 1,2,...,jhigh  is generated
c        order by order , i.e., column by column .
c                     if  index = 2 ,
c        only the b-spline values of order  j+1, j+2, ..., jout  are ge-
c        nerated, the assumption being that  biatx , j , deltal , deltar
c        are, on entry, as they were on exit at the previous call.
c           in particular, if  jhigh = 0, then  jout = j+1, i.e., just
c        the next column of b-spline values is generated.
c
c  w a r n i n g . . .  the restriction   jout .le. jmax (= 20)  is im-
c        posed arbitrarily by the dimension statement for  deltal  and
c        deltar  below, but is  n o w h e r e  c h e c k e d  for .
c
c  x.....the point at which the b-splines are to be evaluated.
c  left.....an integer chosen (usually) so that
c                  t(left) .le. x .le. t(left+1)  .
c
c******  o u t p u t  ******
c  biatx.....array of length  jout , with  biatx(i)  containing the val-
c        ue at  x  of the polynomial of order  jout  which agrees with
c        the b-spline  b(left-jout+i,jout,t)  on the interval (t(left),
c        t(left+1)) .
c
c******  m e t h o d  ******
c  the recurrence relation
c
c                       x - t(i)              t(i+j+1) - x
c     b(i,j+1)(x)  =  -----------b(i,j)(x) + ---------------b(i+1,j)(x)
c                     t(i+j)-t(i)            t(i+j+1)-t(i+1)
c
c  is used (repeatedly) to generate the (j+1)-vector  b(left-j,j+1)(x),
c  ...,b(left,j+1)(x)  from the j-vector  b(left-j+1,j)(x),...,
c  b(left,j)(x), storing the new values in  biatx  over the old. the
c  facts that
c            b(i,1) = 1  if  t(i) .le. x .lt. t(i+1)
c  and that
c            b(i,j)(x) = 0  unless  t(i) .le. x .lt. t(i+j)
c  are used. the particular organization of the calculations follows al-
c  gorithm  (8)  in chapter x of the text.
c

C Arguments
      integer lent, jhigh, index, left
      double precision t(lent),x, biatx(jhigh)
c     dimension     t(left+jout), biatx(jout)
c     -----------------------------------
c current fortran standard makes it impossible to specify the length of
c  t  and of  biatx  precisely without the introduction of otherwise
c  superfluous additional arguments.

C Local Variables
      integer jmax
      parameter(jmax = 20)
      integer i,j,jp1
      double precision deltal(jmax), deltar(jmax),saved,term

      save j,deltal,deltar
      data j/1/
c
                                        go to (10,20), index
   10 j = 1
      biatx(1) = 1e0
      if (j .ge. jhigh)                 go to 99
c
   20    jp1 = j + 1
         deltar(j) = t(left+j) - x
         deltal(j) = x - t(left+1-j)
         saved = 0e0
         do 26 i=1,j
            term = biatx(i)/(deltar(i) + deltal(jp1-i))
            biatx(i) = saved + deltar(i)*term
   26       saved = deltal(jp1-i)*term
         biatx(jp1) = saved
         j = jp1
         if (j .lt. jhigh)              go to 20
c
   99                                   return
      end

