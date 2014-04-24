#!/usr/bin/perl

###
# rommon.pl
#
# This file is the main entry of ROM Monitor.
#
# Input:
# Assume the database excel file will be located in current working directory
# All output file will be located to $root 
#
# Output:
# 1. $load_ver.{img_size,fsize,readelf*,apk_*}.csv in $root
#

use strict;
use warnings;

use Switch;
use Math::BigInt;
use IO::File;
use File::Basename;

my $LOCAL_PATH; # script path(relative path)

BEGIN
{
  $LOCAL_PATH = dirname($0);
}

use lib "./${LOCAL_PATH}/../";
use lib "./${LOCAL_PATH}/../Spreadsheet";
use lib "./${LOCAL_PATH}";

use Spreadsheet::ParseExcel;
use Spreadsheet::WriteExcel;
use Data::Dumper;

require "common.pl";

# sample only these image for rommon
# XXX should be replaced with platform name
my @sys_imgs = (
  "preloader_XXX.bin", 
  "DSP_BL",
  "uboot_XXX.bin", 
  "boot.img", 
  "recovery.img", 
  "secro.img", 
  "system.img", 
  "logo.bin", 
  "userdata.img", 
);
my @sys_tree = (
  "bin", 
  "xbin", 
  "lib",
  "lib/egl",
  "lib/hw",
  "lib/modules",
);
my @sys_profiling = (
  "img_size",
  "system_fsize",
  "root_fsize",
  "elf_r",
  "elf_re",
  "elf_rw",
  "kelf_r",
  "kelf_re",
  "kelf_rw",
  "apk_total",
  "apk_resource",
);
my %mem_stat;

# load information
my $label = "";
my $project ="";
my $flavor = "";

# internal global variable
my $root = "rommon_results";  # main directory of rommon output
my $load_ver="";    # $lable + $project + $flavor (unique id)
my $database_dir = "";    # The directory which stores all databases in perforce
my $project_out_dir = "";  # the output of android bulid
my $tmp_dir="";      # tmp file location
my $database = "";     # the database excel name

# tool information
my $cross_prefix = "arm-eabi";
my $readelf = "$cross_prefix-readelf";
my $objdump = "$cross_prefix-objdump";

my $__debug = 0;    # set 1 to output debug information

sub parse_readelf
{
  my $fh = new IO::File;
  my $binary = $_[0];
  my $binary_basename = $binary;
  my $fname;
  $binary_basename =~ s/$project_out_dir//;
  $fname = $binary_basename; #file name for storing in file system
  $fname =~ s/\.\.\///g;
  $fname =~ s/\//\_/g;
  #print $fname;

  if (-d $binary)
  {
    return;
  }

  if (-B $binary)
  {
    system("$readelf -l $binary > $tmp_dir/.tmp.readelf");
    if ($? < 0) {return;}
  }
  else
  {
    return;
  }

  $mem_stat{$binary_basename}{"R"} = 0;
  $mem_stat{$binary_basename}{"RE"} = 0;
  $mem_stat{$binary_basename}{"RW"} = 0;

  die ".tmp.procrank open fails\n" unless $fh->open("$tmp_dir/.tmp.readelf");
  while ( <$fh> )
  {
    chomp;
    s/[\n,\r]//g;
    my ($hdr,$offset,$vaddr,$paddr,$fsize,$memsize,$flag,$align) =
        /^\s+(\S+)\s+0x([\d\S]+)\s+0x([\d\S]+)\s+0x([\d\S]+)\s+(0x[\d\S]+)\s+(0x[\d\S]+)\s([\S ]{3})\s+(0x\d+)\s*$/;

    if ( defined($hdr) )
    {
      #if ($__debug) {print "$hdr $fsize $memsize $flag\n";}
      $fsize = hex($fsize); # convert the hex string to dec int

      switch ($flag)
      {
        case "R  "
          { $mem_stat{$binary_basename}{"R"} += $fsize; }
        case "R E"
          { $mem_stat{$binary_basename}{"RE"} += $fsize; }
        case "RW "
          { $mem_stat{$binary_basename}{"RW"} += $fsize; }
        case "RWE"
          { $mem_stat{$binary_basename}{"RW"} += $fsize; }
      }
    }
  }
  $fh->close;

  #system("$objdump -h $binary > $root/$fname.hdr");
  #system("$objdump -t $binary > $root/$fname.sym");
}

