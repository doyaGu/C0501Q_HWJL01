#!/usr/bin/perl -w
# Generate a file list of specification documentations.
# Initial version V0.1.0, 2014/12/09

use strict;
use warnings;
use File::Copy;
use File::Basename;
use Time::HiRes qw(gettimeofday);
use Cwd;  #get current working directory
use utf8;
use 5.010;

my $selfPath = cwd;

my @dirlist;
my @filelist;

my @files;
my @dirs;

die "Usage: gg.pl src tar\n" if ($#ARGV != 1);
    
my $src = $ARGV[0];
my $tar = $ARGV[1];

if (-e $src) {
    say "source: $src";
} else {
    die "not found source.";
}

unlink $tar if (-e $tar);

system("find $src > list.txt");

open FILE, "list.txt" || die "open list.txt failed, $!";
for(<FILE>) {
    $_ =~ s/$src//gi;
    $_ =~ s/\r|\n//gi;
    $_ = '' unless(/\.\w+$/);
    #say "--- $_";
    push @filelist, $_ if(length($_) > 4);
}
close(FILE);

#say $_ for(@filelist);

#open FILE, ">output.txt" || die "write failed, $!";
#for(@filelist) {
#    say FILE $_;
#}
#close(FILE);

&findDir(@filelist);
@dirs = sort(@dirs);
#say "found dirs: $_" for(@dirs);

@filelist = sort(@filelist);

#say "*******************************";
#say $_ for(@filelist);

&textToHTML;

unlink("list.txt");

# **************************************************************************************

sub textToHTML {
    open FILE, ">$tar" || die "Can not open file, $!.";
    say FILE '<html>';
    say FILE '    <head>';
    say FILE '        <title>ITE Soc documents</title>';
    say FILE '        <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=UTF-8"/>';
    say FILE '        <style>';
    say FILE '            body {color:black;font:18px courier;background-color:white;}';
    say FILE '            span#dir {color:#black;font:18px courier;background-color:white;font-weight: bold;}';
    say FILE '            span#file {color:#00bf00;font:18px courier;background-color:white;}';
    say FILE '            span#title {color:#black;font:24px courier;background-color:white;font-weight: bold;}';
    say FILE '            span#number {color:#B041FF;font:18px courier;background-color:white;}';
    say FILE '            h1 {text-align:center;}';
    say FILE '        </style>';
    say FILE '    </head>';
    say FILE '    <body><p>';
    say FILE '    <h1>ITE Video Soc documents</h1><br />';

    for(my $i = 0; $i <= $#dirs; $i++) {
          say FILE &tab(8).'<h3><span id="dir">'.&tabs(4).$dirs[$i].'</span><br /></h3>';

          for(my $j = 0; $j <= $#filelist; $j++) {
               my $fn = &findFileName($filelist[$j]);
         my $ln = $filelist[$j];
         $ln =~ s/^\///;
         #say "ln = $ln";
               if($filelist[$j] =~ /$dirs[$i]/ && $filelist[$j] =~ '/') {
            my $html = &tab(8).&tabs(8)."<span id=\"file\"><a href=\"$ln\">$fn</a></span><br />";
            #say $html;
            say FILE $html;
               }
          }
    }

    say FILE '    </p></body>';
    say FILE '</html>';
    close FILE;
}

sub tab {
     return ' 'x$_[0];
}

sub tabs {
     return '&nbsp'x$_[0];
}

sub findDir {
    for(@_) {        
        my $s = '';
        $s = substr($_, 0, rindex($_, '/') + 1);
        if(length($s) > 4) {
            #say "*** : $s ".length($s);
            push @dirs, $s unless(/@dirs =~ $s/);
        }
    }
}

sub findFileName {
    my $s = '';
    if($_[0] =~ /^\//) {
        $s = substr($_[0], rindex($_[0], '/') + 1, length($_[0]));
    } else {
        $s = substr($_[0], rindex($_[0], '/') + 1, length($_[0]));
    }
      return $s;
}
