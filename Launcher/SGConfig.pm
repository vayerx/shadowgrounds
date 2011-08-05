package SGConfig;

use strict;
use constant STATUS_EMPTY    => 0;
use constant STATUS_LOADED   => 1;
use constant STATUS_MODIFIED => 2;

sub new
{
    my ($class,$path) = @_;
    my $self = bless(+{ },$class);
    if (defined $path) { $self->_readFile($path); } else { $self->reset(undef,STATUS_EMPTY); }
    return $self;
}

sub DESTROY
{
    my ($self) = @_;
    if ($self->{STATUS} == STATUS_MODIFIED) { $self->_writeFile($self->{PATH}); }
}

sub clone
{
    my ($self) = @_;
    my $result = SGConfig->new;
    $result->assign($self);
    return $result;
}

sub reset
{
    my ($self,$path,$status) = @_;
    $self->{PATH}     = $path || undef;
    $self->{SECTIONS} = { };
    $self->{MASTER}   = { };
    $self->{STATUS}   = $status || STATUS_EMPTY;
}

sub assign
{
    my ($self,$other) = @_;
    $self->reset($other->path,STATUS_LOADED);
    foreach my $section ($other->sections) {
	$self->createSection($section);
	foreach my $option ($other->options($section)) { $self->createOption($section,$option,$other->option($option)); }
    }
}

sub path
{
    my ($self) = @_;
    return $self->{PATH};
}

sub _readFile
{
    my ($self,$path) = @_;
    $self->reset($path,STATUS_LOADED);
    my $section = undef;
    my $depth   = 0;
    open FI,"<:crlf",$path || die "Error opening file: $path";
    while(chomp(my $line = <FI>)) {
	if ($line =~ /^\s*$/) {
	    # Skip empty lines
	} elsif ($line =~ /^\s*{\s*$/) {
	    die "Error in config file" if (++$depth > 1);
	} elsif ($line =~ /^\s*}\s*$/) {
	    die "Error in config file" if (--$depth < 0 || !defined $section);
	    $section = undef;
	} elsif ($line =~ /^\s*(\S+)\s*=\s*(.+)\s*$/ && $depth > 0) {
	    die "Error in config file" unless (defined $section);
	    $self->createOption($section,$1,$2);
	} elsif ($line =~ /^\s*(.+)\s*$/ && $depth == 0) {
	    $self->createSection($section = $1);
	}
    }
    close FI;
}

sub _writeFile
{
    my ($self,$path) = @_;
    open FO,">:crlf",$path;
    foreach my $section (keys %{$self->{SECTIONS}}) {
	my $values = $self->{SECTIONS}->{$section};
	print FO "$section\n{\n";
	foreach my $value (keys %$values) {
	    print FO "   $value = ".$values->{$value}."\n";
	}
	print FO "}\n\n";
    }
}

sub commit
{
    my ($self) = @_;
    $self->{STATUS} = STATUS_MODIFIED;
}

sub sections
{
    my ($self) = @_;
    return keys %{$self->{SECTIONS}};
}

sub section
{
    my ($self,$section) = @_;
    return $self->{SECTIONS}->{$section} || die "No such section: $section";
}

sub options
{
    my ($self,$section) = @_;
    die "No such section: $section" unless (defined $self->{SECTIONS}->{$section});
    return keys %{$self->{SECTIONS}->{$section}};
}

sub createSection
{
    my ($self,$section) = @_;
    die "Section already exists: $section" if ($self->{SECTIONS}->{$section});
    $self->{SECTIONS}->{$section} = { };
}

sub deleteSection
{
    my ($self,$section) = @_;
    die "No such section: $section" unless ($self->{SECTIONS}->{$section});
    foreach my $key (keys %{$self->{SECTIONS}->{$section}}) { undef $self->{MASTER}->{$key} }
    undef $self->{SECTIONS}->{$section};
}

sub createOption
{
    my ($self,$section,$name,$value) = @_;
    die "No such section: $section" unless defined($self->{SECTIONS}->{$section});
    die "Option with the given name ($name) already exits" if (defined $self->{MASTER}->{$name});
    $self->{SECTIONS}->{$section}->{$name} = $value;
    $self->{MASTER}->{$name} = \$self->{SECTIONS}->{$section}->{$name};
}

sub deleteOption
{
    my ($self,$section,$name) = @_;
    die "No such section: $section" unless defined($self->{SECTIONS}->{$section});
    die "Option with the given name ($name) already exits" if (defined $self->{MASTER}->{$name});
    undef $self->{SECTIONS}->{$section}->{$name};
    undef $self->{MASTER}->{$name};
}

sub option
{
    my ($self,$name,$value) = @_;
    if (defined $self->{MASTER}->{$name}) {
	if (defined $value) {
	    ${$self->{MASTER}->{$name}} = $value;
	} else {
	    ${$self->{MASTER}->{$name}};
	}
    } else { die "No such option: $name" }
}

1;
