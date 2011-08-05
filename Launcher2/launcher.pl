#!/usr/bin/perl
# -w
package main;

use strict;
use LauncherWindow;

unless (LauncherWindow->run) {
    exec `./survivor-bin`;
}
