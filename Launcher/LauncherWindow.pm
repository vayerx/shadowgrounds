package LauncherWindow;

use strict;
use warnings;
use Glib qw(TRUE FALSE);
use Gtk2 -init;
use Gtk2::GladeXML;
use SGConfig;
use SGOptions;

sub run
{
    my ($class,$config,$options) = @_;
    my $self = $class->new($config,$options);
    Gtk2->main;
    return $self->result;
}

sub new
{
    my ($class) = @_;
    my $self = { };
    $self->{CONFIG}  = SGConfig->new("Config/options.txt");
    $self->{OPTIONS} = SGOptions->new;
    $self->{RESULT}  = undef;
    bless($self,$class);
    $self->_init;
    return $self;
}

sub _warning
{
    my ($self,$text) = @_;
    print "WARNING: $text\n";
}

sub _config
{
    my ($self) = @_;
    return $self->{CONFIG};
}

sub _options
{
    my ($self) = @_;
    return $self->{OPTIONS};
}

sub _init
{
    my ($self) = @_;
    $self->{GUI} = Gtk2::GladeXML->new('Resource/LauncherWindow.glade');
    $self->{GUI}->signal_autoconnect_from_package($self);
    $self->_initializeWidgets;
    return $self;
}

sub _widget
{
    my ($self,$name) = @_;
    return $self->{GUI}->get_widget($name);
}

# Initializes the widget(s) matching a single option
sub _initializeWidget
{
    my ($self,$name) = @_;
    my $option = $self->_options->option($name);
    my $type = $option->{type};
    my $widget_name = $type."_$name";
    if (my $widget = $self->_widget($widget_name)) {
	if (my $label = $self->_widget("label_$name")) { $label->set(label => $option->{title}); }
	if ($type eq "combo") {
	    for(my $i=0;$i<=$#{$option->{items}};$i+=2) {
		$widget->append_text($option->{items}->[$i]);
	    }
	    $widget->set_active(0);
	    $widget->signal_connect(changed => sub {
		$self->_setConfig($name);
		$self->_setAllWidgets();
	    });
	} elsif ($type eq "check") {
	    $widget->set(label => $option->{title});
	    $widget->signal_connect(toggled => sub {
		$self->_setConfig($name);
		$self->_setAllWidgets();
	    });
	} else { $self->_warning("Unknown option type: ".$option->{type}) }
       	$self->_setWidgets($name);
    } else { $self->_warning("No such widget: $widget_name for option $name"); }
}

# Initializes all widgets
sub _initializeWidgets
{
    my ($self) = @_;
    foreach my $name ($self->_options->options) { $self->_initializeWidget($name); }
}

# UNUSED: Returns true if current widget value matches the current configuration for the named option
sub _matchOption
{
    my ($self,$name) = @_;
    my $option = $self->_options->option($name);
    my $widget = $self->_widget($option->{type}."_$name");
    if ($option->{type} eq "combo") {
	my $value = $widget->get_active_text;
	my $items = $option->{items}->{$value} || die "Current widget value not found in supported options.";
	foreach my $key (keys %$items) {
	    my $configValue = $self->_config->option($key);
	    return 0 if ($configValue ne $items->{$key});
	}
	return 1;
    } elsif ($option->{type} eq "check") {

    } else { $self->_warning("Unknown option type: ".$option->{type}) }
}

# Sets current config values to match the values in the associated widget for the named option
sub _setConfig
{
    my ($self,$name) = @_;
    my $option = $self->_options->option($name);
    my $widget = $self->_widget($option->{type}."_$name");
    my $value = undef;
    if ($option->{type} eq "combo") {
	$value = $widget->get_active_text;
    } elsif ($option->{type} eq "check") {
	$value = $widget->get_active?"true":"false";
    } else { $self->_warning("Unknown option type: ".$option->{type}) }
    if (defined $value) {
	my $items = eval {
	    for(my $i=0;$i<=$#{$option->{items}};$i+=2) {
		return $option->{items}->[$i+1] if ($option->{items}->[$i] eq $value);
	    }
	    return undef;
	};
	foreach my $key (keys %$items) {
	    # print $key," = ",$items->{$key},"\n";
	    $self->_config->option($key,$items->{$key});
	}
    } else { $self->warning("No value found for: $name"); }
}

# Sets the current widget values to match the values in the named option
sub _setWidgets
{
    my ($self,$name) = @_;
    my $option = $self->_options->option($name);
    if (my $widget = $self->_widget($option->{type}."_$name")) {
	my $value = undef;
        MAIN: for(my $i=0;$i<=$#{$option->{items}};$i+=2) {
	    my $items = $option->{items}->[$i+1];
	    foreach my $key (keys %$items) {
		my $current = $self->_config->option($key);
		if (defined $current) {
		    next MAIN if ($current ne $items->{$key});
		} else { $self->_warning("Unable to find config value matching: $key"); }
	    }
	    if ($option->{type} eq "combo") {
		$value = $i/2;
	    } elsif ($option->{type} eq "check") {
		$value = $option->{items}->[$i] eq "true";
	    } else { die "Unknown option type: ".$option->{type}; }
	    last MAIN;
	}
	if (defined $value) {
	    $widget->set_active($value);
	} else { die "Unable to find a matching option value for: $name"; }
    } else { $self->_warning("Unable to find a widget for option: $name"); }
}

# Sets all widgets to match the relevant config values
sub _setAllWidgets
{
    my ($self) = @_;
    foreach my $option ($self->_options->options) { $self->_setWidgets($option); }
}

sub _notImplemented
{
    my ($self) = @_;
    $self->_warning("Not yet implemented!");
}

sub result
{
    my ($self) = @_;
    return $self->{RESULT};
}

sub quit
{
    my ($self,$result) = @_;
    $self->{RESULT} = $result;
    Gtk2->main_quit;
}

sub on_LauncherWindow_delete_event
{
    my ($self) = @_;
    $self->quit(-1);
}

sub on_launcher_launch_clicked
{
    my ($self) = @_;
    $self->_config->commit;
    $self->_widget("LauncherWindow")->set(visible => FALSE);
    $self->quit(0);
}

sub on_launcher_cancel_clicked
{
    my ($self) = @_;
    $self->_widget("LauncherWindow")->set(visible => FALSE);
    $self->quit(-1);
}

sub on_launcher_advanced_clicked
{
    my ($self) = @_;
    $self->{ADVANCED} = $self->{CONFIG}->clone;
    $self->_widget("notebook_main")->set(page => 1);
}

sub on_advanced_button_ok_clicked
{
    my ($self) = @_;
    undef $self->{ADVANCED};
    $self->_setAllWidgets;
    $self->_widget("notebook_main")->set(page => 0);
}

sub on_advanced_button_cancel_clicked
{
    my ($self) = @_;
    $self->{CONFIG}->assign($self->{ADVANCED});
    undef $self->{ADVANCED};
    $self->_setAllWidgets;
    $self->_widget("notebook_main")->set(page => 0);
}

sub on_launcher_activatemod_clicked
{
    my ($self) = @_;
    $self->_notImplemented;
}

1;
