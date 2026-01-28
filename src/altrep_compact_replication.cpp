#include "altrepisode.h"
#include <type_traits>

template<typename T>
struct repetition {
  unsigned int size;
  T value;
  repetition(int n, T v) : size(n), value(v) {}
};

static R_altrep_class_t compact_repetition_integer;
static R_altrep_class_t compact_repetition_real;
static R_altrep_class_t compact_repetition_logical;

template<typename T>
struct compact_repetition
{
  // constructor function
  static SEXP Make(repetition<T>* data, bool owner)
  {
    //Rprintf("Making a compact repetition with %d x %d\n", data->value, data->size);

    R_altrep_class_t class_t;

    SEXP xp = PROTECT(R_MakeExternalPtr(data, R_NilValue, R_NilValue));
    if (owner) R_RegisterCFinalizerEx(xp, compact_repetition::Finalize, TRUE);

    if (std::is_same<T, int>::value)
      class_t = compact_repetition_integer;

    if (std::is_same<T, double>::value)
      class_t = compact_repetition_real;

    if (std::is_same<T, bool>::value)
      class_t = compact_repetition_logical;

    SEXP res = R_new_altrep(class_t, xp, R_NilValue);
    UNPROTECT(1);
    return res;
  }

  // finalizer for the external pointer
  static void Finalize(SEXP xp){
    //Rprintf("Deleting a compact repetition at %p\n", R_ExternalPtrAddr(xp));
    delete static_cast<repetition<T>*>(R_ExternalPtrAddr(xp));
  }

  // get the std::vector<string>* from the altrep object `x`
  static repetition<T>* Ptr(SEXP x) {
    return static_cast<repetition<T>*>(R_ExternalPtrAddr(R_altrep_data1(x)));
  }

  // same, but as a reference, for convenience
  static repetition<T>& Get(SEXP vec) {
    return *Ptr(vec) ;
  }

  // ALTREP methods -------------------
  // The length of the object
  static R_xlen_t Length(SEXP vec){
    return Get(vec).size;
  }

  static SEXP int_Max(SEXP vec, Rboolean writable){
    return Rf_ScalarInteger(Get(vec).value);
  }

  static SEXP int_Min(SEXP vec, Rboolean writable){
    return Rf_ScalarInteger(Get(vec).value);
  }

  static SEXP real_Max(SEXP vec, Rboolean writable){
    return Rf_ScalarReal(Get(vec).value);
  }

  static SEXP real_Min(SEXP vec, Rboolean writable){
    return Rf_ScalarReal(Get(vec).value);
  }

  // What gets printed when .Internal(inspect()) is used
  static Rboolean Inspect(SEXP x, int pre, int deep, int pvec, void (*inspect_subtree)(SEXP, int, int, int)){
    //Rprintf("compact representaion of %d x %d\n", Get(x).value, Length(x));
    return TRUE;
  }

  // ALTVEC methods ------------------

  static SEXP Serialized_state_int(SEXP x)
  {
    //Rprintf("Calling serialize_state on a compact_repetition at %p\n", R_ExternalPtrAddr(x));

    int val = Get(x).value;
    int n = Get(x).size;
    SEXP size = PROTECT(Rf_ScalarInteger(n));
    SEXP value = PROTECT(Rf_ScalarInteger(val));
    SEXP vec = PROTECT(Rf_allocVector(VECSXP, 2));
    SET_VECTOR_ELT(vec, 0, value);
    SET_VECTOR_ELT(vec, 1, size);
    UNPROTECT(3);
    return vec;
  }

  static SEXP Serialized_state_dbl(SEXP x)
  {
    //Rprintf("Calling serialize_state on a compact_repetition at %p\n", R_ExternalPtrAddr(x));

    double val = Get(x).value;
    int n = Get(x).size;
    SEXP size = PROTECT(Rf_ScalarInteger(n));
    SEXP value = PROTECT(Rf_ScalarReal(val));
    SEXP vec = PROTECT(Rf_allocVector(VECSXP, 2));
    SET_VECTOR_ELT(vec, 0, value);
    SET_VECTOR_ELT(vec, 1, size);
    UNPROTECT(3);
    return vec;
  }

