/**************
 *  Energy.c  *
 **************/


/*=====================================*
 * Berechnet die potentielle Energie.  *
 *=====================================*/


#include <math.h>
#include <stdio.h>
#include "vector2.h"
#include "vector3.h"
#include "data.h"
#include "cubature.h"
#include "interpolate_pwl.h"
#include "gauss_square.h"
#include "energy_pwl.h"
#include "kern.h"
#include "phi.h"


#if !defined pi
#define pi 3.1415926535897932385
#endif


double energy_orig_pwl(u, F, T, p, m)
double *u;                      /* vorgegebene Dichte                          */
unsigned int **F;               /* Patchliste                                  */
vector3 ****T;                  /* Koeffizienten zur Oberflaecheninterpolation */
unsigned int p;                 /* Anzahl der Parametergebiete                 */
unsigned int m;                 /* Zahl der Level                              */
{
    unsigned int n = 1 << m;    /* n*n Patches pro Parametergebiet             */
    unsigned int i1, i2, i3;    /* Laufindizes fuer Ansatzfunktion             */
    unsigned int zi = 0;        /* Zeilenindex hieraus: zi = i1*(n*n)+i2*n+i3  */
    cubature *Q;                /* Kubatur-Formeln                             */
    unsigned int g = 0;         /* Quadraturgrad                               */
    unsigned int l;             /* Laufindex fuer Quadratur                    */
    vector2 s;                  /* Linker, unterer Eckpunkt des Patches zi     */
    vector2 t;                  /* Auswertepunkte der Gaussquadratur           */
    double U;                   /* Auswertung der Dichte im Quadraturpunkt     */
    double E = 0;               /* Energie                                     */
    double C = 0;               /* Charge                                      */
    double h = 1. / n;          /* Schrittweite                                */

/* Initialisierung */
    init_Gauss_Square(&Q, g + 1);       /* Kubatur-Formeln */

/* Berechnung des Fehlers */
    for (i1 = 0; i1 < p; i1++) {
        s.y = 0;
        for (i2 = 0; i2 < n; i2++) {
            s.x = 0;
            for (i3 = 0; i3 < n; i3++) {        /* zeilenweise Durchnumerierung der Patches zi = (i1,i2,i3) */
                for (l = 0; l < Q[g].nop; l++) {
                    t = vector2_add(s, vector2_Smul(h, Q[g].xi[l]));
                    U = u[F[zi][0]] * Phi0(Q[g].xi[l])
                        + u[F[zi][1]] * Phi1(Q[g].xi[l])
                        + u[F[zi][2]] * Phi2(Q[g].xi[l])
                        + u[F[zi][3]] * Phi3(Q[g].xi[l]);
                    E += Q[g].w[l] * U * f(Chi_pwl(t, T[i1], m));
                    C += Q[g].w[l] * U;
                }
                s.x += h;
                zi++;
            }
            s.y += h;
        }
    }
    E = -0.5 * h * E; /* correct scaling */
    printf("Computed energy:            %g\n", E);
    printf("Computed charge:            %g\n", C*h);
    free_Gauss_Square(&Q, g + 1);
    return (E);
}

