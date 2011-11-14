#!/usr/bin/env perl

use strict;
use warnings;
use lib qw(lib);

use Clubtuer::DB;
use List::MoreUtils qw(uniq);

use JSON;

sub calculate_trust {
	my ($users, $threshold) = @_;

	my $changed = 1;
	while($changed) {

		$changed = 0;
		foreach my $user_name (keys %$users) {
			my $user = $users->{$user_name};

			next if exists $user->{trust};

			if(exists $user->{autotrust}) {
				$user->{trust} = $user->{autotrust};
				$changed = 1;
			}

			my @trusted_buergen = grep($users->{$_}->{trust}, @{$user->{buergen}});

			if(@trusted_buergen >= $threshold) {
				$user->{trust} = 1;
				$changed = 1;
			}
		}
	}
}

my $db = Clubtuer::DB->new("./db");

my $users = $db->load_all;

calculate_trust($users, 3);

print <<EOF;
digraph trust {
EOF

foreach my $user (keys %$users) {
	my @buergen = @{$users->{$user}{buergen}};
	if(exists $users->{$user}{autotrust}) {
		print "\t$user [color=" . ($users->{$user}{autotrust} ? "red" : "grey") .
		    ", style=filled];\n";
	}
	elsif(exists $users->{$user}{trust}) {
		print "\t$user [color=green, style=filled];\n"
	}
	foreach my $buerge (@buergen) {
		print "\t$buerge -> $user;\n";
	}
}

print "}\n";
