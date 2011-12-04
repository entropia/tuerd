#!/usr/bin/env perl

use strict;
use warnings;

use JSON::XS;

sub slurp {
	my ($f) = @_;

	my $str;
	open(my $fh, "<$f");
	{
		local $/;
		$str = <$fh>;
	}

	close($fh);
	return $str;
}

if(@ARGV < 1) {
	print "Usage: $0 foo.json";
	exit 1;
}

my $json = decode_json(slurp($ARGV[0]));

open(my $out, ">", $json->{card}{uid} . ".xml");

print $out <<EOF;
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<card uid="$json->{card}{uid}">
    <active>true</active>
    <picc_key>$json->{card}{picc_key}</picc_key>
    <ca0523_master_key>$json->{card}{ca0523_master_key}</ca0523_master_key>
    <ca0523_door_key>$json->{card}{ca0523_door_key}</ca0523_door_key>
    <personal>
        <name>$json->{name}</name>
        <sponsors>
            <name></name>
        </sponsors>
    </personal>
</card>
EOF

close($out);