  static SEXP Serialized_state_lgl(SEXP x)
  {
    //Rprintf("Calling serialize_state on a compact_repetition at %p\n", R_ExternalPtrAddr(x));

    double val = Get(x).value;
    int n = Get(x).size;
    SEXP size = PROTECT(Rf_ScalarInteger(n));
    SEXP value = PROTECT(Rf_ScalarLogical(val));
    SEXP vec = PROTECT(Rf_allocVector(VECSXP, 2));
    SET_VECTOR_ELT(vec, 0, value);
    SET_VECTOR_ELT(vec, 1, size);
    UNPROTECT(3);
    return vec;
  }

  static SEXP Unserialize(SEXP /* class_ */, SEXP state)
  {
    //Rprintf("Calling unserialize on a compact_repetition\n");
    int n = Rf_asInteger(VECTOR_ELT(state, 1));
    //Rprintf("n = %d\n", n);

    switch (TYPEOF(VECTOR_ELT(state, 0)))
    {
    case INTSXP:
    {
      //Rprintf("unserialize int\n");
      auto x = new repetition<int>(n , Rf_asInteger(VECTOR_ELT(state, 0)));
      return compact_repetition<int>::Make(x, true);
      break;
    }
    case REALSXP:
    {
      //Rprintf("unserialize dbl\n");
      auto x = new repetition<double>(n, Rf_asReal(VECTOR_ELT(state, 0)));
      return compact_repetition<double>::Make(x, true);
      break;
    }
    case LGLSXP:
    {
      //Rprintf("unserialize bool\n");
      auto x = new repetition<bool>(n, Rf_asLogical(VECTOR_ELT(state, 0)));
      return compact_repetition<bool>::Make(x, true);
      break;
    }
    default:
    {
      Rf_error("Not supported input SEXP in compact repetition");
      break;
    }
    }
  }

  // The start of the data, i.e. the underlying string array from the random_string_data
  //
  // This is guaranteed to never allocate (in the R sense)
  static const void* Dataptr_or_null(SEXP vec) {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 == R_NilValue) return nullptr;

