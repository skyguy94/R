read.fwf <- function(file, widths, sep = "", as.is = FALSE,
		     skip = 0, row.names, col.names)
{
    FILE <- tempfile("R.")
    on.exit(unlink(FILE))
    args <- paste("-f", deparse(paste("A", widths, sep = "", collapse = " ")),
                  "-s", deparse(sep), "-o", FILE, file)
    cmd <- paste(file.path(R.home(), "bin", "fwf2table"))
    system(paste("perl", cmd, args))
    read.table(file = FILE, header = FALSE, sep = sep, as.is = as.is,
	       skip = skip, row.names = row.names, col.names = col.names)
}