double energy_pwl(u, potential, F, T, p, m)
double *u;                      /* vorgegebene Dichte                          */
double *potential;              /* electrostatic potential                     */
unsigned int **F;               /* Patchliste                                  */
vector3 ****T;                  /* Koeffizienten zur Oberflaecheninterpolation */
unsigned int p;                 /* Anzahl der Parametergebiete                 */
unsigned int m;                 /* Zahl der Level                              */
{
    unsigned int n = 1 << m;    /* n*n Patches pro Parametergebiet             */
    unsigned int i1, i2, i3;    /* Laufindizes fuer Ansatzfunktion             */
    unsigned int zi = 0;        /* Zeilenindex hieraus: zi = i1*(n*n)+i2*n+i3  */
    cubature *Q;                /* Kubatur-Formeln                             */
    unsigned int g = 0;         /* Quadraturgrad                               */
    unsigned int l;             /* Laufindex fuer Quadratur                    */
    vector2 s;                  /* Linker, unterer Eckpunkt des Patches zi     */
    vector2 t;                  /* Auswertepunkte der Gaussquadratur           */
    double U;                   /* Auswertung der Dichte im Quadraturpunkt     */
    double E = 0;               /* Energie                                     */
    double h = 1. / n;          /* Schrittweite                                */

/* Initialisierung */
    init_Gauss_Square(&Q, g + 1);       /* Kubatur-Formeln */

/* Berechnung des Fehlers */
    for (i1 = 0; i1 < p; i1++) {
        s.y = 0;
        for (i2 = 0; i2 < n; i2++) {
            s.x = 0;
            for (i3 = 0; i3 < n; i3++) {        /* zeilenweise Durchnumerierung der Patches zi = (i1,i2,i3) */
                for (l = 0; l < Q[g].nop; l++) {
                    t = vector2_add(s, vector2_Smul(h, Q[g].xi[l]));
                    U = u[F[zi][0]] * Phi0(Q[g].xi[l])
                        + u[F[zi][1]] * Phi1(Q[g].xi[l])
                        + u[F[zi][2]] * Phi2(Q[g].xi[l])
                        + u[F[zi][3]] * Phi3(Q[g].xi[l]);
                    E += Q[g].w[l] * U * potential[zi];
                }
                s.x += h;
                zi++;
            }
            s.y += h;
        }
    }
    E = -0.5 * h * E; /* correct scaling */
    printf("Computing the energy_pwl:            %g\n", E);
    free_Gauss_Square(&Q, g + 1);
    return (E);
}

double charge_pwl(u, charge, F, T, p, m)
double *u;                      /* vorgegebene Dichte                          */
double *charge;                 /* surface charge (old PCM style)              */
unsigned int **F;               /* Patchliste                                  */
vector3 ****T;                  /* Koeffizienten zur Oberflaecheninterpolation */
unsigned int p;                 /* Anzahl der Parametergebiete                 */
unsigned int m;                 /* Zahl der Level                              */
{
    unsigned int n = 1 << m;    /* n*n Patches pro Parametergebiet             */
    unsigned int i1, i2, i3;    /* Laufindizes fuer Ansatzfunktion             */
    unsigned int zi = 0;        /* Zeilenindex hieraus: zi = i1*(n*n)+i2*n+i3  */
    cubature *Q;                /* Kubatur-Formeln                             */
    unsigned int g = 0;         /* Quadraturgrad                               */
    unsigned int l;             /* Laufindex fuer Quadratur                    */
    vector2 s;                  /* Linker, unterer Eckpunkt des Patches zi     */
    vector2 t;                  /* Auswertepunkte der Gaussquadratur           */
    double U;                   /* Auswertung der Dichte im Quadraturpunkt     */
    double h = 1. / n;          /* Schrittweite                                */
    double C = 0;               /* surface meassure                            */

/* Initialisierung */
    init_Gauss_Square(&Q, g + 1);       /* Kubatur-Formeln */

/* Berechnung des Fehlers */
    for (i1 = 0; i1 < p; i1++) {
        s.y = 0;
        for (i2 = 0; i2 < n; i2++) {
            s.x = 0;
            for (i3 = 0; i3 < n; i3++) {        /* zeilenweise Durchnumerierung der Patches zi = (i1,i2,i3) */
                for (l = 0; l < Q[g].nop; l++) {
                    t = vector2_add(s, vector2_Smul(h, Q[g].xi[l]));
                    U = u[F[zi][0]] * Phi0(Q[g].xi[l])
                        + u[F[zi][1]] * Phi1(Q[g].xi[l])
                        + u[F[zi][2]] * Phi2(Q[g].xi[l])
                        + u[F[zi][3]] * Phi3(Q[g].xi[l]);
                    charge[zi] = h * Q[g].w[l] * U;
                    C += charge[zi];
                }
                s.x += h;
                zi++;
            }
            s.y += h;
        }
    }
    
    /* Datenausgabe */
    printf("Testing total charge: %g\n", C);
    free_Gauss_Square(&Q, g + 1);
    return (C);
}
