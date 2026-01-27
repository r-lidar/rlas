#' ALTREP utilities
#'
#' Test if an a vector is compressed using the ALTREP framework
#'
#' @param x an R object
#'
#' @examples
#' lazfile <- system.file("extdata", "example.las", package = "rlas")
#' las <- read.las(lazfile)
#' is_compressed(las)
#'
#' # The difference is more substantial on bigger point clouds (~30%)
#' object.size(las)
#' true_size(las)
#'
#' @name compression
#' @rdname compression
NULL

#' @rdname compression
#' @export
is_compressed <- function(x)
{
  if (is.data.frame(x) | is.list(x))
    return(sapply(x, is_compact))

  return(!R_is_materialized(x))
}

#' @rdname compression
#' @export
true_size <- function(x)
{
  size <- 0
  gz <- is_compressed(x)

  if (length(gz) == 1)
  {
    size <- if (isTRUE(gz)) 644 else utils::object.size(x)
  }
  else
  {
    for (i in seq_along(gz))
    {
      if (isTRUE(gz[i]))
        size <- size + 644L
      else
        size <- size + utils::object.size(x[[i]])
    }
  }

  class(size) <- 'object_size'
  return(size)
}

is_compact <- function(x)
{
  altrep <- R_altrep_full_class(x)
  cat(">>>>>>>", altrep[[2]], "\n")
  if (is.null(altrep)) return(FALSE)
  if (altrep[[2]] == "rlas") {
    if (R_is_materialized(x)) return(FALSE)
    return(TRUE)
  }
  return(FALSE)
}

