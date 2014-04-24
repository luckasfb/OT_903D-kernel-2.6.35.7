#!/usr/bin/perl
($#ARGV != 0) && &Usage;
($prj) = @ARGV;

($prj = "generic") if ($prj eq "emulator");

$flag_subrel = "mediatek/build/android/full/config.mk";
$flag_custrel = "mediatek/build/android/full/config.mk.custrel";
$srcDir = "vendor/mediatek/$prj/artifacts/out/";
$dstDir = "out/";
$tmpDir = "vendor/mediatek/$prj/artifacts/kernel/mediatek/";

exit 0, if (!-e $flag_subrel && !-e $flag_custrel);
exit 0, if (-e $flag_subrel && -e $flag_custrel);

if (!-e $flag_subrel)
#if (0)
{
	if (-d $srcDir)
	{
	  system("rsync -av --exclude=.svn --exclude=.git --exclude=.cvs $srcDir $dstDir > auto_sync_android.log 2>&1");
	}
	exit 0;
}

if (!-e $flag_custrel)
#if (!-e $flag_subrel)
{
	if ($ENV{"MTK_SIGNATURE_CUSTOMIZATION"} eq "yes")
	{
	  my $binaryAppPath = $srcDir . "/target/product/$prj/system/app/";
	  my @applist = <$binaryAppPath/*.apk>;
	  foreach my $app (@applist)
	  {
	    print "Signing using customerization signature for $app \n";
	    &signApk($app);
	  }
	}
	if (-d $srcDir)
	{
	  system("rsync -av --exclude=.svn --exclude=.git --exclude=.cvs $srcDir $dstDir > auto_sync_android.log 2>&1");
	} 
	if (-d $tmpDir)
	{
	  system("rsync -av --exclude=.svn --exclude=.git --exclude=.cvs $tmpDir mediatek/ > auto_sync_kernel.log 2>&1\n");
	}
}

exit 0;

sub Usage {
  warn << "__END_OF_USAGE";
Usage: $myCmd project
__END_OF_USAGE
  exit 1;
}

sub signApk {
  my ($src) = @_;
  my $src_tmp = $src . ".bak";
  my $signTool = $srcDir . "/host/linux-x86/framework/signapk.jar";
  my $key1 = "build/target/product/security/$prj/platform.x509.pem";
  my $key2 = "build/target/product/security/$prj/platform.pk8";
  system ("java -jar $signTool $key1 $key2 $src $src_tmp");
  system ("mv $src_tmp $src");
}