sub dump_readelf_results
{
  my $elfout_r = "$root/elf_r";
  my $elfout_re = "$root/elf_re";
  my $elfout_rw = "$root/elf_rw";
  sysopen (FL_OUT_R, $elfout_r, O_RDWR|O_CREAT, 0644);
  sysopen (FL_OUT_RE, $elfout_re, O_RDWR|O_CREAT, 0644);
  sysopen (FL_OUT_RW, $elfout_rw, O_RDWR|O_CREAT, 0644);

  my $kelfout_r = "$root/kelf_r";
  my $kelfout_re = "$root/kelf_re";
  my $kelfout_rw = "$root/kelf_rw";
  sysopen (FL_KOUT_R, $kelfout_r, O_RDWR|O_CREAT, 0644);
  sysopen (FL_KOUT_RE, $kelfout_re, O_RDWR|O_CREAT, 0644);
  sysopen (FL_KOUT_RW, $kelfout_rw, O_RDWR|O_CREAT, 0644);

  print FL_OUT_R "0Name\t$label\n";
  print FL_OUT_RE "0Name\t$label\n";
  print FL_OUT_RW "0Name\t$label\n";
  print FL_KOUT_R "0Name\t$label\n";
  print FL_KOUT_RE "0Name\t$label\n";
  print FL_KOUT_RW "0Name\t$label\n";

  foreach my $pname (keys %mem_stat)
  {
    my $line; 

    if (basename($pname) eq "vmlinux")
    {
      $line = sprintf("%s\t%d\n", basename($pname), $mem_stat{$pname}{"R"});
      print FL_KOUT_R $line;

      $line = sprintf("%s\t%d\n", basename($pname), $mem_stat{$pname}{"RE"});
      print FL_KOUT_RE $line;

      $line = sprintf("%s\t%d\n", basename($pname), $mem_stat{$pname}{"RW"});
      print FL_KOUT_RW $line;
    }
    else
    {
      $line = sprintf("%s\t%d\n", $pname, $mem_stat{$pname}{"R"});
      print FL_OUT_R $line;

      $line = sprintf("%s\t%d\n", $pname, $mem_stat{$pname}{"RE"});
      print FL_OUT_RE $line;

      $line = sprintf("%s\t%d\n", $pname, $mem_stat{$pname}{"RW"});
      print FL_OUT_RW $line;
    }
  }

  close(FL_OUT_R);
  close(FL_OUT_RE);
  close(FL_OUT_RW);
  close(FL_KOUT_R);
  close(FL_KOUT_RE);
  close(FL_KOUT_RW);
  system("sort $elfout_r > $elfout_r.sort");
  system("mv $elfout_r.sort $elfout_r");
  system("sort $elfout_re > $elfout_re.sort");
  system("mv $elfout_re.sort $elfout_re");
  system("sort $elfout_rw > $elfout_rw.sort");
  system("mv $elfout_rw.sort $elfout_rw");
}

# output binary information
sub dump_readelfs
{
  my $sys_dir = $_[0];
  my $pathname;

  foreach my $node (@sys_tree)
  {
    $pathname = "$sys_dir/$node";
    if (-d $pathname)
    {
      opendir(DIR, $pathname);
      foreach my $file (readdir(DIR))
      {
        if ($file ne "." && $file ne "..")
        {
          parse_readelf("$pathname/$file");
        }
      }
      closedir(DIR);
    }
    else
    {
      warn "\"$node\" does NOT exist in system image!\n";
    }
  }

  # for kernel image
  parse_readelf("$sys_dir/../../../../../kernel/vmlinux");

  dump_readelf_results;
}

