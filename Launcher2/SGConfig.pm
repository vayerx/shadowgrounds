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
    $self->_writeFile($self->{PATH}) if ($self->{STATUS} == STATUS_MODIFIED);
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
    my %defaults = (Camera => {
                        camera_rotation_fade_start => 768,
                        camera_rotation_safe => 300,
                        camera_rotation_spring => 1.57143,
                        camera_rotation_fade_end => 769,
                        camera_rotation_strength => 3,
                        camera_rotation_auto_strength => 0.7
                    },
                    Display => {
                        anisotrophy => 0,
                        screen_height => 768,
                        contrast => 1,
                        windowed => 1,
                        gamma => 1,
                        brightness => 1,
                        window_titlebar => 0,
                        antialias_samples => 0,
                        screen_bpp => 32,
                        maximize_window => 1,
                        render_use_vsync => 0,
                        screen_width => 1024,
                    },
                    GUI => {
                        gui_tip_message_level => 2,
                        show_custom_survival_missions => 0,
                        scorewindow_name_input => 1,
                        use_old_loadgamemenu => 0,
                    },
                    Video => {
                        high_quality_video => 0,
                        menu_logo_video_enabled => 0,
                        menu_video_enabled => 0,
                        video_enabled => 1,
                    },
                    Reserved => {
                        camera_rotation_auto_fade_end => 0,
                        camera_rotation_auto_fade_start => 0
                    },
                    Controllers => {
                        joystick_sensitivy => 1,
                        player4_mouse_ID => 0,
                        mouse_force_given_boundary => 0,
                        multiple_input_devices_enabled => 0,
                        player2_mouse_ID => 1,
                        player3_mouse_ID => 0,
                        joystick_enabled => 1,
                        mouse_enabled => 1,
                        player1_mouse_ID => 2,
                        mouse_sensitivity => 1,
                        keyboard_enabled => 1,
                    },
                    Locale => {
                        menu_language => 0,
                        speech_language => 0,
                        subtitle_language => 0,
                    },
                    Game => {
                        game_very_hard_available => 0,
                        force_mission_failure => 0,
                        corpse_disappear_time => 0,
                        forcewear_enabled => 0,
                        friendly_fire => 0,
                        game_extremely_hard_available => 0,
                        game_mode_aim_upward => 0,
                        show_tutorial_hints => 1,
                    },
                    Debug => {
                        console_loglevel => 3,
                        show_player_pos => 0,
                        raise_console_loglevel => 0,
                        console_history_save => 1,
                        loglevel => 4,
                    },
                    Graphics => {
                        environment_animations => 1,
                        procedural_fallback => 0,
                        fakeshadows_texture_quality => 50,
                        high_quality_lightmap => 0,
                        shadows_texture_quality => 50,
                        render_distortion => 1,
                        render_half_resolution => 0,
                        hit_effect_image => 0,
                        shadows_level => 50,
                        extra_gamma_effects => 1,
                        gore_level => 50,
                        lighting_level => 50,
                        render_max_pointlights => 5,
                        reset_renderer_when_loaded => 1,
                        render_sky_bloom => 1,
                        render_reflection => 0,
                        weather_effects => 1,
                        render_particle_reflection => 0,
                        lighting_texture_quality => 50,
                        better_glow_sampling => 0,
                        particle_effects_level => 50,
                        render_glow => 0,
                        layer_effects_level => 50,
                        texture_detail_level => 100,
                        green_blood => 0,
                    },
                    Sounds => {
                        sound_software_channels => 32,
                        ambient_volume => 51,
                        music_enabled => 1,
                        sound_max_hardware_channels => 32,
                        music_volume => 95,
                        sound_speaker_type => "stereo",
                        sound_use_eax => 0,
                        fx_volume => 52,
                        master_volume => 100,
                        music_shuffle => 1,
                        sound_required_hardware_channels => 16,
                        sound_mixrate => 44100,
                        sounds_enabled => 1,
                        fx_enabled => 1,
                        speech_enabled => 1,
                        speech_volume => 100,
                        sound_use_hardware => 0,
                    },
                    Effects => {
                        weapon_eject_effect_level => 50,
                        decal_max_amount => 400,
                        decal_fade_time => 3000,
                    },
                    Physics => {
                        physics_enabled => 1,
                        physics_fluids_enabled => 1,
                        physics_max_fluid_particles => 50,
                        physics_use_multithreading => 1,
                        physics_max_model_particles => 300,
                        physics_max_model_particles_spawn_per_tick => 1,
                        physics_particles => 1,
                        physics_use_hardware => 0,
                        physics_use_hardware_fully => 0,
                    });
    $self->reset($path,STATUS_LOADED);
    foreach my $section (keys %defaults) {
        $self->createSection($section);
        foreach my $name (keys %{$defaults{$section}}) {
            $self->createOption($section,$name,$defaults{$section}->{$name});
        }
    }
    my $section = undef;
    my $depth   = 0;
    if (open FI,"<:crlf",$path) {
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
                $self->sectionOption($section,$1,$2);
                # $self->createOption($section,$1,$2);
            } elsif ($line =~ /^\s*(.+)\s*$/ && $depth == 0) {
                $section = $1;
                # $self->createSection($section = $1);
            }
        }
        close FI;
    }
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
    $self->_writeFile($self->{PATH}) if ($self->{STATUS} == STATUS_MODIFIED);
    $self->{STATUS} = STATUS_LOADED;
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
            $self->{STATUS} = STATUS_MODIFIED;
        } else {
            return ${$self->{MASTER}->{$name}};
        }
    } else { die "No such option: $name" }
}

sub sectionOption
{
    my ($self,$section,$name,$value) = @_;
    if (defined $self->{SECTIONS}->{$section}) {
        if (defined $self->{SECTIONS}->{$section}->{$name}) {
            if (defined $value) {
                $self->{SECTIONS}->{$section}->{$name} = $value;
            } else {
                return $self->{SECTIONS}->{$section}->{$name};
            }
        } else { die "No such option: $name" }
    } else { die "No such section: $section" }
}

1;
