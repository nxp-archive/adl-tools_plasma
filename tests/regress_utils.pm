##
## Place regression utility functions specific to Plasma in this file.
##

package regress_utils;

require Exporter;

our @ISA = ("Exporter"); 
our @EXPORT = qw( $src check_channel check_qchannel
		  check_clocked_channel check_spawn 
		  check_sequence check_time_seq check_times file_diff filter_errors is_cygwin );

use FindBin;
use lib $FindBin::RealBin;
use lib "$FindBin::RealBin/../share";
use rdriver;
use Data::Dumper;

# This imports variables that Automake defines in the makefile.  For
# consistancy, it defines them in the regression even if they're not set
# externally. If the variable is not found in the environment, then we put it
# there so that any child-processes will also see a consistent view between
# running the regression manually and being invoked by make.
$src = $ENV{srcdir};

$ENV{srcdir} = $src = "." if (!$src);

sub is_cygwin {
  if ( -e "/proc/version" ) {
	if (system("grep -q '^CYGWIN' /proc/version") == 0) {
	  return 1;
	}
  }
  return 0;
}

##
## Place generic helper functions here.
## <HELPERS>
##

# Filters out leading paths from compiler error messages, then does a file
# comparison against a reference file.
# arg0:  Output to write to a temporary file.
# arg1:  Temporary file name.
# arg2:  Reference file.
sub filter_errors {
  my ($data,$out,$regress) = @_;
  open OUT,">$out" or die "Could not open $out";
  $data =~ s#(\S+/)+##g;
  print OUT $data;
  close OUT;

  file_diff ($out,$regress);
}


# Runs diff on two files.
# arg0:  Generated file- will be erased if diff finds no differences.
# arg1:  Reference file.
sub file_diff {
  my ($gen,$exp,$nodel) = @_;
  dprint ("Comparing $gen (found) vs. $exp (expected)\n");
  if (system "diff -bi $gen $exp") {
	print "Differences were found.\n",
	  " <---- Generated $gen | Reference $exp ---->\n";
	die;
  }
  unlink $gen unless $keepoutput;
}

# Make sure that all expected integers are received.
sub check_channel {
  my ($input,$regex,$odata,$expNumFinish,$expstr) = (shift,shift,shift,shift,shift);
  # Store expected expressions in a hash.
  my %exphash;
  if ($expstr) {
    @exphash{@$expstr} = (0) x @$expstr;
  }
  # Convert array of arrays into an array of hashes.
  my @data;
  for my $i ( @{ $odata } ) {
    my %hash;
    @hash{@$i} = (1) x @$i;
    push @data,\%hash;
  }
  dprint "Expected data:  ",Dumper(\@data),"\n";
  my @lines = split /\n/,$input;
  my $max = 10;
  my $numFinish = 0;
  my $done = 0;
  dprint "Regex:  $regex\n";
  for (@lines) {
    dprint "Line:  $_\n";
    if ( /$regex/ ) {
      my ($chan,$val) = (eval $1,eval $2);
      dprint "Read $val from channel $chan.\n";
      if ( exists $data[$chan]->{$val} ) {
	delete $data[$chan]->{$val};
      } else {
	print "Data out of bound for channel $chan:  Got $val, expected data left:  ",
	  join(',',keys(%{ $data[$chan] })),"\n";
	die;
      }
    } elsif ( /(Producer|Consumer) done./ ) {
      ++$numFinish;
    } elsif ( /Done\./ ) {
      ++$done;
    }
    for my $k (keys %exphash ) {
      if ( /$k/ ) {
	++$exphash{$k};
      }
    }
  }
  if ($expNumFinish) {
    if ($numFinish != $expNumFinish) {
      print "Expected $expNumFinish threads to finish, found only $numFinish.\n";
      die;
    }
  }
  if (!$done) {
    die "Did not find main-thread finish message.\n";
  }
  for my $i (0..$#data) {
    if (keys(%{$data[$i]})) {
      print "Not all data was found for channel $i:  ",join(',',keys(%{ $data[$i] })),"\n";
      die;
    }
  }
  for my $i (keys %exphash) {
    if (!$exphash{$i}) {
      print "Expected to find an occurrance of \"$i\" but did not.\n";
      die;
    }
  }
}

