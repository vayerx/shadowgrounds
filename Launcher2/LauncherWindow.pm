package LauncherWindow;

use strict;
use warnings;
use Glib qw(TRUE FALSE);
use Gtk2 -init;
use Gtk2::GladeXML;
use SGConfig;
use SGOptions;
use PerlIO;

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
    $self->{CONFIG}  = SGConfig->new("$ENV{HOME}/.survivor/Config/options.txt");
    $self->{OPTIONS} = SGOptions->new;
    $self->{RESULT}  = undef;
    bless($self,$class);
    $self->_init;
    return $self;
}

# Used to print warning messages
sub _warning
{
    my ($self,$text) = @_;
    print "WARNING: $text\n";
}

# Returns the associated config instance
sub _config
{
    my ($self) = @_;
    return $self->{CONFIG};
}

# Returns the associated options instance
sub _options
{
    my ($self) = @_;
    return $self->{OPTIONS};
}

# Initializes the widgets
sub _init
{
    my ($self) = @_;
    $self->{GUI} = Gtk2::GladeXML->new('LauncherWindow.glade');
    $self->{GUI}->signal_autoconnect_from_package($self);
    $self->_initializeWidgets;
    return $self;
}

# Returns the named widget or undef if no widget is found
sub _widget
{
    my ($self,$name) = @_;
    if (my $result = $self->{GUI}->get_widget($name)) {
        return $result;
    } else {
        return undef;
    }
}

# Returns the named widget. Causes the program to terminate if no widget is found
sub _existingWidget
{
    my ($self,$name) = @_;
    if (my $result = $self->_widget($name)) { return $result; } else { die "No such widget: $name"; }
}

# Initializes the widget(s) matching a single option
sub _initializeWidget
{
    my ($self,$name) = @_;
    if (my $option = $self->_options->option($name)) {
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
                    $self->_setAllWidgets($widget);
                });
            } elsif ($type eq "check") {
                $widget->set(label => $option->{title});
                $widget->signal_connect(toggled => sub {
                    $self->_setConfig($name);
                    $self->_setAllWidgets($widget);
                });
            } else { $self->_warning("Unknown option type: ".$option->{type}) }
            $self->_setWidgets(undef,$name);
        } else { $self->_warning("No such widget: $widget_name for option $name"); }
    } else { $self->_warning("No such option: $name"); }
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
    if (my $widget = $self->_widget($option->{type}."_$name")) {
        if ($option->{type} eq "combo") {
            my $value = $widget->get_active_text;
            my $items = $option->{items}->{$value} || die "Current widget value not found in supported options.";
            foreach my $key (keys %$items) {
                my $configValue = $self->_config->option($key);
                return 0 if ($configValue ne $items->{$key});
            }
            return 1;
        } elsif ($option->{type} eq "check") {
            my $value = $widget->get_active?"true":"false";
            my $items = $option->{items}->{$value} || die "Current widget value not found in supported options.";
            foreach my $key (keys %$items) {
                my $configValue = $self->_config->option($key);
                return 0 if ($configValue ne $items->{$key});
            }
            return 1;
        } else { $self->_warning("Unknown option type: ".$option->{type}) }
    } else { $self->_warning("No widget for option: $name"); }
    return 0;
}

# Sets current config values to match the values in the associated widget for the named option
sub _setConfig
{
    my ($self,$name) = @_;
    if (my $option = $self->_options->option($name)) {
        if (my $widget = $self->_widget($option->{type}."_$name")) {
            my $value = undef;
            if ($option->{type} eq "combo") {
                $value = $widget->get_active_text;
            } elsif ($option->{type} eq "check") {
                $value = $widget->get_active?"true":"false";
            } else { $self->_warning("setConfig: Unknown option type: ".$option->{type}) }
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
            } else { $self->_warning("setConfig: No value found for: $name"); }
        } else { $self->_warning("setConfig: No widget found for: $name"); }
    } else { $self->_warning("setConfig: No option found for: $name"); }
}

# Sets the current widget values to match the values in the named option
sub _setWidgets
{
    my ($self,$source,$name) = @_;
    my $option = $self->_options->option($name);
    if (my $widget = $self->_widget($option->{type}."_$name")) {
        return if (defined $source && $widget == $source);
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
							$value = ($option->{items}->[$i] eq "true")?1:0;
					} else { die "Unknown option type: ".$option->{type}; }
					last MAIN;
			  }
				if (defined $value) {
            $widget->set_active($value) if ($widget->get_active != -1);
				} else { $self->_warning("Unable to find a matching option value for: $name"); }
    } else { $self->_warning("Unable to find a widget for option: $name"); }
}

# Sets all widgets to match the relevant config values
sub _setAllWidgets
{
    my ($self,$widget) = @_;
    foreach my $option ($self->_options->options) { $self->_setWidgets($widget,$option); }
}

# Used signal not yet implemented functionality
sub _notImplemented
{
    my ($self) = @_;
    $self->_warning("Not yet implemented!");
}

# Returns the current result of this dialog
#  0 = Run game after quit
#  1 = Just quit
sub result
{
    my ($self) = @_;
    return $self->{RESULT};
}

# Quits the program and sets result to the given value
sub quit
{
    my ($self,$result) = @_;
    $self->{RESULT} = $result;
    $self->_config->commit if ($self->{RESULT} == 0);
    Gtk2->main_quit;
}

# Event handlers 

sub on_LauncherWindow_delete_event
{
    my ($self) = @_;
    $self->quit(-1);
}

sub on_launcher_launch_clicked
{
    my ($self) = @_;
    $self->_existingWidget("LauncherWindow")->set(visible => FALSE);
    $self->quit(0);
}

sub on_launcher_cancel_clicked
{
    my ($self) = @_;
    $self->_existingWidget("LauncherWindow")->set(visible => FALSE);
    $self->quit(-1);
}

sub on_launcher_advanced_clicked
{
    my ($self) = @_;
    $self->{ADVANCED} = $self->{CONFIG}->clone;
    $self->_existingWidget("notebook_main")->set(page => 1);
}

sub on_advanced_button_ok_clicked
{
    my ($self) = @_;
    undef $self->{ADVANCED};
    $self->_setAllWidgets(undef);
    $self->_existingWidget("notebook_main")->set(page => 0);
}

sub on_advanced_button_cancel_clicked
{
    my ($self) = @_;
    $self->{CONFIG}->assign($self->{ADVANCED});
    undef $self->{ADVANCED};
    $self->_setAllWidgets(undef);
    $self->_existingWidget("notebook_main")->set(page => 0);
}

sub on_launcher_activatemod_clicked
{
    my ($self) = @_;
    $self->_notImplemented;
}

# EOF

1;
