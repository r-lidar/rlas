alt_rep <- function(x, time) rlas:::R_compact_rep(time, x)
is_altrep <- rlas:::R_is_altrep
is_materialized <- rlas:::R_is_materialized

int <- alt_rep(2L, 10L)
dbl <- alt_rep(2.1, 10L)
lgl <- alt_rep(TRUE, 10L)

sint <- serialize(int, NULL)
int <- unserialize(sint)

sdbl <- serialize(dbl, NULL)
dbl <- unserialize(sdbl)

slgl <- serialize(lgl, NULL)
lgl <- unserialize(slgl)

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
