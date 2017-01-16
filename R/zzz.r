.onUnload <- function (libpath) {
  library.dynam.unload("rlas", libpath)
}
