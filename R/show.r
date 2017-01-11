#' @export
print.LASheader =  function(x, ...)
{
  cat("File signature:          ", x$`File Signature`, "\n")
  cat("File source ID:          ", x$`File Source ID`, "\n")
  cat("Global encoding:         ", x$`Global Encoding`, "\n")
  cat("Project ID - GUID:       ", x$`Project ID - GUID`, "\n")
  cat("Version:                 ", x$`Version Major`, ".", x$`Version Minor`, "\n")
  cat("System identifier:       ", x$`System Identifier`, "\n")
  cat("Generating software:     ", x$`Generating Software`, "\n")
  cat("File creation d/y:       ", x$`File Creation Day of Year`, "/", x$`File Creation Year`, "\n")
  cat("header size:             ", x$`Header Size`, "\n")
  cat("Offset to point data:    ", x$`Offset to point data`, "\n")
  cat("Num. var. length record: ", x$`Number of variable length records`, "\n")
  cat("Point data format:       ", x$`Point Data Format ID`, "\n")
  cat("Point data record length:", x$`Point Data Record Length`, "\n")
  cat("Num. of point records:   ", x$`Number of point records`, "\n")
  cat("Num. of points by return:", x$`Number of 1st return`, x$`Number of 2nd return`, x$`Number of 3rd return`, x$`Number of 4th return`, x$`Number of 5th return`, "\n")
  cat("Scale factor X Y Z:      ", x$`X scale factor`, x$`Y scale factor`, x$`Z scale factor`, "\n")
  cat("Offset X Y Z:            ", x$`X offset`, x$`Y offset`, x$`Z offset`, "\n")
  cat("min X Y Z:               ", x$`Min X`, x$`Min Y`, x$`Min Z`, "\n")
  cat("max X Y Z:               ", x$`Max X`, x$`Max Y`, x$`Max Z`, "\n")

  n = length(x$`Variable Length Records`)

  if(n > 0 )
  {
    cat("Variable length records: \n")

    for(i in 1:n)
    {
      vlr = x$`Variable Length Records`[[i]]

      cat("   Variable length records ", i, "/", n, "\n", sep ="")
      cat("       Reserved:            ", vlr$reserved, "\n")
      cat("       User ID:             ", vlr$`user ID`, "\n")
      cat("       record ID:           ", vlr$`record ID`, "\n")
      cat("       Length after heaader:", vlr$`length after header`, "\n")
      cat("       Description:         ", vlr$description, "\n")

      if(vlr$`record ID` == 34735)
      {
        cat("       Tags:\n")
        lapply(vlr[[6]], function(xx)
        {
          cat("          Key", xx$key, "tiff_tag_location", xx$`tiff tag location`, "count", xx$count, "value offset", xx$`value offset`, "\n")
        })
      }
      else if(vlr$`record ID` == 34736)
      {
          cat("       data:                ", vlr[[6]], "\n")
      }
      else if(vlr$`record ID` == 34737)
      {
          cat("       data:                ", vlr[[6]], "\n")
      }
    }
  }
  else
  {
      cat("Variable length records:  void\n")
  }

  return(invisible())
}
