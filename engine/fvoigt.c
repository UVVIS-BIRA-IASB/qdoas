
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  VOIGT PROFILE FUNCTION
//  Name of module    :  FVOIGT.C
//  Author            :  Daniel HURTMANS, Université Libre de Bruxelles
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  The Voigt profile function is the line shape produced by the convolution of
//  a Gaussian and a Lorentzian functions.  This function is used for fitting
//  the resolution of an instrument from spectra.  By fitting a different
//  L/G ratio on left and right sides of the line shape, WinDOAS can fit
//  the asymetry of the line shape.
//
//  Functions of this module calculate Voigt profile function according to
//  Kuntz's algorithm.  They have been written and kindly provided by Daniel
//  HURTMANS, from Université Libre de Bruxelles (ULB).
//
//  Ref : A new implementation of the Humlicek algorithm for the calculation of
//        the Voigt profile function, M.Kuntz, J. Quant. Spectrosc. Radiat.
//        Transfer, Vol. 57, No 6, pp. 819-824, 1997.
//
//  ----------------------------------------------------------------------------
//
//  Voigtx - calculate the Voigt profile function in the region determined by
//           the values of input x and y;
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <math.h>
#include <stdint.h>

#include "doas.h"

// =========
// FUNCTIONS
// =========

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  fRegionI(x,y)                                                                     *
 *                                                                                              *
 * PURPOSE:   Returns the real part of the Humlicek's approximation to the Voigt profile in the *
 *            I domain                                                                          *
 *                                                                                              *
 ***********************************************************************************************/
double fRegionI(double x, double y)
{
 double t1,t2,t3,t6,t7;

 // Common
 t1 = x*x;
 t3 = y*y;
 // Denominator
 t2 = t1*t1;
 t6 = t3*t3;
 t7 = t2+(-1.0+2.0*t3)*t1+t6+0.25+t3;
 // Numerator
 t6 = 0.564189355*y*(t1+t3+0.5);

 return t6/t7;
}

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  fRegionII(x,y)                                                                    *
 *                                                                                              *
 * PURPOSE:   Returns the real part of the Humlicek's approximation to the Voigt profile in the *
 *            II domain                                                                         *
 *                                                                                              *
 ***********************************************************************************************/
double fRegionII(double x, double y)
{
 double t1,t2,t3,t5,t8,t11,t14,t15,t16;

 // Common
 t1 = x*x;
 t2 = t1*t1;
 t5 = y*y;
 t8 = t5*t5;
 // Numerator
 t16 = y*(t2*(0.5641895836*t1+(-2.538853126+1.692568751*t5))
          +(0.564189584*t5+2.961995314+1.692568751*t8)*t1
          +(1.057855469+4.654564065*t5+t8*(3.10304271+0.5641895836*t5)));
 // Denominator
 t3 = t2*t2;
 t11 = t8*t5;
 t14 = t8*t8;
 t15 = t3+t2*((-6.0+4.0*t5)*t1+(6.0*t8+10.5-6.0*t5))+(6.0*t8-4.5+9.0*t5+4.0*t11)*t1+0.5625
      +10.5*t8+6.0*t11+t14+4.5*t5;

 return t16/t15;
}

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  fRegionIII(x,y)                                                                   *
 *                                                                                              *
 * PURPOSE:   Returns the real part of the Humlicek's approximation to the Voigt profile in the *
 *            III domain                                                                        *
 *                                                                                              *
 ***********************************************************************************************/
