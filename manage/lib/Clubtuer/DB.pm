package Clubtuer::DB;

use strict;
use warnings;

use Carp;
use JSON;

sub new {
	my $self = shift;
	my $dbdir = shift;

	my $class = ref($self) || $self;
	return bless {dbdir => $dbdir}, $class;
}

sub slurp {
	my $file = shift;

	open(my $fh, "<$file") or croak "Opening file $file failed: $!";

	my $contents;
	eval {
		local $/;
		$contents = <$fh>;
		1;
	} or croak "Reading file $file failed: $!";

	close($fh);
	return $contents;
}

sub user_file {
	my ($self, $name) = @_;

	return $self->{dbdir} . "/$name.json";
}

sub user_exists {
	my ($self, $name) = @_;

	return -f $self->user_file($name);
}

sub load {
	my $self = shift;
	my $name = shift;

	my $json;
	if($self->user_exists($name)) {
		my $file = $self->user_file($name);

		$json = decode_json(slurp($file)) or croak "Loading JSON failed: $@";
	}
	else {
		$json = {};
	}

	$json->{name} = $name;
	return $json;
}

sub store {
	my ($self, $user) = @_;

	croak "User has no name" unless $user->{name};

	my $file = $self->user_file($user->{name});
	open(my $fh, ">$file") or croak "Opening file $file failed: $!";

	my $json = JSON->new;

	print $fh $json->pretty->encode($user);
	close($fh);
}

sub load_all {
	my $self = shift;

	my @users = map {$_ =~ s!.*/([^.]*).json!$1!; $_} glob $self->{dbdir} . "/*.json";

	my $db;
	foreach my $user (@users) {
		$db->{$user} = $self->load($user);
	}

	return $db;
}

1;
