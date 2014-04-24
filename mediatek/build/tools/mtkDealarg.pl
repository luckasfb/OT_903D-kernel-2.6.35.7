#!/usr/bin/perl
($addP, $arCmd, $gArF, $lArF, $arlib, $obj) = @ARGV;
print ":" . join(' ', @ARGV) . "\n";
@objs = split(' ', $obj);
foreach $f (@objs) {
  push(@reviseObjs, "$addP/$f");
}
if ($#objs < 900) {
  $cmd = "$arCmd $gArF $lArF $arlib " . join(' ',@reviseObjs);
  $rslt = system "$cmd";
  print "$cmd:$rslt\n";
  exit $rslt;
} else {
  $addP2 = $addP;
  $addP2 =~ s/\//\\\//g;
  chdir($addP);
  $arlib =~ s/^${addP2}\///;
  $cmd = "..\/..\/$arCmd $gArF $lArF $arlib " . join(' ',@objs);
    $rslt = system "$cmd";
  chdir("..\/..");
  #chdir("..");
#  $i = 0;
#  $j = 599;
#  while ($j < $#objs) {
#    $cmd = "$arCmd $gArF $lArF $arlib " . join(' ',@reviseObjs[$i..$j]);
#    $rslt = system "$cmd";
#    print "$cmd:$rslt\n";
#    die "Failed to $cmd\n" if ($rslt != 0);
  #$i = $j + 1;
#    $j += 600;
#  }
#  #$i = $j + 1;
#  $j = $#objs;
#  $cmd = "$arCmd $gArF $lArF $arlib " . join(' ',@reviseObjs[$i..$j]);
#  $rslt = system "$cmd";
#  print "$cmd:$rslt\n";
#  die "Failed to $cmd\n" if ($rslt != 0);
}
#print "$cmd\n";

#$rslt = system "$cmd";

exit $rslt;

