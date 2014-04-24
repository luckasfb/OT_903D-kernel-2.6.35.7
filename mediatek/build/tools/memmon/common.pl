# Get name/value/line information from csv file for current load
# NOTE: we are able to accept "space" for the name

sub get_name
{
	my $line = $_[0];

	if ($line =~ /^\s*([\S\d ]+)\t+([\d\S\s]+)$/) {
		return $1;
	} else {
		return 0;
	}
}

sub get_value
{
	my $line = $_[0];

	if ($line =~ /^\s*([\S\d ]+)\t+([\d\S\s]+)$/) {
		return $2;
	} else {
		return 0;
	}
}

sub gen_empty_record
{
	my $num = $_[0];

	my $line = "0";
	for (;$num > 1; $num--) {
		$line = $line . "\t0";
	}
	return $line;
}

sub get_line
{
	my $fh = shift;
	my $line = <$fh>;
	if ($line) {
		chomp($line);
		$line =~ s/[\n,\r]//g;
	}
	return $line;
}

1;
