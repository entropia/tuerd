#!/usr/bin/env perl

use strict;
use warnings;
use lib qw(lib);

use Clubtuer::DB;
use List::MoreUtils qw(uniq);

use JSON;

my $db = Clubtuer::DB->new("./db");

sub calculate_trust {
	my ($db, $threshold) = @_;

	my $changed = true;
	while($changed) {

		$changed = false;
		foreach my $user_name (keys $db) {
			my $user = $db->{$user_name};

			next if exists $user->{trust};

			if(exists $user->{autotrust}) {
				$user->{trust} = $user->{autotrust};
				$changed = true;
			}

			my @trusted_buergen = grep {
				exists $db->{$_}->{trust} and
				    $db->{$_}->{trust};
			}, $user->{buergen};

			if(@trusted_buergen >= 3) {
				$user->{trust} = true;
				$changed = true;
			}
		}
	}
}

my $users = $db->load_all;

