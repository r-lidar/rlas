alt_rep <- function(x, time) rlas:::R_compact_rep(time, x)
is_altrep <- rlas:::R_is_altrep
is_materialized <- rlas:::R_is_materialized

int <- alt_rep(2L, 10L)
dbl <- alt_rep(2.1, 10L)
lgl <- alt_rep(TRUE, 10L)

expect_true(is.integer(int))
expect_true(is.double(dbl))
expect_true(is.logical(lgl))

expect_equal(int[1], 2L)
expect_equal(dbl[1], 2.1)
expect_equal(lgl[1], TRUE)

expect_equal(length(int), 10)
expect_equal(length(dbl), 10)
expect_equal(length(lgl), 10)

expect_true(is_altrep(int))
expect_true(is_altrep(dbl))
expect_true(is_altrep(lgl))

expect_false(is_materialized(int))
expect_false(is_materialized(dbl))
expect_false(is_materialized(lgl))

isub = int[1:5]
dsub = dbl[1:5]
lsub = lgl[1:5]

expect_equal(isub[1], 2L)
expect_equal(dsub[1], 2.1)
expect_equal(lsub[1], TRUE)

expect_equal(length(isub), 5)
expect_equal(length(dsub), 5)
expect_equal(length(lsub), 5)

expect_true(is_altrep(isub))
expect_true(is_altrep(dsub))
expect_true(is_altrep(lsub))

expect_false(is_materialized(isub))
expect_false(is_materialized(dsub))
expect_false(is_materialized(lsub))


isub = int[10:14]
dsub = dbl[10:14]
lsub = lgl[10:14]

expect_equal(isub[2], NA_integer_)
expect_equal(dsub[2], NA_real_)
expect_equal(lsub[2], NA)

expect_equal(length(isub), 5)
expect_equal(length(dsub), 5)
expect_equal(length(lsub), 5)

expect_false(is_altrep(isub))
expect_false(is_altrep(dsub))
expect_false(is_altrep(lsub))

int_ = int[]
dbl_ = dbl[]
lgl_ = lgl[]

expect_true(is_altrep(int))
expect_true(is_altrep(dbl))
expect_true(is_altrep(lgl))

expect_false(is_altrep(int_))
expect_false(is_altrep(dbl_))
expect_false(is_altrep(lgl_))
