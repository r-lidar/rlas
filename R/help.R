#' LASlib filter and transform operation
#'
#' Print the existing LASlib filter and transform commands
#'
#' @return NULL. The function is used for its side effect of printing.
#' @export
#' @rdname LASlibdoc
help_filter = function()
{
  lasfilterusage()
}

#' @export
#' @rdname LASlibdoc
help_transform = function()
{
  lastransformusage()
}