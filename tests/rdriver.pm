
=head1 NAME

driver:  Regression driver.

=head1 SYNOPSIS

The regression driver is a framework for a regression suite.  Refer
to the full man page for more information.

=head1 OPTIONS

Command-line options:

=over 8

=item B<--list, --l>

List the tests in this regression.

=item B<--tests=range, --t=range>

Specify what tests to run.  Should be a number or a range, where the numbers
refer to indices of the test array (1 is the first test).

=item B<--exclude=range, --x=range>

Exclude tests.  Should be a number or a range.  Range format is the same as for
the "tests" parameter.

=item B<--string=str, --s=str>

Run any tests which contain string I<str>.  This is a simple substring search,
not a regular expression search.

=item B<--regex=re, --r=re>

Run any tests which match regular expression I<re>.

=item B<--keepoutput, --ko>

Do not delete temporary files. 

=item B<--debug, --d>

Enable debug messages.

=item B<--seed>

Specify a seed value.

=back

The valid range forms are:

      x-y:  Run tests x through y.  Example:  --tests=3-5

       -y:  Run tests from start, up to and including y.

       x-:  Run tests x through the last one.

Note:  Multiple parms are allowed and are concatenated together.

=head1 DESCRIPTION

Library usage:

use lib "..";
use rdriver;

doTest(@tests_list);

B<doTest>, the main entry function, takes an array reference as a parameter.

=head2 Hash Keys

Each entry in the supplied array should be a hash.  These are the allowed keys:

=over 8

=item B<cmd>

Command-line to execute.  If the token &seed is found, it will be replaced with
a numerical seed.  This value will either be the value specified by the seed
command-line parameter, or the current result of time().

=item B<cmts>

Preserve comments in diff.  Default is 0 (do not preserve).

=item B<stderr>

If 1, stderr and stdout are both captured.  Otherwise, only stdout is captured.
The default is 0.

=item B<diff>

Diff the output with the file specified.

=item B<dpfx>

Only diff the output that starts with the given prefix.  This may be a regular
expression.

=item B<checker>

If present, this should be a function reference.  The function is called with
the test output as its argument and should die if an error occurs.

=item B<fail>

If this is 1 then the cmd run is expected to have a non-zero return value. The
test will fail if 0 is returned.  The checking commands will still be executed
if they are present.

=item B<temps>

Specify temporary files.  These will be unlinked after the test is complete,
unless the --keepoutput option is set on the command-line.

=over

=head2 Utility Functions

The library also contains a number of utility functions.  These are listed
below.

=back 8

=item file_rdiff(<filename>,<array ref of regular expressions>)

This is a simple function for checking a file.  The second argument is a
reference to an array of strings.  Each string is considered to be a regular
expression.  The function must be able to successfully match lines in the file
against the array elements, proceeding to the end of the array, or else it fails
and calls die.

=item str_rdiff(<input string>,<array ref of regular expressions>)

Same as file_rdiff, except that we take the input, split on newlines, then try
and match against the regular expressions.

=item dprint(...)

Same functionality as print if the --debug option is specified on the
command-line.  Otherwise, nothing is printed.

=item dprintf(...)

Same functionality as printf if the --debug option is specified on the
command-line.  Otherwise, nothing is printed.

=back

=cut
    
package rdriver;

require Exporter;

our @ISA = ("Exporter");
our @EXPORT = qw( doTest file_rdiff str_rdiff dprint dprintf );

$diff = "diff -bi";
$tmpfile = "cmp.out";

$debug = 0;

# These are global because Perl's closures are *broken*!!!!.
$seed = 0;
$cmd = "";
$fails = 0;
$keepoutput = 0;

sub error {
  print shift;
  print "  Executed command:  '$cmd'\n";
  ++$fails;
  die;
};

use strict;

use vars qw(@Tests $diff $tmpfile $cmd $seed $fails $keepoutput $debug);