    switch (TYPEOF(data2)) {
    case INTSXP: return INTEGER(data2);
    case REALSXP: return REAL(data2);
    case LGLSXP: return LOGICAL(data2);
    case RAWSXP: return RAW(data2);
    default: return nullptr; // fallback if unknown type
    }
  }

  static void* DataptrInt(SEXP vec, Rboolean writeable)
  {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 != R_NilValue) {
      return INTEGER(data2);
    }

    int n = Length(vec);
    auto v = Get(vec).value;
    SEXP val = PROTECT(Rf_allocVector(INTSXP, n));
    int *p = INTEGER(val);
    for (int i = 0; i < n; i++) p[i] = v;
    R_set_altrep_data2(vec, val);
    UNPROTECT(1);
    return INTEGER(val);
  }

  static void* DataptrReal(SEXP vec, Rboolean writeable)
  {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 != R_NilValue) {
      return REAL(data2);
    }

    int n = Length(vec);
    double v = Get(vec).value;
    SEXP val = PROTECT(Rf_allocVector(REALSXP, n));
    double *p = REAL(val);
    for (int i = 0; i < n; i++) p[i] = v;
    R_set_altrep_data2(vec, val);
    UNPROTECT(1);
    return REAL(val);
  }

  static void* DataptrLogical(SEXP vec, Rboolean writeable)
  {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 != R_NilValue) {
      return LOGICAL(data2);
    }

    int n = Length(vec);
    bool v = Get(vec).value;
    SEXP val = PROTECT(Rf_allocVector(LGLSXP, n));
    int *p = LOGICAL(val);
    for (int i = 0; i < n; i++) p[i] = v ? TRUE : FALSE;
    R_set_altrep_data2(vec, val);
    UNPROTECT(1);
    return LOGICAL(val);
  }


  // ALTINT methods -----------------
  // the element at the index `i`
  // this does not do bounds checking because that's expensive, so
  // the caller must take care of that
  static int int_Elt(SEXP vec, R_xlen_t i) {
    //Rprintf("int_Elt method called at %d \n", i+1);
    if (i > Length(vec)) { return NA_INTEGER; }
    return Get(vec).value;
  }

  static double real_Elt(SEXP vec, R_xlen_t i) {
    //Rprintf("real_Elt method called at %d \n", i+1);
    if (i > Length(vec)) { return NA_REAL; }
    return Get(vec).value;
  }

  static int logical_Elt(SEXP vec, R_xlen_t i) {
    //Rprintf("logical_Elt method called at %d \n", i+1);
    if (i > Length(vec)) { return NA_LOGICAL; }
    return Get(vec).value ? TRUE : FALSE;
  }

  static SEXP extract_subset_int(SEXP x, SEXP indx, SEXP call)
  {
    //Rprintf("Extracting subset\n");
    if (x == R_NilValue) return x;

    int *p = INTEGER(indx);
    R_xlen_t n = XLENGTH(indx);
    R_xlen_t s = Length(x);
    int v = Get(x).value;

    bool indx_out_of_bound = false;
    for (int i = 0; i < n; i++)
    {
      if (p[i] > s || p[i] < 1)
      {
        indx_out_of_bound = true;
        break;
      }
    }

    if (indx_out_of_bound)
    {
      SEXP out = PROTECT(Rf_allocVector(INTSXP, n));
      int *pout = INTEGER(out);
      p = INTEGER(indx);
      for (int i = 0; i < n; i++)
      {
        if (p[i] <= s && p[i] > 0)
          pout[i] = v;
        else
          pout[i] = NA_INTEGER;
      }
      UNPROTECT(1);
      return out;
    }
    else
    {
      auto viu = new repetition<T>(n, v);
      return compact_repetition::Make(viu, true);
    }
  }

  static SEXP extract_subset_real(SEXP x, SEXP indx, SEXP call)
  {
    //Rprintf("Extracting subset\n");
    if (x == R_NilValue) return x;

    int *p = INTEGER(indx);
    R_xlen_t n = XLENGTH(indx);
    R_xlen_t s = Length(x);
    double v = Get(x).value;

    bool indx_out_of_bound = false;
    for (int i = 0; i < n; i++)
    {
      if (p[i] > s || p[i] < 1)
      {
        indx_out_of_bound = true;
        break;
      }
    }

    if (indx_out_of_bound)
    {
      SEXP out = PROTECT(Rf_allocVector(REALSXP, n));
      double *pout = REAL(out);
      p = INTEGER(indx);
      for (int i = 0; i < n; i++)
      {
        if (p[i] <= s && p[i] > 0)
          pout[i] = v;
        else
          pout[i] = NA_REAL;
      }
      UNPROTECT(1);
      return out;
    }
    else
    {
      auto viu = new repetition<T>(n, v);
      return compact_repetition::Make(viu, true);
    }
  }

  static SEXP extract_subset_logical(SEXP x, SEXP indx, SEXP call)
  {
    //Rprintf("Extracting subset\n");
    if (x == R_NilValue) return x;

    int *p = INTEGER(indx);
    R_xlen_t n = XLENGTH(indx);
    R_xlen_t s = Length(x);
    bool v = Get(x).value;

    bool indx_out_of_bound = false;
    for (int i = 0; i < n; i++)
    {
      if (p[i] > s || p[i] < 1)
      {
        indx_out_of_bound = true;
        break;
      }
    }

    if (indx_out_of_bound)
    {
      SEXP out = PROTECT(Rf_allocVector(LGLSXP, n));
      int *pout = LOGICAL(out);
      p = INTEGER(indx);
      for (int i = 0; i < n; i++)
      {
        if (p[i] <= s && p[i] > 0)
          pout[i] = v ? TRUE: FALSE;
        else
          pout[i] = NA_LOGICAL;
      }
      UNPROTECT(1);
      return out;
    }
    else
    {
      auto viu = new repetition<T>(n, v);
      return compact_repetition::Make(viu, true);
    }
  }

  // -------- initialize the altrep class with the methods above
  static void InitInt(DllInfo* dll)
  {
    R_altrep_class_t class_t = R_make_altinteger_class("compact repetition (int)", "rlas", dll);
    compact_repetition_integer = class_t;

    // altrep
    R_set_altrep_Length_method(class_t, Length);
    R_set_altrep_Inspect_method(class_t, Inspect);

    // altvec
    R_set_altvec_Dataptr_method(class_t, DataptrInt);
    R_set_altvec_Dataptr_or_null_method(class_t, Dataptr_or_null);
    R_set_altvec_Extract_subset_method(class_t, extract_subset_int);
    R_set_altrep_Serialized_state_method(class_t, Serialized_state_int);
    R_set_altrep_Unserialize_method(class_t, Unserialize);

    // altint
    R_set_altinteger_Elt_method(class_t, int_Elt);
    R_set_altinteger_Min_method(class_t, int_Max);
    R_set_altinteger_Max_method(class_t, int_Min);
  }

  static void InitReal(DllInfo* dll)
  {
    R_altrep_class_t class_t = R_make_altreal_class("compact repetition (double)", "rlas", dll);
    compact_repetition_real = class_t;

    // altrep
    R_set_altrep_Length_method(class_t, Length);
    R_set_altrep_Inspect_method(class_t, Inspect);

    // altvec
    R_set_altvec_Dataptr_method(class_t, DataptrReal);
    R_set_altvec_Dataptr_or_null_method(class_t, Dataptr_or_null);
    R_set_altvec_Extract_subset_method(class_t, extract_subset_real);
    R_set_altrep_Serialized_state_method(class_t, Serialized_state_dbl);
    R_set_altrep_Unserialize_method(class_t, Unserialize);

    // altint
    R_set_altreal_Elt_method(class_t, real_Elt);
    R_set_altreal_Min_method(class_t, real_Max);
    R_set_altreal_Max_method(class_t, real_Min);
  }

  static void InitLogical(DllInfo* dll)
  {
    R_altrep_class_t class_t = R_make_altlogical_class("compact repetition (bool)", "rlas", dll);
    compact_repetition_logical = class_t;

    // altrep
    R_set_altrep_Length_method(class_t, Length);
    R_set_altrep_Inspect_method(class_t, Inspect);

    // altvec
    R_set_altvec_Dataptr_method(class_t, DataptrLogical);
    R_set_altvec_Dataptr_or_null_method(class_t, Dataptr_or_null);
    R_set_altvec_Extract_subset_method(class_t, extract_subset_logical);
    R_set_altrep_Serialized_state_method(class_t, Serialized_state_lgl);
    R_set_altrep_Unserialize_method(class_t, Unserialize);

    // altlgl
    R_set_altlogical_Elt_method(class_t, logical_Elt);
  }
};