sub dump_img_size
{
  my $fh = new IO::File;
  my $tmpfile = "$root/img_size";

  foreach my $_img (@sys_imgs)
  {
    $_img =~ s/XXX/$project/;
    if (-f "$project_out_dir/$_img")
    {
      system("ls $project_out_dir/$_img | xargs du -abL >> $tmp_dir/.tmp.img");
    }
    else
    {
      print "\n= Rommon Fatal ERROR ====================\n";
      print "$project_out_dir/$_img not found!!\n";
      print "This build should be failed!! \n";
      print "DO NOT check in execel this time\n";
      print "=========================================\n\n";
    }
  }

  system("echo \"0Name\t$label\" > $tmpfile");
  die "$tmp_dir/.tmp.img open fails\n" unless $fh->open("$tmp_dir/.tmp.img");
  while ( <$fh> )
  {
    chomp;
    s/[\n,\r]//g;
    my ($val,$name) =
        /^\s*(\d+)\s+(\S+)\s*$/;
    $name =~ s/$project_out_dir//;
    system("echo \"$name\t$val\" >> $tmpfile");
  }
  system("sort $tmpfile > $tmpfile.sort");
  system("mv $tmpfile.sort $tmpfile");
}

#
# dump_dir_fsize
# traverse the directory, output each filesize to a file.
# if $_[1] is empty, then we just return the size
#
# inputs:
#  $_[0]: the directory to traverse
#  $_[1]: the profiling we wanna do
# return:
#  the total size in byte of all files in the directory
sub dump_dir_fsize
{
  my $fh = new IO::File;
  my @dirs = ($_[0]);
  my $tab_name = $_[1]; 
  my $tmpfile = "$root/$tab_name";
  my $total = 0;

  unless (-d $_[0])
  {
    print "In dump_dir_fsize(): $_[0] doesn't exist\n";
    return;
  }

  if ($tab_name ne "")
  {
    system("echo \"0Name\t$label\" > $tmpfile");
  }

  # traverse all directories
  foreach my $dir (@dirs)
  {
    opendir(DIR, $dir);
    if ($__debug) {print("in $dir:\n");}
    foreach my $file (readdir(DIR))
    {
      if ($file ne "." && $file ne "..")
      {
        if (-d "$dir/$file")
        {
          push(@dirs, "$dir/$file");
        }
        elsif (-f "$dir/$file")
        {
          # "du" will count additional 4096 bytes (. or ..) for each directory
          # therefore we count the size by ourselves.
          # system("du -ab $dir/$file >> .tmp.all");
          my $fsize = -s "$dir/$file";
          my $pathname = "$dir/$file";
          $pathname =~ s/$project_out_dir//; 
          if ($tab_name ne "")
          {
            system("echo \"$pathname\t$fsize\" >> $tmpfile");
          }
          $total = $total + $fsize;
        }
      }
    }
    closedir(DIR);
  }

  if ($tab_name ne "")
  {
    system("sort $tmpfile > $tmpfile.sort");
    system("mv $tmpfile.sort $tmpfile");
  }

  return $total;
}

sub dump_system_size
{
  dump_dir_fsize("$project_out_dir/system", "system_fsize");
}

sub dump_root_size
{
  dump_dir_fsize("$project_out_dir/root", "root_fsize");
}

sub dump_settings
{
  print "Settings:\n";
  print "* Iutput Directory: $project_out_dir\n";
  print "* Platform: $project\n";
  print "* Label: $label\n";
  print "* Cross Prefix: $cross_prefix\n";
  print "* Readelf: $readelf\n";
  print "* output database: $root/$database\n";
  print "*******************************\n";
}