double fRegionIII(double x, double y)
{
 double t2,t3,t7,t11,t12,t15,t16,t19,t21,t1,t5,t8,t9,t13,t14,t17,t18,t23;

 // Common
 t1 = x*x;
 t2 = t1*t1;
 t3 = t2*t2;
 t5 = y*y;

 // numerator
 t7 = t5*y;
 t11 = t5*t5;
 t12 = t11*y;
 t15 = t11*t5;
 t16 = t11*t7;
 t19 = t11*t11;
 t21 = (0.000971359+0.564223565*y)*t3
      +t2*(t1*(-0.12891018+2.25689426*t7+7.56185918*t5+1.66203892*y)
          +(22.67974937*t11+18.5457583*y+42.5682261*t5+4.580094577+3.38534139*t12+52.8453856*t7))
      +(336.3639005*t7+22.67780666*t15+100.7046544*t12+2.25689426*t16+247.1978497*t11
        +220.8433585*t5-60.5639454-2.3436382*y)*t1
      +y*(973.7779393+0.564223565*t19)+1174.79956*t11+272.1013718+581.7460702*t12+7.558945099*t19
      +204.5007135*t15+1629.763205*t5+1678.331189*t7+49.52130772*t16;

 // Denominator
 t8 = t5*y;
 t9 = t5*t5;
 t13 = t9*y;
 t14 = t9*t5;
 t17 = t9*t8;
 t18 = t9*t9;
 t23 = t3*(t1+5.0*t5+1.4964597+13.39879602*y)
      +t2*(t1*(53.59518408*t8+55.0293276*y+92.7567868*t5+22.0352268+10.0*t9)+269.2916022*t9
           +497.3013754*t5+308.18521*y+78.865852+80.39277612*t13+479.2576236*t8+10.0*t14)
      +t1*(1758.336424*t5+266.2986828*t14+1549.675109*t9+902.306594*y+793.4272644*t13
           +53.59518408*t17+5.0*t18+211.67807+2037.310058*t8)
      +t18*(t5+88.2674077+13.39879602*y)
      +3447.628806*t9+2256.980703*t13+1074.40896*t14+369.1989684*t17+272.1013718+1280.829942*y
      +2802.870018*t5+3764.966159*t8;

 return t21/t23;

}

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  fRegionIV(x,y)                                                                    *
 *                                                                                              *
 * PURPOSE:   Returns the real part of the Humlicek's approximation to the Voigt profile in the *
 *            IV domain                                                                         *
 *                                                                                              *
 ***********************************************************************************************/
