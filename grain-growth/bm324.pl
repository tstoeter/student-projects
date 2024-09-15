#!/usr/bin/perl -w
# usage: ./bm324.pl < 32bit.bm3 > 24bit.bm3

my $i = 0;

while (defined($c = getc(STDIN)))
{
	$i++;

	if ($i == 16)
	{
		print chr(0x18);
		next;
	}

	if ($i <= 20)
	{
		print $c;
		next;
	}

	if ($i % 4 != 0)
	{
		print $c;
	}
}

