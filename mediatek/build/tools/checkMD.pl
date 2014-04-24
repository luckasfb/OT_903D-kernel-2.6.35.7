#!/usr/bin/perl

usage() if ($#ARGV < 1);

my ($DSP_BL, $DSP_ROM, $MTK_MODEM_SUPPORT) = @ARGV;

my $position = 0x16;
my $whence = 0;

my $buffer;
my $length = 4;

my $mode;
my $temp;
my $DSP_BL_MODE;
my $DSP_BL_FILE_TYPE;
my $DSP_ROM_MODE;
my $DSP_ROM_FILE_TYPE;

my $errCnt = 0;

if (defined $MTK_MODEM_SUPPORT)
{
  # get current used modem mode
  if ($MTK_MODEM_SUPPORT eq "modem_3g")
  {
    $mode = 0;
  }
  elsif ($MTK_MODEM_SUPPORT eq "modem_2g")
  {
    $mode = 1;
  }
  else
  {
    $mode = -1;
  }
}

# check if DSP_BL & DSP_ROM exist
die "[MODEM CHECK FAILED]: The file \"$DSP_BL\" does NOT exist!\n" if (!-e $DSP_BL);
die "[MODEM CHECK FAILED]: The file \"$DSP_ROM\" does NOT exist!\n" if (!-e $DSP_ROM);

# read modem mode, debug/release flag & file type
open(BL, "<$DSP_BL") or die "Can NOT open file $DSP_BL"; 
binmode(BL);
seek(BL, $position, $whence) or die "Can NOT seek to the position $position in \"$DSP_BL\"!\n";
read(BL, $buffer, $length) == $length or die "Failed to read the file \"$DSP_BL\"!\n";
($DSP_BL_MODE, $temp, $DSP_BL_FILE_TYPE) = unpack("c2S", $buffer);
close(BL);

open(ROM, "<$DSP_ROM") or die "Can NOT open file \"$DSP_ROM\"!\n";
binmode(ROM);
seek(ROM, $position, $whence) or die "Can NOT seek to the position $position in \"$DSP_ROM\"!\n";
read(ROM, $buffer, $length) == $length or die "Failed to read the file $DSP_ROM";
($DSP_ROM_MODE, $temp, $DSP_ROM_FILE_TYPE) = unpack("c2S", $buffer);
close(ROM);

# rules
# check the file type of DSP_BL & DSP_ROM
# 0x0003 stands for DSP_BL
# 0x0104 stands for DSP_ROM
print "[MODEM CHECK INFO]: DSP bootloader ==> 0x0003, DSP ROM ==> 0x0104\n";

if ($DSP_BL_FILE_TYPE != 0x0003)
{
  $errCnt++;
  warn "[MODEM CHECK FAILED]: Wrong file type, NOT DSP bootloader!\n"
     . "[MODEM CHECK FAILED]: \"$DSP_BL\" ==> " . sprintf("%s%04X", "0x", $DSP_BL_FILE_TYPE) ."\n";
}

if ($DSP_ROM_FILE_TYPE != 0x0104)
{
  $errCnt++;
  warn "[MODEM CHECK FAILED]: Wrong file type, NOT DSP ROM!\n"
     . "[MODEM CHECK FAILED]: \"$DSP_ROM\" ==> " . sprintf("%s%04X", "0x", $DSP_ROM_FILE_TYPE) . "\n";
}

die "[MODEM CHECK FAILED]: Please check the modem file type!\n" if ($errCnt);

#
# check the file consistency of DSP_BL & DSP_ROM
#
# 2G/3G
# Todo: unknown scenario...
if (($DSP_BL_MODE & 0x01) != ($DSP_ROM_MODE & 0x01))
{
  $errCnt++;
  warn "[MODEM CHECK FAILED]: Inconsistent mode of \"$DSP_BL\" & \"$DSP_ROM\"!\n"
     . "[MODEM CHECK FAILED]: \"$DSP_BL\" ==> " . ((($DSP_BL_MODE & 0x01) == 0) ? "3G" : "2G") . "\n"
     . "[MODEM CHECK FAILED]: \"$DSP_ROM\" ==> " . (( ($DSP_ROM_MODE & 0x01) == 0) ? "3G" : "2G") . "\n";
}

# debug/release
# Todo: unknown scenario...
if (($DSP_BL_MODE & 0x02) != ($DSP_ROM_MODE & 0x02))
{
  $errCnt++;
  warn "[MODEM CHECK FAILED]: Inconsistent version type of \"$DSP_BL\" & \"$DSP_ROM\"!\n"
     . "[MODEM CHECK FAILED]: \"$DSP_BL\" ==> " . ((($DSP_BL_MODE & 0x02) == 0) ? "release" : "debug") . "\n"
     . "[MODEM CHECK FAILED]: \"$DSP_ROM\" ==> " . (( ($DSP_ROM_MODE & 0x02) == 0) ? "release" : "debug") . "\n";
}

#
# check if current project use the wrong modem
#
# Todo: unknown scenario...
if (defined $MTK_MODEM_SUPPORT)
{
  if (($DSP_BL_MODE & 0x01) != $mode)
  {
    $errCnt++;
    warn "[MODEM CHECK FAILED]: Inconsistency between \"Project Configuration\" & \"$DSP_BL\"!\n"
       . "[MODEM CHECK FAILED]: \"Project Configuration\" ==> " . $MTK_MODEM_SUPPORT . "\n"
       . "[MODEM CHECK FAILED]: \"$DSP_BL\" ==> " . ((($DSP_BL_MODE & 0x01) == 0) ? "3G" : "2G") . "\n";
  }

  if (($DSP_ROM_MODE & 0x01) != $mode)
  {
    $errCnt++;
    warn "[MODEM CHECK FAILED]: Inconsistency between \"Project Configuration\" & \"$DSP_ROM\"!\n"
       . "[MODEM CHECK FAILED]: \"Project Configuration\" ==> " . $MTK_MODEM_SUPPORT . "\n"
       . "[MODEM CHECK FAILED]: \"$DSP_ROM\" ==> " . ((($DSP_ROM_MODE & 0x01) == 0) ? "3G" : "2G") . "\n";
  }
}

die "[MODEM CHECK FAILED]: Please check the modem images inconsistency!\n" if ($errCnt);

print "[MODEM CHECK INFO]: Check modem image DONE!\n";
exit 0;

sub usage
{
  print <<"__EOFUSAGE";

Usage:
$0  DSP_BL DSP_ROM [MTK_MODEM_SUPPORT]

DSP_BL               DSP bootloader image
DSP_ROM              DSP rom image
MTK_MODEM_SUPPORT    current project used modem mode
                     (ex. modem_3g, modem_2g)

__EOFUSAGE
  exit 1;
}

