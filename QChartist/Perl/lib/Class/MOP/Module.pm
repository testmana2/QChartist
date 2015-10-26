
package Class::MOP::Module;
BEGIN {
  $Class::MOP::Module::AUTHORITY = 'cpan:STEVAN';
}
$Class::MOP::Module::VERSION = '2.1206';
use strict;
use warnings;

use Carp         'confess';
use Scalar::Util 'blessed';

use parent 'Class::MOP::Package';

use Moose::Util 'throw_exception';

sub _new {
    my $class = shift;
    return Class::MOP::Class->initialize($class)->new_object(@_)
        if $class ne __PACKAGE__;

    my $params = @_ == 1 ? $_[0] : {@_};
    return bless {
        # Need to quote package to avoid a problem with PPI mis-parsing this
        # as a package statement.

        # from Class::MOP::Package
        'package' => $params->{package},
        namespace => \undef,

        # attributes
        version   => \undef,
        authority => \undef
    } => $class;
}

sub version {
    my $self = shift;
    ${$self->get_or_add_package_symbol('$VERSION')};
}

sub authority {
    my $self = shift;
    ${$self->get_or_add_package_symbol('$AUTHORITY')};
}

sub identifier {
    my $self = shift;
    join '-' => (
        $self->name,
        ($self->version   || ()),
        ($self->authority || ()),
    );
}

sub create {
    my $class = shift;
    my @args = @_;

    unshift @args, 'package' if @args % 2 == 1;
    my %options = @args;

    my $package   = delete $options{package};
    my $version   = delete $options{version};
    my $authority = delete $options{authority};

    my $meta = $class->SUPER::create($package => %options);

    $meta->_instantiate_module($version, $authority);

    return $meta;
}

sub _anon_package_prefix { 'Class::MOP::Module::__ANON__::SERIAL::' }

sub _anon_cache_key {
    my $class = shift;
    my %options = @_;
    throw_exception( PackagesAndModulesAreNotCachable => class_name => $class,
                                                         params     => \%options,
                                                         is_module  => 1
                   );
}

sub _instantiate_module {
    my($self, $version, $authority) = @_;
    my $package_name = $self->name;

    $self->add_package_symbol('$VERSION' => $version)
        if defined $version;
    $self->add_package_symbol('$AUTHORITY' => $authority)
        if defined $authority;

    return;
}

1;

# ABSTRACT: Module Meta Object

__END__

=pod

=encoding UTF-8

=head1 NAME

Class::MOP::Module - Module Meta Object

=head1 VERSION

version 2.1206

=head1 DESCRIPTION

A module is essentially a L<Class::MOP::Package> with metadata, in our
case the version and authority.

=head1 INHERITANCE

B<Class::MOP::Module> is a subclass of L<Class::MOP::Package>.

=head1 METHODS

=over 4

=item B<< Class::MOP::Module->create($package, %options) >>

Overrides C<create> from L<Class::MOP::Package> to provide these additional
options:

=over 4

=item C<version>

A version number, to be installed in the C<$VERSION> package global variable.

=item C<authority>

An authority, to be installed in the C<$AUTHORITY> package global variable.

This is a legacy field and its use is not recommended.

=back

=item B<< $metamodule->version >>

This is a read-only attribute which returns the C<$VERSION> of the
package, if one exists.

=item B<< $metamodule->authority >>

This is a read-only attribute which returns the C<$AUTHORITY> of the
package, if one exists.

=item B<< $metamodule->identifier >>

This constructs a string which combines the name, version and
authority.

=item B<< Class::MOP::Module->meta >>

This will return a L<Class::MOP::Class> instance for this class.

=back

=head1 AUTHORS

=over 4

=item *

Stevan Little <stevan.little@iinteractive.com>

=item *

Dave Rolsky <autarch@urth.org>

=item *

Jesse Luehrs <doy@tozt.net>

=item *

Shawn M Moore <code@sartak.org>

=item *

יובל קוג'מן (Yuval Kogman) <nothingmuch@woobling.org>

=item *

Karen Etheridge <ether@cpan.org>

=item *

Florian Ragwitz <rafl@debian.org>

=item *

Hans Dieter Pearcey <hdp@weftsoar.net>

=item *

Chris Prather <chris@prather.org>

=item *

Matt S Trout <mst@shadowcat.co.uk>

=back

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2006 by Infinity Interactive, Inc..

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut