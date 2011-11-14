#!/usr/bin/env perl

use strict;
use warnings;
use lib qw(lib);

use Clubtuer::DB;
use ZeroMQ qw(:all);

use JSON;

my $db = Clubtuer::DB->new("./db");
my $users = $db->load_all;

use Data::Dumper;

print Data::Dumper->Dump([$users]);

sub lookup {
	my ($uid) = @_;

	$uid = unpack "H*", $uid;

	foreach my $user_name (keys %$users) {
		my $user = $users->{$user_name};

		next unless lc($user->{card}{uid}) eq lc($uid);

		print "User: $user_name\n";

		return "t" . pack "H*", $user->{card}{ca0523_door_key};
	}

	print "Did not find $uid\n";
	return "f";
}

my $cxt = ZeroMQ::Context->new;
my $sock = $cxt->socket(ZMQ_REP);
$sock->bind("tcp://127.0.0.1:4242");

while(1) {
	my $msg = $sock->recv();

	$msg = lookup($msg->data);

	$sock->send($msg);
}
