#!/usr/bin/perl
($#ARGV != 1) && &Usage;
($prj, $platform) = @ARGV;

(exit 0) if ($prj eq "generic");
$logDir = "out/target/product/$prj";
#die "Can NOT find $logDir" if (!-d $logDir);
(exit 0) if (!-d $logDir);

#print "plat = $platform\n";
my @lnmatrix = ();
   @lnmatrix = (
  "mediatek/source/preloader/preloader_${prj}.bin",
  "mediatek/source/misc/${platform}_Android_scatter.txt",
  "bootable/bootloader/uboot/uboot_${prj}.bin",
  "bootable/bootloader/uboot/logo.bin",
  "kernel/kernel_${prj}.bin",
  );
 
chdir($logDir);
$relDir = $logDir;
$relDir =~ s/[^\/]+/../g;

foreach $i (@lnmatrix) {
  $lnfile = "${relDir}/$i";
  $i =~ /([^\/]+)$/;
  $j = $1;
  if ($j =~ /kernel\.bin/) {
    $j = "kernel.bin";
  }
  system("rm $j") if (-e $j);
  if (!-e $lnfile) {
     print("$lnfile does NOT exist!\n");
     next;
  }
  if ($lnfile =~ /kernel\.bin/) {
    system("ln -s $lnfile kernel.bin");
  } else {
    system("ln -s $lnfile .");
  }
}

exit 0;

sub Usage {
  warn << "__END_OF_USAGE";
Usage: $myCmd project
__END_OF_USAGE
  exit 1;
}

