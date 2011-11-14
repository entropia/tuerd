#!/usr/bin/env perl

use v5.12;
use strict;
use warnings;
use lib qw(lib);

use Clubtuer::DB;
use ZeroMQ qw(:all);
use Linux::Inotify2;
use JSON;

my $db = Clubtuer::DB->new("./db");
my $users = $db->load_all;

sub lookup {
	my ($uid) = @_;

	lock($users);

	$uid = unpack "H*", $uid;
	print "UID: $uid\n";

	foreach my $user_name (keys %$users) {
		my $user = $users->{$user_name};

		next unless exists $user->{card};
		next unless lc($user->{card}{uid}) eq lc($uid);

		print "User: $user_name\n";

		return "t" . pack("H*", $user->{card}{ca0523_door_key}) . $user->{name};
	}

	print "Did not find $uid\n";
	return "f";
}

my $inotify = Linux::Inotify2->new() or die "Failed to create inotify";
$inotify->blocking(0);
$inotify->watch("./db", IN_CREATE | IN_MODIFY, sub {
		my ($e) = @_;

		say "New file in DB: " . $e->fullname;
		$users = $db->load_all;
	});

my $cxt = ZeroMQ::Context->new;
my $sock = $cxt->socket(ZMQ_REP);
$sock->bind("tcp://127.0.0.1:4242");

while(1) {
	my $msg = $sock->recv();

	$inotify->poll;

	$msg = lookup($msg->data);

	$sock->send($msg);
}
