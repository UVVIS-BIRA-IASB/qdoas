
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS               
//  Module purpose    :  GEOCENTRIC MOON POSITIONS COMPUTATION
//  Name of module    :  MOON.H
//  Creation date     :  1996
//
//  ----------------------------------------------------------------------------
//
//  REFERENCES :
//
//  [1] Astronomy on the Personal Computer, Montenbruck & Pfleger, 1989-1994
//  [2] Astronomical formulae for calculators, Meeus, 1979
//  Qdoas is a cross-platform application for spectral analysis with the DOAS
//  algorithm (Differential Optical Analysis software) 
//
//  ----------------------------------------------------------------------------
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
//  ----------------------------------------------------------------------------

#ifndef MOON_H
#define MOON_H

void MOON_GetPosition(char  *inputDate,                                        // input date and time for moon positions calculation
                      double  longitude,                                        // longitude of the observation site
                      double  latitude,                                         // latitude of the observation site
                      double  altitude,                                         // altitude of the observation site
                      double *pA,                                               // azimuth, measured westward from the south
                      double *ph,                                               // altitude above the horizon
                      double *pFrac);                                           // illuminated fraction of the moon

#endif