double fRegionIV(double x, double y)
{
 double t1,t2,t3,t4,t5,t6,t12,t13,t14,t15,t16,t17,t18,t19,t21,t23,t24,
        t25,t27,t29,t33,t38,t39,t43,t47,
        t51,t53,t54,t55,t59,t60,t64,t66,
        t7,t8,t9,t31,t52,t56,t57,s1;

 t1 = x*x;
 t2 = t1*t1;
 t3 = t2*t2;
 t4 = t3*t1;
 t5 = t3*t3;
 t8 = y*y;
 t13 = t8*t8;
 t16 = t2*t1;
 t19 = t13*t8;
 t24 = t13*t13;
 t29 = t24*t8;
 t33 = t24*t13;
 t38 = t24*t19;
      t43 = t24*t24;
      t47 = t43*t8;
      t51 = t43*t13;
      t52 = -0.2285628E3*t47-0.8381986E4*t43+41501289.0*t19-0.22404016E7*t29+
0.664313E5*t38+0.2638939912E9-0.383111891E8*t24+0.30356907E6*t33-270166292.0*t8
+99622414.0*t13+0.16135836E3*t51;
      t55 = t43*t19;
      t56 = 0.238180071E8*t29+0.25338331E5*t43-0.10977749E4*t47-0.9762031E2*t51
+0.6515228792E9-294261924.0*t13-0.9807901E5*t38-0.229302391E8*t24-247157022.0*
t8+204466737.0*t19+0.44006822E2*t55-0.57605489E6*t33;
      t59 = t43*t24;
      t60 = -0.230312345E2*t55+0.234416202E6*t38-0.452513015E5*t43-0.35991535E7
*t33+0.772358718E7*t29+0.234143099E3*t51+985385414.0*t8+0.733447046E1*t59
-806985708.0*t13-0.864829016E8*t24+0.226919043E4*t47+560505996.0+0.2918765749E9
*t19;
      t64 = -0.235944344E1*t59+0.1535749125E9*t19+0.729359257E2*t55
-0.5716876099E3*t51+0.5641900381*t43*t29-0.4566620488E9*t13-0.4064922279E5*t43+
0.9866056739E9*t8-0.1684098981E7*t33+0.3207723599E6*t38+0.9694637662E7*t29
-1160275352.0+0.5860680259E4*t47-0.4081683778E8*t24;
      s1 = 0.564190039*y*t5*t4+y*(0.292264335E1+0.73344704E1*t8)*t5*t3+y*(
0.29789634E2*t8+0.770538895E2+0.4400683E2*t13)*t5*t16+y*(0.622057821E3+
0.27120477E3*t8+0.1347915E3*t13+0.1613584E3*t19)*t5*t2+y*(0.3524669E3*t19+
0.800909718E4-0.21988491E4*t8+0.403395E3*t24-0.953646E3*t13)*t5*t1+y*(
0.235865302E5-0.49883761E5*t8-0.2653856E5*t13-0.8073139E4*t19+0.575165E3*t24+
0.726112E3*t29)*t5+y*(0.610620905E6+0.96815E3*t33-0.8640778E5*t8-0.7252097E5*
t19+0.571646E3*t29-0.2313705E5*t24-0.15346755E6*t13)*t3*t16;
      t66 = s1+y*(0.260201E3*t33-0.7990262E5*t24-0.373719E5*t29+0.123164039E7+0.9000099E6*t13+0.968151E3*t38+0.1866824E6*t19-0.752882927E7*t8)*t3*t2+y*(
            0.332895441E8*t13-0.8821E4*t29-0.3754485E5*t33-0.125589E3*t38+0.1698456871E8-0.40738185E7*t8+0.726112E3*t43+0.9347172E6*t24+0.1931143E7*t19)*t4+y*(
            -0.2350768E5*t38+0.6621212E5*t33+0.4033959E3*t47-24620128.0*t19-0.296381E3*t43+0.10029962E7*t29-0.140676389E9*t8+0.631770243E8-5569654.0*t13-0.4681427E6*t24)*
            t3+y*t52*t16+y*t56*t2+y*t60*t1+y*t64;



      t1 = y*y;
      t2 = t1*t1;
      t3 = t2*t1;
      t5 = x*x;
      t6 = t5*t5;
      t7 = t6*t5;
      t8 = t6*t6;
      t9 = t8*t8;
      t12 = t2*t2;
      t13 = t12*t12;
      t14 = t13*t12;
      t15 = t13*t1;
      t16 = t13*t3;
      t17 = t13*t2;
      t18 = t12*t3;
      t19 = t8*t6;
      t21 = t12*t2;
      t23 = t12*t1;
      t24 = 1028266433.0+(0.9551943356E3+0.19887546E3*t2+364.0*t3+0.53325372E3*
t1)*t9*t7+0.4837369216E6*t13+0.1265316307E3*t14-0.7094609824E5*t15
-0.9551943356E3*t16+0.9504646789E4*t17-0.2857212666E7*t18+t9*t19+t13*t21+
1170224774.0*t2-1559901270.0*t1-0.6111483527E8*t23+0.1446475222E8*t21;
      t25 = 0.19547025E7*t21-50101721.0*t12-5257223.0*t23+0.2403728E6*t18
-0.5558193E5*t13+63349711.0*t2+0.2111073882E9-199846195.0*t3-0.101279E4*t15
-289676232.0*t1+1001.0*t17;
      t27 = -0.16493704E5*t15-0.68900202E7*t21+660077110.0*t2+54037122.0*t3+
0.5790991644E9+199846195.0*t12+0.2804273E6*t18+0.16146064E6*t13-6876554.0*t23
-753829540.0*t1+364.0*t16-0.5671632E3*t17;
      t31 = 1060017992.0*t2+0.148410241E8*t21-0.10635181E7*t18-0.21780123E6*t13
+63349712.0*t12+1170224774.0-1664209012.0*t1-0.19887546E3*t16+0.48153269E5*t15+
91.0*t14-46039602.0*t23-660077110.0*t3-0.15001711E4*t17;
      t39 = t8*t5;
      t53 = t13*t23;
      t54 = -0.556001184E5*t15+1664209011.0*t2+289676232.0*t12+1559901270.0+
0.53325371E3*t16+0.498334204E6*t13+14.0*t53+0.30582613E4*t17+0.139465295E8*t21
-0.701358111E8*t23-0.284954577E7*t18-2288544194.0*t1-0.405116658E2*t14
-753829540.0*t3;
      s1 = 0.2111073882E9*t12-0.5790991644E9*t3+t25*t8+t27*t7+(-0.498334204E6*
t1+0.1093818E4*t23-0.16146065E6*t3-0.5558192E5*t12+0.4837369215E6-0.21780123E6*
t2+3003.0*t21)*t9+t31*t6+(0.48613E3*t21+3432.0*t18-0.284954577E7*t1+0.2804273E6
*t3-0.1066634E6*t23+0.2857212666E7+0.10635181E7*t2-0.2403728E6*t12)*t8*t7;
      t56 = s1+(-0.1230519E6*t23+0.1446475222E8-0.139465295E8*t1+0.68900202E7*
t3-0.48613E3*t18+0.19547023E7*t12-0.1313364E6*t21+3003.0*t13+0.148410239E8*t2)*
t19+(-6876555.0*t3-0.70135811E8*t1+0.6111483528E8+0.30431574E7*t23+46039601.0*
t2+5257223.0*t12+0.1230519E6*t21+2002.0*t15-0.1093817E4*t13-0.1066634E6*t18)*
t39+(-0.556001184E5*t1+2002.0*t23-0.48153268E5*t2+0.101279E4*t12+0.7094609824E5
-0.16493703E5*t3)*t9*t5+(-0.30582613E4*t1+1001.0*t12+0.9504646789E4
-0.15001711E4*t2+0.5671632E3*t3)*t9*t6+(14.0*t1+0.368287872E1)*t9*t39+(91.0*t2+
0.1265316307E3+0.405116658E2*t1)*t9*t8-0.368287872E1*t53+t54*t5;
      t57 = t24+t56;

 return t66/t57+exp(t1-t5)*cos(2*x*y);
}

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  VoigtCase1(xinf,xsup,y,Nbp,*lpVoigt)                                              *
 *                                                                                              *
 * PURPOSE:   Handles the case y<0.75. Returns in lpVoigt the line simulated on 'Nbp' points    *
 *            from 'xinf', to 'xsup'.                   ¨                                       *
 *                                                                                              *
 ***********************************************************************************************/
