#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME:
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP rlas_lasdatareader(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP rlas_lasfilterusage();
extern SEXP rlas_lasheaderreader(SEXP);
extern SEXP rlas_laswriter(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"rlas_lasdatareader",   (DL_FUNC) &rlas_lasdatareader,   13},
    {"rlas_lasfilterusage",  (DL_FUNC) &rlas_lasfilterusage,   0},
    {"rlas_lasheaderreader", (DL_FUNC) &rlas_lasheaderreader,  1},
    {"rlas_laswriter",       (DL_FUNC) &rlas_laswriter,       18},
    {NULL, NULL, 0}
};

void R_init_rlas(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}

