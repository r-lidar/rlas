// Compact representation of a vector made of n times the same value. Used for the point
// attributes that are constant in a file (e.g. UserData = 0) to save memory.
// The vector is materialized only when R needs a pointer to its data.

#include "altrepisode.h"

#include <stdexcept>

template <typename T>
struct repetition
{
  unsigned int size;
  T value;
  repetition(int n, T v) : size(n), value(v) {}
};

// R type specific functions for each supported C++ type
template <typename T> struct rtype;

template <> struct rtype<int>
{
  static constexpr SEXPTYPE sexptype = INTSXP;
  static inline R_altrep_class_t altclass;
  static int na() { return NA_INTEGER; }
  static int to_R(int v) { return v; }
  static int* ptr(SEXP x) { return INTEGER(x); }
  static SEXP scalar(int v) { return Rf_ScalarInteger(v); }
};

template <> struct rtype<double>
{
  static constexpr SEXPTYPE sexptype = REALSXP;
  static inline R_altrep_class_t altclass;
  static double na() { return NA_REAL; }
  static double to_R(double v) { return v; }
  static double* ptr(SEXP x) { return REAL(x); }
  static SEXP scalar(double v) { return Rf_ScalarReal(v); }
};

template <> struct rtype<bool>
{
  static constexpr SEXPTYPE sexptype = LGLSXP;
  static inline R_altrep_class_t altclass;
  static int na() { return NA_LOGICAL; }
  static int to_R(bool v) { return v ? TRUE : FALSE; }
  static int* ptr(SEXP x) { return LOGICAL(x); }
  static SEXP scalar(bool v) { return Rf_ScalarLogical(v); }
};

SEXP Unserialize(SEXP, SEXP state);

template <typename T>
struct compact_repetition
{
  using RT = rtype<T>;

  static SEXP Make(repetition<T>* data)
  {
    SEXP xp = PROTECT(R_MakeExternalPtr(data, R_NilValue, R_NilValue));
    R_RegisterCFinalizerEx(xp, Finalize, TRUE);
    SEXP res = R_new_altrep(RT::altclass, xp, R_NilValue);
    UNPROTECT(1);
    return res;
  }

  static SEXP Make(R_xlen_t n, T value) { return Make(new repetition<T>(n, value)); }

  static void Finalize(SEXP xp)
  {
    delete static_cast<repetition<T>*>(R_ExternalPtrAddr(xp));
  }

  static repetition<T>& Get(SEXP vec)
  {
    return *static_cast<repetition<T>*>(R_ExternalPtrAddr(R_altrep_data1(vec)));
  }

  // ALTREP methods -------------------

  static R_xlen_t Length(SEXP vec) { return Get(vec).size; }

  // Nothing special is printed by .Internal(inspect())
  static Rboolean Inspect(SEXP, int, int, int, void (*)(SEXP, int, int, int)) { return TRUE; }

  // Serialized as list(value, size)
  static SEXP Serialized_state(SEXP x)
  {
    SEXP value = PROTECT(RT::scalar(Get(x).value));
    SEXP size  = PROTECT(Rf_ScalarInteger(Get(x).size));
    SEXP vec   = PROTECT(Rf_allocVector(VECSXP, 2));
    SET_VECTOR_ELT(vec, 0, value);
    SET_VECTOR_ELT(vec, 1, size);
    UNPROTECT(3);
    return vec;
  }

  // ALTVEC methods ------------------