void VoigtCase1(double xinf, double xsup, double y, int32_t Nbp, double *lpVoigt)
{
 double Disp;
 int32_t   i,i1,i2,i3,i4,i5,i6;

 Disp=(xsup-xinf)/(double)(Nbp-1);

 // Limit points
 i1=(int32_t)((y-15.0-xinf)/Disp);
 i2=(int32_t)((y-5.5-xinf)/Disp);
 i3=(int32_t)((-(15.4*y+2.7)/3.0-xinf)/Disp);
 i4=(int32_t)((+(15.4*y+2.7)/3.0-xinf)/Disp);
 i5=(int32_t)((5.5-y-xinf)/Disp);
 i6=(int32_t)((15.0-y-xinf)/Disp);

 // Bounds to physical limits
 i1=min(max(0,i1),Nbp-1);
 i2=min(max(0,i2),Nbp-1);
 i3=min(max(0,i3),Nbp-1);
 i4=min(max(0,i4),Nbp-1);
 i5=min(max(0,i5),Nbp-1);
 i6=min(max(0,i6),Nbp-1);

 // Calculates for limit to limit.
 for (i=0;i<i1;++i)
     lpVoigt[i]=fRegionI(xinf+(double)i*Disp,y);
 for (i=i1;i<i2;++i)
     lpVoigt[i]=fRegionII(xinf+(double)i*Disp,y);
 for (i=i2;i<i3;++i)
     lpVoigt[i]=fRegionIV(xinf+(double)i*Disp,y);
 for (i=i3;i<i4;++i)
     lpVoigt[i]=fRegionIII(xinf+(double)i*Disp,y);
 for (i=i4;i<i5;++i)
     lpVoigt[i]=fRegionIV(xinf+(double)i*Disp,y);
 for (i=i5;i<i6;++i)
     lpVoigt[i]=fRegionII(xinf+(double)i*Disp,y);
 for (i=i6;i<Nbp;++i)
     lpVoigt[i]=fRegionI(xinf+(double)i*Disp,y);
}

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  VoigtCase2(xinf,xsup,y,Nbp,*lpVoigt)                                              *
 *                                                                                              *
 * PURPOSE:   Handles the case 0.75<=y<5.5. Returns in lpVoigt the line simulated on 'Nbp'      *
 *            points from 'xinf', to 'xsup'.                   ¨                                *
 *                                                                                              *
 ***********************************************************************************************/
void VoigtCase2(double xinf, double xsup, double y, int32_t Nbp, double *lpVoigt)
{
 double Disp;
 int32_t   i,i1,i2,i3,i4;

 Disp=(xsup-xinf)/(double)(Nbp-1);

 // Limit points
 i1=(int32_t)((y-15.0-xinf)/Disp);
 i2=(int32_t)((y-5.5-xinf)/Disp);
 i3=(int32_t)((5.5-y-xinf)/Disp);
 i4=(int32_t)((15.0-y-xinf)/Disp);

 // Bounds to physical limits
 i1=min(max(0,i1),Nbp-1);
 i2=min(max(0,i2),Nbp-1);
 i3=min(max(0,i3),Nbp-1);
 i4=min(max(0,i4),Nbp-1);

 // Calculates for limit to limit.
 for (i=0;i<i1;++i)
     lpVoigt[i]=fRegionI(xinf+(double)i*Disp,y);
 for (i=i1;i<i2;++i)
     lpVoigt[i]=fRegionII(xinf+(double)i*Disp,y);
 for (i=i2;i<i3;++i)
     lpVoigt[i]=fRegionIII(xinf+(double)i*Disp,y);
 for (i=i3;i<i4;++i)
     lpVoigt[i]=fRegionII(xinf+(double)i*Disp,y);
 for (i=i4;i<Nbp;++i)
     lpVoigt[i]=fRegionI(xinf+(double)i*Disp,y);
}

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  VoigtCase3(xinf,xsup,y,Nbp,*lpVoigt)                                              *
 *                                                                                              *
 * PURPOSE:   Handles the case 5.5<=y<15. Returns in lpVoigt the line simulated on 'Nbp' points *
 *            from 'xinf', to 'xsup'.                   ¨                                       *
 *                                                                                              *
 ***********************************************************************************************/