// Called when the package is loaded (needs Rcpp 0.12.18.3)
// [[Rcpp::init]]
void init_alt_rep(DllInfo* dll){
  compact_repetition<int>::InitInt(dll);
  compact_repetition<double>::InitReal(dll);
  compact_repetition<bool>::InitLogical(dll);
}

// [[Rcpp::export]]
SEXP R_compact_rep(int n, SEXP v)
{
  switch (TYPEOF(v))
  {
    case INTSXP:
    {
      auto x = new repetition<int>(n, Rf_asInteger(v));
      return compact_repetition<int>::Make(x, true);
      break;
    }
    case REALSXP:
    {
      auto x = new repetition<double>(n, Rf_asReal(v));
      return compact_repetition<double>::Make(x, true);
      break;
    }
    case LGLSXP:
    {
      auto x = new repetition<bool>(n, Rf_asLogical(v));
      return compact_repetition<bool>::Make(x, true);
      break;
    }
    default:
    {
      Rf_error("Not supported input SEXP in compact repetition");
      break;
    }
  }
}

// [[Rcpp::export]]
bool R_is_altrep(SEXP x)
{
  return ALTREP(x);
}

// [[Rcpp::export]]
bool R_is_materialized(SEXP x)
{
  return DATAPTR_OR_NULL(x) != nullptr;
}

// [[Rcpp::export]]
SEXP R_altrep_full_class(SEXP x) {
  if (!ALTREP(x)) return R_NilValue;
  #if R_VERSION >= R_Version(4, 6, 0)
    return R_altrep_class_package(x);
  #else
    return ATTRIB(ALTREP_CLASS(x));
  #endif
}