# Make sure that all expected integers are received.
sub check_qchannel {
  my ($input,$regex,$data,$expNumFinish,$expregex,$expmax) = (shift,shift,shift,shift,shift,shift);

  my %alldata = ();
  for my $i (@$data) {
    for my $j (@$i) {
      $alldata{$j} = 1;
    }
  }

  my @lines = split /\n/,$input;
  my $max = 10;
  my $numFinish = 0;
  my $exprcount = 0;
  my $done = 0;
  dprint "Regex:  $regex\n";
  dprint "Check regex:  $expregex\n";
  dprint "Expected data:  ",join(' ',keys %alldata),"\n";
  for (@lines) {
    dprint "Line:  $_\n";
    if ( /$regex/ ) {
      my $val = eval $1;
      dprint "Read $val from channel.\n";
      if (!(exists $alldata{$val})) {
	print "Found unexpected data value $val.\n";
	die;
      }
      delete $alldata{$val};
    } elsif ( /(Producer|Consumer) done./ ) {
      ++$numFinish;
    } elsif ( /Done\./ ) {
      ++$done;
    } elsif ( $expregex && /$expregex/ ) {
      ++$exprcount;
      if ($1 > $expmax) {
	print "Exceeeded maximum check count of $expmax.\n";
	die;
      }
    }
  }
  if ($expNumFinish) {
    if ($numFinish != $expNumFinish) {
      print "Expected $expNumFinish threads to finish, found only $numFinish.\n";
      die;
    }
  }
  if (scalar(keys %alldata)) {
    die "Did not find all required data:  ",join(' ',keys %alldata),"\n";
  }
  if ($expregex && !$exprcount) {
    die "Found no occurrance of required check expression.\n";
  }
  if (!$done) {
    die "Did not find main-thread finish message.\n";
  }
}

# Similar to check_channel, but expects there to be a time value for each data message.
# The time value must be monotonically increasing and have a period equal to the supplied value.
sub check_clocked_channel {
  my ($input,$regex,$odata,$period,$expNumFinish,$expstr) = (shift,shift,shift,shift,shift,shift);

  # Store expected expressions in a hash.
  my %exphash;
  if ($expstr) {
    @exphash{@$expstr} = (0) x @$expstr;
  }
  # Convert array of arrays into an array of hashes.
  my @data;
  for my $i ( @{ $odata } ) {
    my %hash;
    @hash{@$i} = (1) x @$i;
    push @data,\%hash;
  }
  # Each data line has a time entry that must increase by the specified amount.
  my @times = (-1) x @{ $odata };
  dprint "Expected data:  ",Dumper(\@data),"\n";
  my @lines = split /\n/,$input;
  my $max = 10;
  my $numFinish = 0;
  my $done = 0;
  dprint "Regex:  $regex\n";
  for (@lines) {
    dprint "Line:  $_\n";
    if ( /$regex/ ) {
      my ($chan,$time,$val) = (eval $1,eval $2,eval $3);
      dprint "Read $val from channel $chan at time $time.\n";
      if ( exists $data[$chan]->{$val} ) {
	delete $data[$chan]->{$val};
	if ($period && ($time % $period)) {
	  print "Bad time value of $time for channel $chan:  Must have a period of $period.\n";
	  die;
	}
	if ($times[$chan] < 0) {
	  # First time through- save value.
	  $times[$chan] = $time;
	} else {
	  # Check value.
	  if ($time <= $times[$chan]) {
	    print "Bad time value of $time for channel $chan:  Expected value greater than $times[$chan].\n";
	    die;
	  }
	}
      } else {
	print "Data out of bound for channel $chan:  Got $val, expected data left:  ",
	  join(',',keys(%{ $data[$chan] })),"\n";
	die;
      }
    } elsif ( /(Producer|Consumer) done./ ) {
      ++$numFinish;
    } elsif ( /Done\./ ) {
      ++$done;
    }
    for my $k (keys %exphash ) {
      if ( /$k/ ) {
	++$exphash{$k};
      }
    }
  }
  if ($expNumFinish) {
    if ($numFinish != $expNumFinish) {
      print "Expected $expNumFinish threads to finish, found only $numFinish.\n";
      die;
    }
  }
  if (!$done) {
    die "Did not find main-thread finish message.\n";
  }
  for my $i (0..$#data) {
    if (keys(%{$data[$i]})) {
      print "Not all data was found for channel $i:  ",join(',',keys(%{ $data[$i] })),"\n";
      die;
    }
  }
  for my $i (keys %exphash) {
    if (!$exphash{$i}) {
      print "Expected to find an occurrance of \"$i\" but did not.\n";
      die;
    }
  }
}