  // The materialized data if any. Guaranteed to never allocate (in the R sense)
  static const void* Dataptr_or_null(SEXP vec)
  {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 == R_NilValue) return nullptr;
    return RT::ptr(data2);
  }

  // Materializes the vector the first time a pointer to the data is requested
  static void* Dataptr(SEXP vec, Rboolean)
  {
    SEXP data2 = R_altrep_data2(vec);
    if (data2 != R_NilValue)
      return RT::ptr(data2);

    R_xlen_t n = Length(vec);
    auto v = RT::to_R(Get(vec).value);
    SEXP val = PROTECT(Rf_allocVector(RT::sexptype, n));
    auto* p = RT::ptr(val);
    for (R_xlen_t i = 0 ; i < n ; i++) p[i] = v;
    R_set_altrep_data2(vec, val);
    UNPROTECT(1);
    return RT::ptr(val);
  }

  // The element at the index i. No bounds checking because that's expensive,
  // the caller must take care of that
  static auto Elt(SEXP vec, R_xlen_t i)
  {
    if (i > Length(vec)) return RT::na();
    return RT::to_R(Get(vec).value);
  }

  static SEXP Min(SEXP vec, Rboolean) { return RT::scalar(Get(vec).value); }
  static SEXP Max(SEXP vec, Rboolean) { return RT::scalar(Get(vec).value); }

  // A subset with indices all in bounds is still a compact repetition. Otherwise
  // the out of bound indices are NA and the result is a regular vector.
  static SEXP Extract_subset(SEXP x, SEXP indx, SEXP)
  {
    const int* p = INTEGER(indx);
    R_xlen_t n = XLENGTH(indx);
    R_xlen_t s = Length(x);
    T v = Get(x).value;

    bool out_of_bound = false;
    for (R_xlen_t i = 0 ; i < n && !out_of_bound ; i++)
      out_of_bound = p[i] > s || p[i] < 1;

    if (!out_of_bound)
      return Make(n, v);

    SEXP out = PROTECT(Rf_allocVector(RT::sexptype, n));
    auto* pout = RT::ptr(out);
    for (R_xlen_t i = 0 ; i < n ; i++)
      pout[i] = (p[i] <= s && p[i] > 0) ? RT::to_R(v) : RT::na();
    UNPROTECT(1);
    return out;
  }

  // -------- initialize the altrep class with the methods above
  static void Init(R_altrep_class_t class_t)
  {
    RT::altclass = class_t;

    // altrep
    R_set_altrep_Length_method(class_t, Length);
    R_set_altrep_Inspect_method(class_t, Inspect);
    R_set_altrep_Serialized_state_method(class_t, Serialized_state);
    R_set_altrep_Unserialize_method(class_t, Unserialize);

    // altvec
    R_set_altvec_Dataptr_method(class_t, Dataptr);
    R_set_altvec_Dataptr_or_null_method(class_t, Dataptr_or_null);
    R_set_altvec_Extract_subset_method(class_t, Extract_subset);
  }
};

// The class of the unserialized object is given by the type of the serialized value
SEXP Unserialize(SEXP, SEXP state)
{
  int n = Rf_asInteger(VECTOR_ELT(state, 1));
  SEXP value = VECTOR_ELT(state, 0);

  switch (TYPEOF(value))
  {
    case INTSXP:  return compact_repetition<int>::Make(n, Rf_asInteger(value));
    case REALSXP: return compact_repetition<double>::Make(n, Rf_asReal(value));
    case LGLSXP:  return compact_repetition<bool>::Make(n, Rf_asLogical(value));
    default: throw std::runtime_error("Not supported input SEXP in compact repetition");
  }
}

// Called when the package is loaded (needs Rcpp 0.12.18.3)
// [[Rcpp::init]]
void init_alt_rep(DllInfo* dll)
{
  using int_rep  = compact_repetition<int>;
  using real_rep = compact_repetition<double>;
  using lgl_rep  = compact_repetition<bool>;

  R_altrep_class_t int_class = R_make_altinteger_class("compact repetition (int)", "rlas", dll);
  int_rep::Init(int_class);
  R_set_altinteger_Elt_method(int_class, int_rep::Elt);
  R_set_altinteger_Min_method(int_class, int_rep::Min);
  R_set_altinteger_Max_method(int_class, int_rep::Max);

  R_altrep_class_t real_class = R_make_altreal_class("compact repetition (double)", "rlas", dll);
  real_rep::Init(real_class);
  R_set_altreal_Elt_method(real_class, real_rep::Elt);
  R_set_altreal_Min_method(real_class, real_rep::Min);
  R_set_altreal_Max_method(real_class, real_rep::Max);

  R_altrep_class_t lgl_class = R_make_altlogical_class("compact repetition (bool)", "rlas", dll);
  lgl_rep::Init(lgl_class);
  R_set_altlogical_Elt_method(lgl_class, lgl_rep::Elt);
}

// [[Rcpp::export]]
SEXP R_compact_rep(int n, SEXP v)
{
  switch (TYPEOF(v))
  {
    case INTSXP:  return compact_repetition<int>::Make(n, Rf_asInteger(v));
    case REALSXP: return compact_repetition<double>::Make(n, Rf_asReal(v));
    case LGLSXP:  return compact_repetition<bool>::Make(n, Rf_asLogical(v));
    default: throw std::runtime_error("Not supported input SEXP in compact repetition");
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
SEXP R_altrep_full_class(SEXP x)
{
  if (!ALTREP(x)) return R_NilValue;
  #if R_VERSION >= R_Version(4, 6, 0)
    return R_altrep_class_package(x);
  #else
    return ATTRIB(ALTREP_CLASS(x));
  #endif
}
