@rem = '--*-Perl-*--
@echo off
if "%OS%" == "Windows_NT" goto WinNT
perl -x %R_HOME%\bin\Sd2Rd.bat %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
:WinNT
perl -x %R_HOME%\bin\Sd2Rd.bat %*
if NOT "%COMSPEC%" == "%SystemRoot%\system32\cmd.exe" goto endofperl
if %errorlevel% == 9009 echo You do not have Perl in your PATH.
if errorlevel 1 goto script_failed_so_exit_with_non_zero_val 2>nul
goto endofperl
@rem ';
#!perl
#
# ${R_HOME}/bin/Sd2Rd for converting S documentation to Rd format

# Copyright (C) 1997, 1998, 1999, 2000 Kurt Hornik
#
# This document is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# A copy of the GNU General Public License is available via WWW at
# http://www.gnu.org/copyleft/gpl.html.  You can also obtain it by
# writing to the Free Software Foundation, Inc., 59 Temple Place,
# Suite 330, Boston, MA  02111-1307  USA.

use Getopt::Long;

my $revision = ' $Revision: 1.3 $ ';
my $version;
my $name;

$revision =~ / ([\d\.]*) /;
$version = $1;
($name = $0) =~ s|.*/||;

sub version {
  print STDERR <<END;
Sd2Rd $version
Copyright (C) 1997-2000 Kurt Hornik.
This is free software; see the GNU General Public Licence version 2
or later for copying conditions.  There is NO warranty.    
END
  exit 0;
}

sub usage {
  print STDERR <<END;
Usage: Sd2Rd.bat [options] FILE

Convert S documentation in FILE to R documentation format.

Options:
  -h, --help		print short help message and exit
  -v, --version		print version info and exit
  -n			render examples non-executable by wrapping them
			into a \\dontrun{} environment
  -x			interpret all single-quoted names as code names

Email bug reports to <r-bugs\@lists.r-project.org>.
END
  exit 0; 
}

GetOptions(("v|version", "h|help", "n", "x")) || &usage();
&version() if $opt_v;
&usage() if $opt_h;

my $braceLevel = 0;
my $inVerbatim = 0;
my $inSeeAlso = 0;
my $doprint = 1;
my $needArg = 1;
my $needVal = 0;
my $output = "";