#
# merge_new_csv
#
# merge two sorted csv files
#
sub merge_new_csv
{
  my $db_csv = $_[0];    # this file is for read
  my $db_csv_tmp = "$tmp_dir/.tmp.csv";  # then we write to this file for the final version
  my $db_new = $_[1];
  my $fh_csv = new IO::File;
  my $fh_csv_tmp = new IO::File;
  my $fh_new = new IO::File;
  my $old = "";
  my $new = "";

  # no existing database
  unless (-e $db_csv)
  {
    if (-e $db_new)
    {
      # just mv newly result to the old one
      system("mv $db_new $db_csv");
    }
    else
    {
      print "In merge_new_csv(): output csv file $_[1] is not found (profiling error?!)\n";
    }
    return;
  }

  die "$db_csv open fails\n" unless $fh_csv->open($db_csv);
  die "$db_csv_tmp open fails\n" unless $fh_csv_tmp->open("> $db_csv_tmp");
  die "$db_new open fails\n" unless $fh_new->open($db_new);

  $old = get_line($fh_csv);
  $new = get_line($fh_new);

  # detect the number of historical data
  my $line = $old;
  my $ver_num = $line =~ tr/\t//;
  if ($__debug)
  {
    print "We have $ver_num historical records in $db_csv\n";
  }

  # base on old file, find the same rws and new rows.
  while ($new)
  {
    my $line = "";
    my $new_name = get_name($new);

    while ($old)
    {
      my $old_name = get_name($old);
      #print $old_name.":".$new_name."\n";

      if ($old_name eq $new_name)
      {
        $line = $old_name . "\t" . get_value($old) . "\t" . get_value($new) . "\n";
        if ($__debug) {print "1:".$line;}
        last;
      }
      $old = get_line($fh_csv);
    }

    if (!$line)
    {   # no match! new item
      $line = get_name($new) . "\t" . gen_empty_record($ver_num) . "\t" . get_value($new) . "\n";
    }
    seek($fh_csv,0,0);
    $new = get_line($fh_new);
    $old = get_line($fh_csv);
    print $fh_csv_tmp $line;
  }

  seek($fh_csv,0,0);
  seek($fh_new,0,0);
  $old = get_line($fh_csv);
  $new = get_line($fh_new);

  #base on new files, find deleted rows
  while ($old)
  {
    my $line = "";
    my $same = 0;
    my $old_name = get_name($old);

    while ($new)
    {
      my $new_name = get_name($new);

      if ($old_name eq $new_name)
      {
        if ($__debug) {print "2: same".$new_name."\n";}
        $same = 1;
        last;
      }
      $new = get_line($fh_new);
    }

    if ($same == 0) 
    {
      $line = $old_name . "\t" . get_value($old) . "\t0\n";
      print $fh_csv_tmp $line;
    }

    seek($fh_new,0,0);
    $old = get_line($fh_csv);
    $new = get_line($fh_new);
  }

  $fh_csv_tmp->close();
  system("sort $db_csv_tmp > $db_csv_tmp.sort");
  system("mv $db_csv_tmp.sort $db_csv");
}

#
#==================================================================================================
## apk_extractor by AF1/Arnold
##==================================================================================================

