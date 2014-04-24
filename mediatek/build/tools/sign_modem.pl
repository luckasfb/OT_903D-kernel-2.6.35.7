#!/usr/bin/perl

##########################################################
# Initialize Variables
##########################################################
my $prj = $ARGV[0];
my $modem_encode = $ARGV[1];
my $modem_auth = $ARGV[2];
my $cfg_dir = $ARGV[3];
my $sml_dir = "mediatek/custom/$cfg_dir/security/sml_auth";
my $cipher_tool = "mediatek/build/tools/CipherTool/CipherTool";
my $sign_tool = "mediatek/build/tools/SignTool/SignTool.sh";

##########################################################
# Check Parameter
##########################################################
print "\n\n********************************************\n";
print " CHECK PARAMETER \n";
print "********************************************\n";

if (${modem_auth} eq "yes")
{
	if (${modem_encode} eq "no")
	{
		die "Error! MTK_SEC_MODEM_AUTH is 'yes' but MTK_SEC_MODEM_ENCODE is 'no'\n";
	}
}

print "parameter check pass\n";
print "MTK_SEC_MODEM_AUTH    =  $modem_auth\n";
print "MTK_SEC_MODEM_ENCODE  =  $modem_encode\n";

##########################################################
# Process Modem Image
##########################################################
print "\n\n********************************************\n";
print " PROCESS MODEM IMAGE \n";
print "********************************************\n";

my $md_load = "mediatek/custom/out/$prj/modem/modem.img";
my $backup_md_load = "mediatek/custom/out/$prj/modem/modem.img.bak";
my $cipher_md_load = "mediatek/custom/out/$prj/modem/cipher_modem.img";
my $signed_md_load = "mediatek/custom/out/$prj/modem/signed_modem.img";


if (-e "$backup_md_load")
{
	print "already processed ... \n";
}
else
{
	if (-e "$md_load")
	{
		system("cp -f $md_load $backup_md_load") == 0 or die "can't backup modem image";

		########################################		
		# Encrypt and Sign Modem Image
		########################################		
		if (${modem_encode} eq "yes")
		{
			system("./$cipher_tool ENC $sml_dir/SML_ENCODE_KEY.ini $sml_dir/SML_ENCODE_CFG.ini $md_load $cipher_md_load") == 0 or die "Cipher Tool return error\n";
			
			if(-e "$cipher_md_load")
			{
				system("rm -f $md_load") == 0 or die "can't remove original modem binary\n";
				system("mv -f $cipher_md_load $md_load") == 0 or die "can't generate cipher modem binary\n";
			}
			
			system("./$sign_tool $sml_dir/SML_AUTH_KEY.ini $sml_dir/SML_AUTH_CFG.ini $md_load $signed_md_load");

			if(-e "$signed_md_load")
			{
				system("rm -f $md_load") == 0 or die "can't remove original modem binary\n";
				system("mv -f $signed_md_load $md_load") == 0 or die "can't generate signed modem binary\n";
			}			
		}
		else
		{
			print "doesn't execute Cipher Tool and Sign Tool ... \n";
		}		
	}
}

##########################################################
# Process Modem_E2 Image
##########################################################
print "\n\n********************************************\n";
print " PROCESS MODEM_E2 IMAGE \n";
print "********************************************\n";

$md_load = "mediatek/custom/out/$prj/modem/modem_E2.img";
$backup_md_load = "mediatek/custom/out/$prj/modem/modem_E2.img.bak";
$cipher_md_load = "mediatek/custom/out/$prj/modem/cipher_modem_E2.img";
$signed_md_load = "mediatek/custom/out/$prj/modem/signed_modem_E2.img";


if (-e "$backup_md_load")
{
	print "already processed ... \n";
}
else
{
	if (-e "$md_load")
	{
	
		system("cp -f $md_load $backup_md_load") == 0 or die "can't backup modem image";

		########################################		
		# Encrypt and Sign Modem E2 Image
		########################################
		if (${modem_encode} eq "yes")
		{
			system("./$cipher_tool ENC $sml_dir/SML_ENCODE_KEY.ini $sml_dir/SML_ENCODE_CFG.ini $md_load $cipher_md_load") == 0 or die "Cipher Tool return error\n";

			if(-e "$cipher_md_load")
			{
				system("rm -f $md_load") == 0 or die "can't remove original modem binary";
				system("mv -f $cipher_md_load $md_load") == 0 or die "can't generate cipher modem binary";
			}

			system("./$sign_tool $sml_dir/SML_AUTH_KEY.ini $sml_dir/SML_AUTH_CFG.ini $md_load $signed_md_load");

			if(-e "$signed_md_load")
			{
				system("rm -f $md_load") == 0 or die "can't remove original modem binary\n";
				system("mv -f $signed_md_load $md_load") == 0 or die "can't generate signed modem binary\n";
			}
		}
		else
		{
			print "doesn't execute Cipher Tool and Sign Tool ... \n";
		}		
		
	}
}

##########################################################
# Process SDS.bin
##########################################################
print "\n\n********************************************\n";
print " PROCESS SML DEFAULT SETTING (SDS.bin) \n";
print "********************************************\n";

$sml_cfg_load = "$sml_dir/SDS.bin";
$cipher_sml_load = "mediatek/custom/$cfg_dir/secro/C_SDS.bin";
$signed_sml_load = "mediatek/custom/$cfg_dir/secro/S_SDS.bin";

if (-e "$sml_cfg_load")
{		

	########################################		
	# Encrypt and Sign SML Default Setting Binary
	########################################
	if (${modem_encode} eq "yes")
	{
		system("./$cipher_tool ENC $sml_dir/SML_ENCODE_KEY.ini $sml_dir/SDS_ENCODE_CFG.ini $sml_cfg_load $cipher_sml_load") == 0 or die "Cipher Tool return error\n";
		system("./$sign_tool $sml_dir/SML_AUTH_KEY.ini $sml_dir/SDS_AUTH_CFG.ini $cipher_sml_load $signed_sml_load");

		if(-e "$signed_sml_load")
		{
			system("rm -f $cipher_sml_load") == 0 or die "can't remove temp SML CFG binary\n";			
		}
	}
	else
	{
		print "doesn't execute Cipher Tool and Sign Tool ... \n";
	}

}

##########################################################
# Process SECFL.ini
##########################################################
print "\n\n********************************************\n";
print " PROCESS SECFL.ini \n";
print "********************************************\n";

my $secfl_perl = "mediatek/build/tools/sign_sec_file_list.pl";
system("./$secfl_perl $cfg_dir") == 0 or die "SECFL Perl return error\n";