# Main test code:  For each test, execute it, then scan the output for
# the tags we look for.  After that, check everything.
sub doTest($) {
  # get_run_list actually returns a hash, where the keys are
  # indices and the values are the items specified in the 
  # original test array.
  my $tests = get_run_list(shift);

  print "\n";
  # We sort the keys of the hash here, numerically, so that
  # the user sees the tests execute in the expected order.
 TEST: for my $iter (sort { $a <=> $b} (keys %{$tests})) {
    print " Test $iter...\n";
    my $t = $$tests{$iter};
    $cmd = $t->{cmd};
    $cmd =~ s/&seed/$seed/g;
    if ($t->{stderr}) {
      $cmd .= " 2>&1";
    } else {
      $cmd .= " 2>/dev/null";
    }
    my $output = `$cmd`;

    my $failokay = ($t->{fail});
    #print "Output:\n\n$output\n\n";
    eval {
      if (($? >> 8)) {
		if (!($failokay)) {
		  error (" Test failed and was not expected to.  Return code was $?. Output is:\n\n$output\n");
		} else {
		  print "  ...expected fail found.\n";
		}
      } elsif ($failokay) {
		error (" Test did not fail but was expected to.  Return code was $?. Output is:\n\n$output\n");
      }

      if ( $t->{diff} ) {
		error() if (!doDiff($output,$t->{diff},$t->{dpfx},$t->{cmts}));
      }
      if ($t->{checker}) {
		# Call the check function.
		my $checker = $t->{checker};
		eval { &$checker($output) };
		if ($@) {
		  error ("  Failed checker:  $@\n");
		  next TEST;
		}
		print "  ...checker test passed.\n";
      }
    };
    # Remove listed temporary files unless overridden by the user.
    if (!$keepoutput) {
      for (@{ $t->{temps} }) {
		unlink $_;
      }
    }
  }

  if (!$fails) {
    print "\nRegression SUCCEEDED.\n";
  } else {
    print "\nRegression FAILED:  $fails fails found.\n";
    exit 1;
  }
}