sub apk_extractor
{
  my $cmd;
  my $apk_name;
  my $full_apk_name;
  my $res_folder;
  my $sum_total;
  my $sum_res;
  my $res_size;
  my $total_size;
  my @file_list;
  my @res_size;
  my @total_size;
  my $path = $_[0];
  my $tmpfile1 = "$root/apk_resource";
  my $tmpfile2 = "$root/apk_total";

  #print length($path);
  if (length($path)<2)
  {
    #print "null argument, use . as default!!!\n";
    $path = `pwd`;
  }

  $path =~ chomp($path);
  #print "processing directory : $path\n";

  $cmd = 'ls '.$path;
  @file_list = `$cmd`;

  if (@file_list < 1)
  {
    print "nothing found!!\n";
    return
  }
  $sum_total = 0;
  $sum_res =0;

  #open(F_CSV,'>>apk_report.csv');
  open(F_RES, ">", $tmpfile1);
  print F_RES "0Name\t$label\n";
  #print F_RES "APK_NAME,Resource Size [KB]\n";

  open(F_TOT, ">", $tmpfile2);
  print F_TOT "0Name\t$label\n";
  #print F_TOT "APK_NAME,Total Size [KB]\n";

  foreach $apk_name (@file_list)
  {
    $apk_name =~ chomp($apk_name);
    if ($apk_name =~ /\.apk$/)
    {
      $full_apk_name =$path."/".$apk_name;
      my $apk_tmp = "$tmp_dir/apk_temp";
      my $apk_tmp_res = "$apk_tmp/res";
      `mkdir $apk_tmp`;
      `unzip -d $apk_tmp $full_apk_name`;
      $res_folder = join($path,"$apk_tmp_res");
      if (-e $res_folder)
      {
        #$res_size = `du -s temp/res`;
        $res_size = dump_dir_fsize("$apk_tmp_res", "");
      }
      else
      {
        $res_size=0;
      }
      #$total_size = `du -s temp`;
      $total_size = dump_dir_fsize("$apk_tmp", "");
      #@res_size = split /\t/, $res_size;
      #@total_size = split /\t/, $total_size;
      #$sum_total = $sum_total + $total_size[0];
      #$sum_res = $sum_res + $res_size[0];
      #printf "%-30s: %d\/%d KB\n", $apk_name, $res_size[0], $total_size[0];
      #print F_RES $apk_name, "\t",$res_size[0], "\n";
      #print F_TOT $apk_name, "\t",$total_size[0], "\n";
      print F_RES $apk_name, "\t",$res_size, "\n";
      print F_TOT $apk_name, "\t",$total_size, "\n";
      `rm -rf $apk_tmp`;
    }
  }
  #print "All Apk Size   : $sum_total KB\n";
  #print "All Apk res Size : $sum_res KB\n";
  close(F_RES);
  close(F_TOT);
  system("sort $tmpfile1 > $tmpfile1.sort");
  system("mv $tmpfile1.sort $tmpfile1");
  system("sort $tmpfile2 > $tmpfile2.sort");
  system("mv $tmpfile2.sort $tmpfile2");
}

#
# split_excel()
#
# This function split the excel by making each worksheet as an csv file.
#
# input: the pathname of the excel to be splitted
#
sub split_excel
{
  my $excel = new Spreadsheet::ParseExcel;
  my $src_book = $excel->Parse($_[0]);

  if (!defined $src_book)
  {
    return;
  }

  for(my $iSheet=0; $iSheet < $src_book->{SheetCount} ; $iSheet++)
  {
    my $oWkS = $src_book->{Worksheet}[$iSheet];
    my $filename = "$root/$load_ver.$oWkS->{Name}.csv";

    sysopen (XLSFH, $filename, O_RDWR|O_CREAT, 0644);

    if ($__debug)
    {
      print "--------- SHEET:", $oWkS->{Name}, "\n";
    }

    for(my $iR = $oWkS->{MinRow}; defined $oWkS->{MaxRow} && $iR <= $oWkS->{MaxRow}; $iR++)
    {
      for(my $iC = $oWkS->{MinCol}; defined $oWkS->{MaxCol} && $iC <= $oWkS->{MaxCol}; $iC++)
      {
        if ($iC != $oWkS->{MinCol})
        {
          print XLSFH "\t";
        }
        my $oWkC = $oWkS->{Cells}[$iR][$iC];
        print XLSFH $oWkC->Value;
        print "( $iR , $iC ) =>", $oWkC->Value, "\n" if($__debug);
      }
      print XLSFH "\n";
    }
    close(XLSFH);
    system("sort $filename > $filename.sort");
    system("mv $filename.sort $filename");
  }
}

