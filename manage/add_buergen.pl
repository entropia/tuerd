#!/usr/bin/env perl

use strict;
use warnings;
use lib qw(lib);

use Clubtuer::DB;
use List::MoreUtils qw(uniq);

use JSON;

my $db = Clubtuer::DB->new("./db");

if(@ARGV < 2) {
	print STDERR "Usage: $0 user buerge1 [... buergeN]\n";
	exit 1;
}

my $user = $db->load(shift @ARGV);

$user->{buergen} = [uniq @{$user->{buergen}}, @ARGV];

$db->store($user);
