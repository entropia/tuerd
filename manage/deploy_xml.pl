#!/usr/bin/env perl

use strict;
use warnings;
use lib qw(lib);

use JSON;

sub picc_deploy {
	open(my $fh, "./deploy_club|");
	my $card;
      	{
		local $/;
		$card=<$fh>;
	};

	if(not close($fh)) {
		die "Deploying card failed";
	}

	return decode_json $card;
}

if(@ARGV < 1) {
	print STDERR "Usage: $0 user\n";
	exit 1;
}

my $json = picc_deploy;

open(my $out, ">", $json->{uid} . ".xml");

print $out <<EOF;
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<card uid="$json->{uid}">
    <active>true</active>
    <picc_key>$json->{picc_key}</picc_key>
    <ca0523_master_key>$json->{ca0523_master_key}</ca0523_master_key>
    <ca0523_door_key>$json->{ca0523_door_key}</ca0523_door_key>
    <personal>
        <name>$ARGV[0]</name>
        <sponsors>
            <name></name>
        </sponsors>
    </personal>
</card>
EOF

close($out);