while (<>) {
  chop;
  &substitute unless /^\./;
  my @word = split;
  
  if (/^\s*$/) { &output("\n"); }
  if (/^[^.]/) { &output($_); }

  ## Added by BDR 1998/08/27
  if (/^\.\\\"/o) {
    s/^\.\\\"/%/;
    &output($_);
  }
  ## End
  
  if (/^\.AG/) {
    if ($needArg) {
      &section(0, "\\arguments\{");
      $needArg = 0;
    }
    &section(1, "\\item\{$word[1]\}\{");
  }
  if (/^\.CS/) {
    &section(0, "\\usage\{");
    $inVerbatim = 1;
  }
  if (/^\.DN/) { &section(0, "\\description\{"); }
  if (/^\.DT/) { &section(0, "\\details\{"); }
  if (/^\.EX/) {
    if ($opt_n) {
      &section(1, "\\examples\{\\dontrun\{");
    } else {
      &section(0, "\\examples\{");
    }
    $inVerbatim = 1;
  }
  if (/^\.FN/) {
    unless($fun) { $fun = $word[1]; }
    push(@aliases, $word[1]);
  }
  if (/^\.(IP|PP)/) { &output("\n"); }
  if (/^\.KW/) { 
    if ($braceLevel > 0) {
      &section(0, "");
      $braceLevel = 0;
    }
    if ($word[1] =~ /sysdata/) {
      &output("\\keyword\{datasets\}");
    } else {
      &output("\\keyword\{$word[1]\}");
    }
  }
  if (/^\.RC/) {
    if ($needVal) {
      $needVal = 0;	    
      &section(0, "\\value\{\n$output\n");
      $doprint = 1;
    }
    &section(1, "\\item\{$word[1]\}\{");
  }
  if (/^\.RT/) {
    $needVal = 1;
    $doprint = 0;
    $output = "";
  }
  if (/^\.SA/) {
    &section(0, "\\seealso\{");
    $inSeeAlso = 1;
  }
  if (/^\.SE/) { &section(0, "\\section\{Side Effects\}\{"); }
  if (/^\.SH/) {
    if ($word[1] =~ /REFERENCE/) {
      &section(0, "\\references\{");
    } elsif ($word[1] =~ /SOURCE/) {
      &section(0, "\\source\{");
    } elsif ($word[1] =~ /SUMMARY/) {
      &section(0, "\\description\{");
    } elsif (join(" ", @word[1..2]) =~ /DATA DESCRIPTION/) {
      &section(0, "\\format\{");
    } else {
      # This line may be of the form .SH "A B C"
      ($tmp = join(" ", @word[1..$#word])) =~ s/\"(.*)\"/$1/;
      &section(0, "\\section\{$tmp\}\{");
    }
  }
  if (/^\.sp/) { output("\n"); }
  if (/^\.TL/) {
    print("\\name\{$fun\}\n");
    print("\\alias\{", join("\}\n\\alias\{", @aliases), "\}\n");
    &section(0, "\\title\{");
    $inVerbatim = 1;
  }
  if (/^\.WR/) {
    &section(0, "");
    print("% Converted by $name version $version.\n");
  }
  if (/^\.AO/) {
    output("Arguments for function \\code\{$word[1]()\} can also be");
    output("supplied to this function.");
  }
  if (/^\.GE/) {
    output("This is a generic function.");
    output("Functions with names beginning in \\code\{$fun.\} will be");
    output("methods for this function.");
  }
  if (/^\.GR/) {
    output("Graphical parameters (see \\code\{\\link\{par\}\}) may also");
    output("be supplied as arguments to this function.");
  }
  if (/^\.ME/) {
    output("This function is a method for the generic function");
    output("\\code\{$word[1]()\} for class \\code\{$word[2]\}.");
    output("It can be invoked by calling \\code\{$word[1](x)\} for an");
    output("object \\code\{x\} of the appropriate class, or directly by");
    output("calling \\code\{$word[1].$word[2](x)\} regardless of the");
    output("class of the object.");
  }
  if (/^\.NA/) { output("Missing values (\\code\{NA\}s) are allowed."); }
  if (/^\.Tl/) {
    output("In addition, the high-level graphics control arguments");
    output("described under \\code\{\\link\{par\}\} and the arguments to");
    output("\\code\{\\link\{title\}\} may be supplied to this function.");
  }
  ## Added by BDR 1998/06/20
  if (/^\.ul/) {
    $_ = <>;
    &substitute;
    chomp;
    output("\\emph{".$_."\}");
  }
  ## End
}

sub substitute {
  if (!$inVerbatim) {
    s/\{/\\\{/g;
    s/\}/\\\}/g;
#    s/&/\\&/g; removed BDR 2000/02/10
    ## Added by BDR 1998/06/20
    s/\\\(aa/'/g;		# extra ' for highlight matching
    s/\\\(em/--/g;		# em dash
    s/\\\(tm/ (TM) /g;		# Trademark
    s/\\\(mu/ x /g;		# multiply sign
    s/\\\(\*a/\alpha/g;		# greek
    s/\\\(\*b/\beta/g;
    s/\\\(\*e/\epsilon/g;
    s/\\\(\*l/\lambda/g;
    s/\\\(\*m/\mu/g;
    s/\\\(\*p/\pi/g;
    s/\\\(\*s/\sigma/g;
    ## End
  }
  if ($inVerbatim) {
    s/\.\.\./\\dots/g;
  } else {
    s/\.\.\./\\dots\{\}/g;
  }
  s/\\fB/\\bold\{/g;
  s/\\fR/\}/g;
  ## Added by BDR 1998/06/20
  s/\\fI/\\emph\{/g;
  s/\\fP/\}/g;
  ## End
  s/\%/\\%/g;
  s/\\\.(.*)$/# $1)/g;
  if ($inSeeAlso) {
    if ($opt_x) {
      s/\`?([\.\w]*\w+)\'?/\\code{\\link{$1}}/g;
    } else {
      s/\`([^\']*)\'/\\code{\\link{$1}}/g;
    }
  } elsif (!$inVerbatim) {
    s/\`([^\']*)\'/\\code{$1}/g;
  }
}

sub section {
  my($level, $text) = @_;
  $n = $braceLevel - $level;
  print "\}" x $n, "\n" if ($n > 0);
  if ($needVal) {
    print("\\value\{\n$output\n\}\n");
    $needVal = 0;
  }
  print("$text\n") if $text;    
  $braceLevel = $level + 1;
  $inVerbatim = 0;
  $inSeeAlso = 0;
  $doprint = 1;    
}

sub paragraph {
  my($name) = @_;
  &output("\n\\bold\{$name.\} ");
}
    
sub output {
  my($text) = @_;
  if ($doprint) {
    print("$text\n");
  } elsif ($output) {
    $output .= "\n$text";
  } else {
    $output = $text;
  }
}

### Local Variables: ***
### mode: perl ***
### perl-indent-level: 2 ***
### End: ***
__END__
:endofperl
