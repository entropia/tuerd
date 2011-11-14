#!/usr/bin/env perl

use strict;
use warnings;
use lib qw(lib);

use Clubtuer::DB;
use JSON;

my $db = Clubtuer::DB->new("./db");

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

my $user = $db->load($ARGV[0]);

die "User already deployed" if exists $user->{card};

$user->{card} = picc_deploy;

$db->store($user);