#
# merge_excel()
#
# this function merge the profiling results (*.csv) into an excel
# by making each csv file a excel tab
#
# input: the output file name
#
sub merge_excel
{
  my $src_book;

  $src_book = Spreadsheet::WriteExcel->new("$root/$_[0]");

  foreach my $profile (@sys_profiling)
  {
    my $wksheet;
    my $fh_csv = new IO::File;
    my $db_csv = "$root/$load_ver.$profile.csv";
    die "$db_csv open fails\n" unless $fh_csv->open($db_csv);

    $wksheet = $src_book->add_worksheet($profile);

    my $row = 0;
    my $col = 0;
    while (my $line = get_line($fh_csv))
    {
      my @values = split /\t/, $line;
      my $array_ref = \@values;
      if ($__debug) { print Dumper @values; }
      $wksheet->write_row($row++, 0, $array_ref);
    }
  }
  $src_book->close();
}

sub main
{   
#  if(defined $ARGV[0] && defined $ARGV[1] && defined $ARGV[2] && defined $ARGV[3])
  if ($#ARGV == 4)
  {
    $label = $ARGV[0];
    $project = $ARGV[1];
    $flavor = $ARGV[2];
    $database_dir = $ARGV[3];
    $project_out_dir = $ARGV[4];
  }
  elsif ($#ARGV == 3)
  {
    $label = $ARGV[0];
    $project = $ARGV[1];
    $database_dir = $ARGV[2];
    $project_out_dir = $ARGV[3];
  }
  else
  {
    print "Usage: rommon.pl <label> <project> [flavor] <database directory> <project out directory>\n";
    return;
  }

  unless (-d $database_dir) {
    warn("Input: $database_dir NOT exist & will create it!\n");
    system("mkdir -p $database_dir");
  }
  unless (-d $project_out_dir) {
    die("Input: $project_out_dir NOT exist!\n");
  }
  $load_ver = ($flavor ne "")? "$label.$project\[$flavor\]" : "$label.$project";
  $database = ($flavor ne "")? "ALPS_$project\[$flavor\].xls" : "ALPS_$project.xls";

  $root = ($flavor ne "")? $root."/".$label."/".$project."\[$flavor\]" : $root."/".$label."/".$project;
  $tmp_dir = $root . "/tmp";
  # build directory structure for store binary information
  if (-d $root)
  {
    system("rm -rf $root");
  }
  system("mkdir -p $root/tmp");

  dump_settings;

  if (-f "$database_dir/$database") {
    print "A database is located: $database_dir/$database\n";
    print "Backup $database_dir/$database to $database_dir/$database.bak\n";
    `cp $database_dir/$database $database_dir/$database.bak`;
    split_excel("$database_dir/$database");
  } else {
    print "Src excel is not found\nA new excel file will be created in $root\n";
  }

  dump_img_size;
  merge_new_csv("$root/$load_ver.img_size.csv", "$root/img_size");

  dump_system_size;
  merge_new_csv("$root/$load_ver.system_fsize.csv", "$root/system_fsize");

  dump_root_size;
  merge_new_csv("$root/$load_ver.root_fsize.csv", "$root/root_fsize");

  dump_readelfs("$project_out_dir/system");
  merge_new_csv("$root/$load_ver.elf_r.csv", "$root/elf_r");
  merge_new_csv("$root/$load_ver.elf_re.csv", "$root/elf_re");
  merge_new_csv("$root/$load_ver.elf_rw.csv", "$root/elf_rw");
  merge_new_csv("$root/$load_ver.kelf_r.csv", "$root/kelf_r");
  merge_new_csv("$root/$load_ver.kelf_re.csv", "$root/kelf_re");
  merge_new_csv("$root/$load_ver.kelf_rw.csv", "$root/kelf_rw");

  apk_extractor("$project_out_dir/system/app");
  merge_new_csv("$root/$load_ver.apk_resource.csv", "$root/apk_resource");
  merge_new_csv("$root/$load_ver.apk_total.csv", "$root/apk_total");

  merge_excel($database);

  print "Sync new database $root/$database back to $database_dir/$database\n\n";
  `cp $root/$database $database_dir/$database`;
  if (!$__debug)
  {
    # remove folder which name contains special characters
    system("rm -rf rommon_results");
  }
}
main;

