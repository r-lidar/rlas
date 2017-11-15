#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME:
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP _rlas_lasdatareader(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _rlas_lasdatareader_inpoly(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _rlas_lasfilterusage();
extern SEXP _rlas_lasheaderreader(SEXP);
extern SEXP _rlas_laswriter(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _rlas_laxwriter(SEXP);
extern SEXP _rlas_point_in_polygon(SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"_rlas_lasdatareader",        (DL_FUNC) &_rlas_lasdatareader,          24},
    {"_rlas_lasdatareader_inpoly", (DL_FUNC) &_rlas_lasdatareader_inpoly,   26},
    {"_rlas_lasfilterusage",       (DL_FUNC) &_rlas_lasfilterusage,          0},
    {"_rlas_lasheaderreader",      (DL_FUNC) &_rlas_lasheaderreader,         1},
    {"_rlas_laswriter",            (DL_FUNC) &_rlas_laswriter,              18},
    {"_rlas_laxwriter",            (DL_FUNC) &_rlas_laxwriter,               1},
    {"_rlas_point_in_polygon",     (DL_FUNC) &_rlas_point_in_polygon,        4},
    {NULL, NULL, 0}
};

void R_init_rlas(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}

