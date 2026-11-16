# Reading with several cores must return exactly what a single core returns.

# The example file is too small to be read in parallel: build files of 1 M points
example  <- system.file("extdata", "example.las", package = "rlas")
header   <- read.lasheader(example)
las      <- read.las(example)
las_file <- file.path(tempdir(), "parallel.las")
laz_file <- file.path(tempdir(), "parallel.laz")

big <- las[rep(seq_len(nrow(las)), length.out = 1000000L), ]
big$Intensity <- as.integer(seq_len(nrow(big)) %% 1000L)
write.las(las_file, header, big)
write.las(laz_file, header, big)

# One read with a single core, one read with 4 cores: same data, same order
same <- function(file, filter = "", select = "*")
{
  t0 = Sys.time()
  single <- read.las(file, select = select, filter = filter, ncores = 1L)
  t1 = Sys.time()
  multi  <- read.las(file, select = select, filter = filter, ncores = 0L)
  t2 = Sys.time()
  tsingle = difftime(t1,t0)
  tmulti  = difftime(t2,t1)
  #print(tsingle)
  #print(tmulti)
  expect_true(nrow(single) > 0L)
  expect_identical(multi, single)

  if (tools::file_ext(file) == "laz")
    expect_true(tmulti<tsingle)
}

xr <- range(big$X)
yr <- range(big$Y)
xm <- mean(xr)
ym <- mean(yr)
r  <- diff(xr) / 4

# No filter
same(laz_file)
same(las_file)

# Filters
same(laz_file, filter = "-keep_first")
same(laz_file, filter = "-keep_class 2")
same(laz_file, filter = "-drop_z_below 1")
same(las_file, filter = "-keep_first")
same(las_file, filter = "-keep_class 2")

# A filter that depends on the order of the points
same(laz_file, filter = "-keep_every_nth 7")
same(las_file, filter = "-keep_every_nth 7")

# Several filters
same(laz_file, filter = "-keep_class 2 -keep_every_nth 3")
same(las_file, filter = "-keep_class 2 -keep_every_nth 3")

# Transforms
same(laz_file, filter = "-translate_xyz 1 2 3")
same(laz_file, filter = "-scale_z 2")
same(laz_file, filter = "-set_classification 5")
same(laz_file, filter = "-translate_z 10 -scale_intensity 0.5")
same(las_file, filter = "-translate_xyz 1 2 3")
same(las_file, filter = "-scale_z 2")

# A filter and a transform
same(laz_file, filter = "-keep_class 2 -translate_x 100")
same(laz_file, filter = "-keep_every_nth 5 -translate_z 1")
same(las_file, filter = "-keep_class 2 -translate_x 100")

# Selection of attributes
same(laz_file, select = "xyzi")
same(laz_file, select = "xyzic", filter = "-keep_every_nth 5 -translate_z 1")

# Spatial queries
same(laz_file, filter = sprintf("-inside %f %f %f %f", xr[1], yr[1], xm, ym))
same(laz_file, filter = sprintf("-inside_circle %f %f %f", xm, ym, r))
same(laz_file, filter = sprintf("-inside_circle %f %f %f -keep_every_nth 2", xm, ym, r))
same(las_file, filter = sprintf("-inside %f %f %f %f", xr[1], yr[1], xm, ym))

# Whatever the number of cores
single <- read.las(laz_file, filter = "-keep_class 2 -keep_every_nth 3", ncores = 1L)
expect_identical(read.las(laz_file, filter = "-keep_class 2 -keep_every_nth 3", ncores = 2L), single)
expect_identical(read.las(laz_file, filter = "-keep_class 2 -keep_every_nth 3", ncores = 3L), single)
expect_identical(read.las(laz_file, filter = "-keep_class 2 -keep_every_nth 3", ncores = 0L), single)

unlink(c(las_file, laz_file))