void VoigtCase3(double xinf, double xsup, double y, int32_t Nbp, double *lpVoigt)
{
 double Disp;
 int32_t   i,i1,i2;

 Disp=(xsup-xinf)/(double)(Nbp-1);

 // Limit points
 i1=(int32_t)((y-15.0-xinf)/Disp);
 i2=(int32_t)((15.0-y-xinf)/Disp);

 // Bounds to physical limits
 i1=min(max(0,i1),Nbp-1);
 i2=min(max(0,i2),Nbp-1);

 // Calculates for limit to limit.
 for (i=0;i<=i1;++i)
     lpVoigt[i]=fRegionI(xinf+(double)i*Disp,y);
 for (i=i1+1;i<=i2;++i)
     lpVoigt[i]=fRegionII(xinf+(double)i*Disp,y);
 for (i=i2+1;i<Nbp;++i)
     lpVoigt[i]=fRegionI(xinf+(double)i*Disp,y);
}

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  VoigtCase1(xinf,xsup,y,Nbp,*lpVoigt)                                              *
 *                                                                                              *
 * PURPOSE:   Handles the case y>=15. Returns in lpVoigt the line simulated on 'Nbp' points     *
 *            from 'xinf', to 'xsup'.                   ¨                                       *
 *                                                                                              *
 ***********************************************************************************************/
void VoigtCase4(double xinf, double xsup, double y, int32_t Nbp, double *lpVoigt)
{
 double Disp;
 int32_t   i;

 Disp=(xsup-xinf)/(double)(Nbp-1);

 for (i=0;i<Nbp;++i)
     lpVoigt[i]=fRegionI(xinf+(double)i*Disp,y);
}

// -----------------------------------------------------------------------------
// FUNCTION      Voigtx
// -----------------------------------------------------------------------------
// PURPOSE       calculate the Voigt profile function in the region determined by
//               the values of input x and y;
//
// INPUT         x : the distance from the line center in units of Doppler halfwidths;
//               y : the ratio of the Lorentzian halfwidth to the Doppler halfwidth;
// -----------------------------------------------------------------------------
// RETURN        the Voigt profile function calculated for the pair (x,y)
// -----------------------------------------------------------------------------

double Voigtx(double x,double y)
 {
  // Declarations

  double rc;
  double xy;

  // Initializations

  x=fabs(x);

  xy=fabs(x)+y;
  rc=(double)0.;

  // Calculate the Voigt profile function in the region determined by the values of input x and y

  if (y*y-x*x>(double)700.)
   ERROR_SetLast("Voigtx",ERROR_TYPE_WARNING,ERROR_ID_OVERFLOW);
  else if (xy>(double)15.)
   rc=fRegionI(x,y);
  else if ((xy>(double)5.5) && (xy<(double)15.))
   rc=fRegionII(x,y);
  else if ((xy<(double)5.5) && (y>(double)0.195*x-0.176))
   rc=fRegionIII(x,y);
  else if ((xy<(double)5.5) && (y<(double)0.195*x-0.176))
   rc=fRegionIV(x,y);
  else
   ERROR_SetLast("Voigtx",ERROR_TYPE_WARNING,ERROR_ID_VOIGT,x,y);

  // Return

  return rc;
 }

/************************************************************************************************
 *                                                                                              *
 * FUNCTION:  Voigt(xinf,xsup,y,Nbp,*lpVoigt)                                                   *
 *                                                                                              *
 * PURPOSE:   Compute the voigt function in lpVoigt. The line is simulated on 'Nbp' points from *
 *            'xinf', to 'xsup'.                   ¨                                            *
 *                                                                                              *
 ***********************************************************************************************/
void Voigt(double xinf, double xsup, double y, int32_t Nbp, double *lpVoigt)
{
 int32_t i;

 // Clean-up the vector
 for (i=0;i<Nbp;++i)
     lpVoigt[i]=0.0;

 // Split the domain according to Humlicek's method.
 if (y<0.75)
    VoigtCase1(xinf,xsup,y,Nbp,lpVoigt);
 else if (y<5.5)
    VoigtCase2(xinf,xsup,y,Nbp,lpVoigt);
 else if (y<15.0)
    VoigtCase3(xinf,xsup,y,Nbp,lpVoigt);
 else
    VoigtCase4(xinf,xsup,y,Nbp,lpVoigt);
}

