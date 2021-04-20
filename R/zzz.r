# nocov start
.onUnload <- function (libpath) {
  library.dynam.unload("rlas", libpath)
}
# nocov end