context("pnpoly")

test_that("Point in polygon works with convex polygon", {

  # in the polygon

  xseed = runif(1, 10000, 50000)
  yseed = runif(1, 10000, 50000)

  xpoly = c(0, 1, 2, 1, 0, 0) + xseed
  ypoly = c(0, 0, 1, 2, 2, 0) + yseed

  xin = c(0.5, 1.5, 1.999, 0.0001) + xseed
  yin = c(1, 1, 1, 0.0001) + yseed

  inp = sapply(1:length(xin), function(i) rlas:::point_in_polygon(xpoly, ypoly, xin[i], yin[i]))

  expect_true(all(inp))

  # outside the polygon

  xout = c(-0.1, 1.5, 2.0001, -0.0001) + xseed
  yout = c(1, 0.4, 2, 0.0001) + yseed

  inp = sapply(1:length(xout), function(i) rlas:::point_in_polygon(xpoly, ypoly, xout[i], yout[i]))

  expect_true(all(!inp))

  # on the polygon

  xon = c(0.5, 1.5, 1.5, 0.5, 0) + xseed
  yon = c(0, 0.5, 1.5, 2, 1) + yseed

  inp = sapply(1:length(xpoly), function(i) rlas:::point_in_polygon(xpoly, ypoly, xon[i], yon[i]))
  # This is complex and tested later

  # is the polygon

  inp = sapply(1:length(xpoly), function(i) rlas:::point_in_polygon(xpoly, ypoly, xpoly[i], ypoly[i]))
  expect_true(all(inp))
})

test_that("Point in polygon works with concave polygon", {

  # in the polygon

  xseed = runif(1, 10000, 50000)
  yseed = runif(1, 10000, 50000)

  xpoly = c(0, 1, 0.5, 1, 0, 0) + xseed
  ypoly = c(0, 0, 1, 2, 2, 0) + yseed

  xin = c(0.4999, 0.5, 0.75) + xseed
  yin = c(1, 1.5, 0.4999) + yseed

  inp = sapply(1:length(xin), function(i) rlas:::point_in_polygon(xpoly, ypoly, xin[i], yin[i]))

  expect_true(all(inp))

  # outside the polygon

  xout = c(0.50001, 0.8, 0.75) + xseed
  yout = c(1, 1.5, 0.6) + yseed

  inp = sapply(1:length(xout), function(i) rlas:::point_in_polygon(xpoly, ypoly, xout[i], yout[i]))

  expect_true(all(!inp))

  # on the polygon

  xon = c(0.5, 0.75, 0.75) + xseed
  yon = c(0, 0.5, 1.5) + yseed

  inp = sapply(1:length(xon), function(i) rlas:::point_in_polygon(xpoly, ypoly, xon[i], yon[i]))
  # This is complex and tested later

  # is the polygon

  inp = sapply(1:length(xpoly), function(i) rlas:::point_in_polygon(xpoly, ypoly, xpoly[i], ypoly[i]))
  expect_true(all(inp))
})

test_that("Point in polygon works with concave polygon", {

  # in the polygon

  xseed = runif(1, 10000, 50000)
  yseed = runif(1, 10000, 50000)

  xpoly = c(0, 1, 0.5, 1, 0, 0) + xseed
  ypoly = c(0, 0, 1, 2, 2, 0) + yseed

  xin = c(0.4999, 0.5, 0.75) + xseed
  yin = c(1, 1.5, 0.4999) + yseed

  inp = sapply(1:length(xin), function(i) rlas:::point_in_polygon(xpoly, ypoly, xin[i], yin[i]))

  expect_true(all(inp))

  # outside the polygon

  xout = c(0.50001, 0.8, 0.75) + xseed
  yout = c(1, 1.5, 0.6) + yseed

  inp = sapply(1:length(xout), function(i) rlas:::point_in_polygon(xpoly, ypoly, xout[i], yout[i]))

  expect_true(all(!inp))

  # on the polygon

  xon = c(0.5, 0.75, 0.75) + xseed
  yon = c(0, 0.5, 1.5) + yseed

  inp = sapply(1:length(xon), function(i) rlas:::point_in_polygon(xpoly, ypoly, xon[i], yon[i]))
  # This is complex and tested later

  # is the polygon

  inp = sapply(1:length(xpoly), function(i) rlas:::point_in_polygon(xpoly, ypoly, xpoly[i], ypoly[i]))
  expect_true(all(inp))
})

test_that("Point in polygon works if point lies on the polygon", {

  xseed = runif(1, 10000, 50000)
  yseed = runif(1, 10000, 50000)

  xpoly1 = c(0, 1, 0.5, 1, 0, 0) + xseed
  ypoly1 = c(0, 0, 1, 2, 2, 0) + yseed

  xpoly2 = c(0, 1, 0.5, 1, 1.5, 1.5, 0, 0) + xseed
  ypoly2 = c(0, 0, 1, 2, 2, -0.5, -0.5, 0) + yseed

  xon = c(0.5, 0.75, 0.75, 0.8963, 0.58941) + xseed
  yon = c(0, 0.5, 1.5, 2*0.8963, -2*0.58941 + 2) + yseed

  inp1 = sapply(1:length(xon), function(i) rlas:::point_in_polygon(xpoly1, ypoly1, xon[i], yon[i]))
  inp2 = sapply(1:length(xon), function(i) rlas:::point_in_polygon(xpoly2, ypoly2, xon[i], yon[i]))

  expect_equal(inp1, !inp2) # The point is found in only one polygon
})