# Perform the diff between the output from the test and the
# regression file.
#
# arg0:  Test output as a single string.
# arg1:  Name of regression file.
# arg2:  (optional).  Diff prefix.  If specified, we extract
#        only the output starting with the given prefix, then 
#        do the diff.
sub doDiff {
  my $output = shift;
  my $rfile = shift;
  my $dpfx = shift;
  my $cmts = shift;

  unlink $tmpfile;
  open OUT,">$tmpfile" or die "Could not open $tmpfile for writing.";
  if ($dpfx) {
    # Prefix specified
    my @lines = split '\n',$output;
    for my $l (@lines) {
      if ($l =~ $dpfx) {
	print OUT $l,"\n";
      }
    }
  } elsif (!$cmts) {
    # We do not want to preserve comments, so strip all blank lines and
    # comment lines.
    my @lines = split '\n',$output;
    for my $l (@lines) {
      next if ($l =~ /^#/ || $l =~ /^\s*$/);
      print OUT $l,"\n";
    }
  } else {
    # No prefix specified, so just print output to the file.
    print OUT $output;
  }
  close OUT;

  my $dout = `$diff $tmpfile $rfile`;
  if ($? > 8) {
    print " Error!  Output differs from regression file '$rfile'.\n";
    print "Diff output was:\n";
    print " <----- Test output | Regression file ----->\n";
    print $dout;
    return 0;
  } else {
    unlink $tmpfile unless $keepoutput;
    print "  ...diff check passed.\n";
    return 1;
  }
}

use Getopt::Long;
use Pod::Usage;

# This takes the original test array and returns a hash reference
# where the keys are test indices and the values are the elements of
# the original test.  The default is to return all tests; we modify that
# by the include and exclude specifiers.
sub get_run_list {
  my $tests = shift;
  my $size = scalar( @$tests );
  my (%includes, %excludes);
  my (@ilist,@xlist,@slist,@rlist);
  my ($help,$man,$list);

  # Process the command-line options, generating up a 
  # list of ranges to include and a list of ranges to exclude.
  if (!&GetOptions
      (
       "l|list"        => \$list,
       "h|help"        => \$help,
       "m|man"         => \$man,
       "d|debug"       => \$debug,
       "t|tests=s"     => \@ilist,
       "x|excludes=s"  => \@xlist,
       "s|string=s"    => \@slist,
       "r|regex=s",    => \@rlist,
       "ko|keepoutput" => \$keepoutput,
       "seed=i"        => \$seed,
      )) {
    printhelp(1,1);
  }

  # Print help if requested to do so.
  printhelp(0,1) if $help;

  printhelp(0,2) if $man;

  # List tests, if requested to do so.
  if ($list) {
    listtests($tests);
  }

  if (!$seed) {
    $seed = time();
  }

  # This converts a list of range expressions (arg1)
  # into a hash (stored into hash reference arg0), where
  # the keys are indices of tests.
  sub get_indices {
    my $h = shift;
    my $pl = shift;

    for my $i (@$pl) {
      if ($i =~ /^([0-9]+)$/) {
	$$h{$1-1} = 1;
      } elsif ($i =~ /^([0-9]+)-$/) {
	for my $c ($1..$size) {
	  $$h{$c-1} = 1;
	}
      } elsif ($i =~ /^-([0-9]+)$/) {
	for my $c (1..$1) {
	  $$h{$c-1} = 1;
	}
      } elsif ($i =~ /^([0-9]+)-([0-9]+)$/) {
	for my $c ($1..$2) {
	  $$h{$c-1} = 1;
	}
      }
    }
  }

  # Now convert the range specifications into hashes, where the
  # keys of each hash is an index of a test to include (or exclude).
  get_indices(\%includes,\@ilist);
  get_indices(\%excludes,\@xlist);

  # If the user didn't specify any includes, use all of them.
  if (!scalar(keys %includes)) {
    for my $i (0..$size-1) {
      $includes{$i} = 1;
    }
  }

  # Now remove all excluded indices from the include list.
  for my $i (keys %excludes) {
    delete $includes{$i};
  }

  # Now create a new test list with only the included indices.
  # If substrings were specified, include each item only if
  # the command contains a listed substring.
  my %newtests;
  for my $i (0..$size-1) {
    if ($includes{$i}) {
      my $test = $$tests[$i];
      if (@slist) {
	# Have substrings, so do search.
	for my $s (@slist) {
	  if ( index($test->{cmd},$s) >= 0) {
	    $newtests{$i+1} = $test;
	    last;
	  }
	}
      } elsif (@rlist) {
	# Have regular expressions, so do search.
	for my $s (@rlist) {
	  if ( $test->{cmd} =~ /$s/ ) {
	    $newtests{$i+1} = $test;
	    last;
	  }
	}
      } else {
	# No substrings, so include item.
	$newtests{$i+1} = $test;
      }
    }
  }

  return \%newtests;
}

sub listtests() {
  my $tests = shift;

  print "\nTests in this regression (commands to be executed):\n\n";
  my $i = 1;
  for my $t (@$tests) {
    print "  $i.  $t->{cmd}\n";
    ++$i;
  }
  print "\n";
  exit 0;
}

sub printhelp() {
  my ($eval,$verbose) = (shift,shift);
  my @pl = split ('/',$0);
  pop @pl;
  my $path = join('/',@pl) . "/../rdriver.pm";
  pod2usage(-exitval => $eval,-input=>$path,-verbose=>$verbose);
  exit 0;
}

sub file_rdiff {
  my $f = shift;
  my $r = shift;

  open IN,$f or die "Could not open $f for comparing.";

  my $i = 0;
  my $max = scalar(@$r);
  while (<IN>) {
    ++$i if ( $i < $max && $_ =~ $r->[$i] )
  }
  if ($i != $max) {
    print "  Failed to match '$r->[$i]' in file '$f'.\n";
    die;
  }
}

sub str_rdiff {
  my @lines = split /\n/,shift;
  my $r = shift;

  my $i = 0;
  my $max = scalar(@$r);
  for (@lines) {
    ++$i if ( $i < $max && $_ =~ $r->[$i] )
  }
  if ($i != $max) {
    print "  Failed to match '$r->[$i]'.\n";
    die;
  }
}

# This is a debug print function:  If debug is enabled, the output will show up.
sub dprint {
    if ($debug) {
        print @_;
    }
}

# Equivalent of above for printf.
sub dprintf {
    if ($debug) {
        printf @_;
    }
}

1;