# Makes sure expected results are found.
sub check_spawn {
  my @lines = split /\n/,shift;
  my @parms = @_;
  my $expcount = scalar(@parms);
  my $ix = 0;
  for (@lines) {
    if ( /^Result is:/ ) {
      my @res = ( /(\d+(?:\.\d+)?)/g );
      my $exp = $parms[$ix];
      for my $i (0..scalar(@{$exp})-1) {
	if ($res[$i] != $exp->[$i]) {
	  die "Mismatch on result value $i (Found $res[$i], expected $exp->[$i]).\n";
	}
      }
      ++$ix;
    }
  }
  if ($ix != $expcount) {
    die "Expected $expcount result lines, found $ix.\n";
  }
}

# Thread execution sequence checker.
sub check_sequence {
  my ($input,$seq,$count) = (shift,shift,shift);
  my @lines = split /\n/,$input;
  
  # In addition to counting the number of occurrances from
  # the count array, we make sure that each key has a corresponding
  # done line in the input.
  my %done;
  @done{keys(%$count)} = (1) x keys(%$count);

  my $final = 0;

  my $iseq;
  for (@lines) {
    if ( /In block (\d+\.?\d*)/ ) {
      my $b = eval $1;
      $iseq .= "$b ";
      --($count->{$b});
    } elsif ( /Done with block (\d+\.?\d*)/ ) {
      my $b = eval $1;
      --$done{$b};
    } elsif (/Done\./) {
      ++$final;
    }
  }
  
  if ($seq) {
    if ( $iseq !~ $seq) {
      print "Error:  Bad sequence.  Expected '$seq', found '$iseq'.\n";
      die;
    }
  }

  for my $i (keys(%done)) {
    if ($done{$i}) {
      print "Error:  Did not find a done message for block $i.\n";
      die;
    }
  }

  for my $i (keys(%$count)) {
    if ($count->{$i}) {
      print "Error:  Did not find expected number of messages from block $i:  Expected $count->{$i} more.\n";
      die;
    }
  }

  if (!$final) {
    die "Did not find main-thread finish message.\n";
  }

}

# This takes a hash, where keys are IDs and values are array
# refs to a sequence of time numbers.  We make sure that we find
# the expected sequences.
# arg0:  Regex string.  Must match the id, then the time.
# arg1:  Hash.  Keys are ids, values are array refs of time sequences.
sub check_time_seq {
  my ($regex,$input,$seq) = (shift,shift,shift);
  my @lines = split /\n/,$input; 

  for (@lines) {
    if ( /$regex/ ) {
      my ($id,$t) = (eval $1,eval $2);
      my $ts = $seq->{$id};
      if (!$ts) {
		die "Unknown id found:  $id\n";
      }
      if (! @$ts ) {
		die "Extra time value ($t) found for id $id.\n";
      } elsif ($ts->[0] == $t) {
		shift @$ts;
      } else {
		die "Bad time value found for id $id:  Found $t, expected $ts->[0].\n";
      }
    }
  }
  for my $id (keys %$seq) {
    my $ts = $seq->{$id};
    if ( @$ts ) {
      die "Did not find time $ts->[0] for id $id.\n";
    }
  }
}

# Checks that the ids specified (keys of hash ref parm arg1) have
# either the expected time difference between start and finish or
# the expected start and stop times.  If the hash value is an integer,
# it represents a time difference.  If it's an array, then it should
# contain a start and stop value.
sub check_times {
  my ($input,$diffs) = (shift,shift);
  my @lines = split /\n/,$input; 

  my %data;
  for (@lines) {
    if ( /Block (\d+) start:\s+(\d+)/ ) {
      my ($id,$s) = (eval $1,eval $2);
      if ($data{$id}) {
	die "Id $id start encountered multiple times.\n";
      }
      $data{$id} = [$s];
    } elsif ( /Block (\d+) done:\s+(\d+)/ ) {
      my ($id,$d) = (eval $1,eval $2);
      if (!exists $data{$id}) {
	die "Id $id start not found.\n";
      }
      $data{$id}->[1] = $d;
    }
  }
  for my $id (keys %$diffs) {
    if (ref($diffs->{$id}) eq "ARRAY") {
      my ($exps,$expf,$fnds,$fndf) = ($diffs->{$id}->[0],$diffs->{$id}->[1],
				      $data{$id}->[0],$data{$id}->[1]);
      if ($exps != $fnds) {
	die "Expected start time of id $id should be $exps, found $fnds.\n";
      }
      if ($expf != $fndf) {
	die "Expected finish time of id $id should be $expf, found $fndf.\n";
      }
    } else {
      my ($exp,$fnd) = ($diffs->{$id},($data{$id}->[1] - $data{$id}->[0]));
      if ($exp != $fnd) {
	die "Expected time diff for id $id to be $exp, found $fnd.\n";
      }
    }
  }
}

##
## </HELPERS>
##

1;
