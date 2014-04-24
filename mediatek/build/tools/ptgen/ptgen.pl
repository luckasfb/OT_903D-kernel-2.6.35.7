#!/usr/local/bin/perl
use File::Basename;

# arrays save excel field values, now, we are using only 4 columns, Partition, Start, SizeKB, and DL.

my $PARTITION_FIELD ;
my $START_FIELD ;
my $SIZEKB_FIELD ;
my $DL_FIELD ;

# Okey, that are the arrays.

my $total_rows = 0 ;


my $os = &OsName();

my $LOCAL_PATH;

BEGIN
{
  $LOCAL_PATH = dirname($0);
}

if ($os eq "linux")
{
   print "Os = linux\n";

   use lib "$LOCAL_PATH/../Spreadsheet";
   use lib "$LOCAL_PATH/../";
   require 'ParseExcel.pm';
   $start_num = 0; 
}
else
{
  die "Only linux is support now!\n";
}

my $DebugPrint    = 1; # 1 for debug; 0 for non-debug

# define for columns
my $COLUMN_PARTITION                = 1 ;
my $COLUMN_TYPE                     = $COLUMN_PARTITION + 1 ;
my $COLUMN_START                    = $COLUMN_TYPE + 1 ;
my $COLUMN_END                      = $COLUMN_START + 1 ;
my $COLUMN_SIZE                     = $COLUMN_END + 1 ;
my $COLUMN_SIZEKB                   = $COLUMN_SIZE + 1 ;
my $COLUMN_SIZE2                    = $COLUMN_SIZEKB + 1 ;
my $COLUMN_SIZE3                    = $COLUMN_SIZE2 + 1 ;
my $COLUMN_DL                       = $COLUMN_SIZE3 + 1 ;


my $PLATFORM                    = $ARGV[0] ; # MTxxxx
my $LCA_PRJ                     = $ARGV[1] ; # weather is it a LCA project, "yes" or "no"
my $XLS_FILENAME                = $ARGV[2] ; # excel file name
my $PARTITION_DEFINE_H_NAME     = $ARGV[3] ; # 
my $SCAT_NAME                   = $ARGV[4] ; # 
my $PAGE_SIZE                   = $ARGV[5] ; #


print "\nargument:\n" ;
print "PLATFORM=$PLATFORM \n" ;
print "LCA_PRJ=$LCA_PRJ \n" ;
print "XLS_FILENAME=$XLS_FILENAME \n" ;
print "SCAT_NAME=$SCAT_NAME \n" ;
print "PARTITION_DEFINE_H_NAME=$PARTITION_DEFINE_H_NAME \n" ;


mkdir($SCAT_NAME) if (!-d $SCAT_NAME);


#out putfile name
if ($LCA_PRJ eq "yes")
{
    $SCAT_NAME = $SCAT_NAME . "/" . $PLATFORM ."_Android_scatter_LCA.txt" ;
}
else
{
    $SCAT_NAME = $SCAT_NAME ."/" .  $PLATFORM ."_Android_scatter.txt" ;
}
$PARTITION_DEFINE_H_NAME = $PARTITION_DEFINE_H_NAME . "/" . "partition_define.h" ;

print "\noutput:\n" ;
print "SCAT_NAME=$SCAT_NAME \n" ;
print "PARTITION_DEFINE_H_NAME=$PARTITION_DEFINE_H_NAME \n" ;

my $SHEET_NAME = $PLATFORM ." " . $LCA_PRJ ;
if ($PAGE_SIZE eq "4K")
{
    $SHEET_NAME = $SHEET_NAME . " " . $PAGE_SIZE
}

print "SHEET_NAME=$SHEET_NAME \n" ;

#****************************************************************************
# read excel
#****************************************************************************
# get already active Excel application or open new
my $parser = Spreadsheet::ParseExcel->new();
$Book = $parser->Parse($XLS_FILENAME); 

&ReadExcelFile () ;

&GenHeaderFile () ;

&GenScatFile () ;

print "scatgen done. \n" ;

exit ;

sub GenHeaderFile ()
{
    my $iter = 0 ;
    my $temp ;
    open (PARTITION_DEFINE_H_NAME, ">$PARTITION_DEFINE_H_NAME") or &error_handler("PARTITION_DEFINE: file error!", __FILE__, __LINE__);
    print PARTITION_DEFINE_H_NAME &copyright_file_header();

    print PARTITION_DEFINE_H_NAME "\n#ifndef __PARTITION_DEFINE_H__\n#define __PARTITION_DEFINE_H__\n\n" ;
    
    print PARTITION_DEFINE_H_NAME "\n\n\n#define KB  (1024)\n#define MB  (1024 * KB)\n#define GB  (1024 * MB)\n\n" ;

        
    for ($iter=0; $iter<$total_rows; $iter++)
    {
        if($PARTITION_FIELD[$iter] eq "BMTPOOL")
        {
					$temp = "#define PART_SIZE_$PARTITION_FIELD[$iter]\t\t\t(0x$SIZEKB_FIELD[$iter])\n" ;
    			print PARTITION_DEFINE_H_NAME $temp ;
        }
        else
        {
    			$temp = "#define PART_SIZE_$PARTITION_FIELD[$iter]\t\t\t($SIZEKB_FIELD[$iter]*KB)\n" ;
					print PARTITION_DEFINE_H_NAME $temp ;
        }
        
    }
    
    
    
    print PARTITION_DEFINE_H_NAME "\n\n" ;

    print PARTITION_DEFINE_H_NAME "#endif\n" ;
    
    close PARTITION_DEFINE_H_NAME ;
}

sub GenScatFile ()
{
    my $iter = 0 ;
    open (SCAT_NAME, ">$SCAT_NAME") or &error_handler("SCAT: file error!", __FILE__, __LINE__) ;

    for ($iter=0; $iter<$total_rows; $iter++)
    {   

        if($PARTITION_FIELD[$iter] eq "CUSTPACK"){
            $PARTITION_FIELD[$iter] = "CUSTPACK1" ;
	}
	
        if($PARTITION_FIELD[$iter] eq "BMTPOOL"){
		

	}else{
		$START_FIELD[$iter]=sprintf("%x",$START_FIELD[$iter]);
	}

        if ($DL_FIELD[$iter] == 1)
        {
            $temp = "$PARTITION_FIELD[$iter] 0x$START_FIELD[$iter]\n{\n}\n" ;
        }
        else
        {
            $temp = "__NODL_$PARTITION_FIELD[$iter] 0x$START_FIELD[$iter]\n{\n}\n" ;
        }
        print SCAT_NAME $temp ;
    }
    
    print SCAT_NAME "\n\n" ;
    close SCAT_NAME ;
}

#****************************************************************************
# subroutine:  error_handler
# input:       $error_msg:     error message
#****************************************************************************
sub error_handler
{
	   my ($error_msg, $file, $line_no) = @_;
	   
	   my $final_error_msg = "scatgen ERROR: $error_msg at $file line $line_no\n";
	   print $final_error_msg;
	   die $final_error_msg;
}


#****************************************************************************
# subroutine:  ReadExcelFile
# return:      
#****************************************************************************

sub ReadExcelFile
{
    my $sheet;
    my $read       = 1; # if this flag counts to '0', it means End Of Sheet
    
    my $row = 1 ;
    my $size_B = 0;
    my $size_B_p;
    my $temp;

    
    $sheet = get_sheet($SHEET_NAME) ;
    
    if ($sheet eq undef)
    {
        print "get_sheet failed? $SHEET_NAME\n" ;
    }
    
    while ($read)
    {
        $PARTITION_FIELD[$row-1] = &xls_cell_value($sheet, $row, $COLUMN_PARTITION) ;
        if ($PARTITION_FIELD[$row-1] eq "END")
        {
#            print "meet END.\n";
            $read = 0 ;
        }
        if ($read)
        {    
             $size_B_p = $size_B;
             $SIZEKB_FIELD[$row-1]    = &xls_cell_value($sheet, $row, $COLUMN_SIZEKB) ;
	     $size_B= $SIZEKB_FIELD[$row-1] * 1024;
	     #print "size_B= $size_B ";
	     
	  if($row-1 == 0)
             {
	        $START_FIELD[$row-1]  =  0;
             }else{
			if($PARTITION_FIELD[$row-1] eq "BMTPOOL"){
				$START_FIELD[$row-1] = &xls_cell_value($sheet, $row, $COLUMN_START) ;				
			}else{
		                $START_FIELD[$row-1] =  $START_FIELD[$row-2]  + $size_B_p;
			}
             }
            
            $DL_FIELD[$row-1]        = &xls_cell_value($sheet, $row, $COLUMN_DL) ;
            
            if( $PARTITION_FIELD[$row-1] eq undef   || 
                $START_FIELD[$row-1] eq undef       || 
                $SIZEKB_FIELD[$row-1] eq undef      || 
                $DL_FIELD[$row-1] eq undef
              )
            {
                if( $PARTITION_FIELD[$row-1] eq undef   && 
                    $START_FIELD[$row-1] eq undef       && 
                    $SIZEKB_FIELD[$row-1] eq undef      && 
                    $DL_FIELD[$row-1] eq undef
                   )
                {
                    $read = 0 ;
                }
                die "error in excel file row $row!" ;
            }
	   #debug
           $temp=sprintf("%x",$START_FIELD[$row-1]);
           print "PARTITION_FIELD=$PARTITION_FIELD[$row-1],SIZEKB=$SIZEKB_FIELD[$row-1],START_FIELD=$START_FIELD[$row-1]/$temp,DL_FIELD=$DL_FIELD[$row-1]\n" ;
           #debug
            $row ++ ;
        }
    }
    
    if ($row == 1)
    {
        die "error in excel file no data!\n" ;
    }
    
    $total_rows = $row - 1 ;
    
    print "$total_rows read.\n" ;
}

#****************************************************************************
# subroutine:  copyright_file_header
# return:      file header -- copyright
#****************************************************************************
sub copyright_file_header
{
    my $template = <<"__TEMPLATE";
__TEMPLATE

   return $template;
}

#****************************************************************************************
# subroutine:  OsName
# return:      which os this script is running
# input:       no input
#****************************************************************************************
sub OsName {
  my $os = `set os`;
  if(!defined $os) { 
    $os = "linux";
  } 
  else {
    die "does not support windows now!!" ;
    $os = "windows";
  }
}
#*************************************************************************************************
# subroutine:  gen_pm
# return:      no return, but will generate a ForWindows.pm in "/perl/lib" where your perl install
#*************************************************************************************************
sub gen_pm {
  foreach (@INC) {
    if(/^.*:\/Perl\/lib$/) {
      open FILE, ">${_}\/ForWindows.pm";
      print FILE "package ForWindows;\n";
      print FILE "use Win32::OLE qw(in with);\n";
      print FILE "use Win32::OLE::Const 'Microsoft Excel';\n";
      print FILE "\$Win32::OLE::Warn = 3;\n";
      print FILE "1;";
      close(FILE);
      last;
    }
  }
}
#****************************************************************************************
# subroutine:  get_sheet
# return:      Excel worksheet no matter it's in merge area or not, and in windows or not
# input:       Specified Excel Sheetname
#****************************************************************************************
sub get_sheet {
  my $MEMORY_DEVICE_TYPE = $_[0];
  if ($os eq "windows") {
    return $Sheet     = $Book->Worksheets($MEMORY_DEVICE_TYPE);
  }
  else {
    return $Sheet     = $Book->Worksheet($MEMORY_DEVICE_TYPE);
  }
}


#****************************************************************************************
# subroutine:  xls_cell_value
# return:      Excel cell value no matter it's in merge area or not, and in windows or not
# input:       $Sheet:  Specified Excel Sheet
# input:       $row:    Specified row number
# input:       $col:    Specified column number
#****************************************************************************************
sub xls_cell_value {
  my ($Sheet, $row, $col) = @_;
  if ($os eq "windows") {
    return &win_xls_cell_value($Sheet, $row, $col);
  }
  else {
      return &lin_xls_cell_value($Sheet, $row, $col);
  }
}
sub win_xls_cell_value
{
    my ($Sheet, $row, $col) = @_;

    if ($Sheet->Cells($row, $col)->{'MergeCells'})
    {
        my $ma = $Sheet->Cells($row, $col)->{'MergeArea'};
        return ($ma->Cells(1, 1)->{'Value'});
    }
    else
    {
        return ($Sheet->Cells($row, $col)->{'Value'})
    }
}
sub lin_xls_cell_value
{
  my ($Sheet, $row, $col) = @_;
  my $cell = $Sheet->get_cell($row, $col);
  exit 1 unless (defined $cell);
  my $value = $cell->Value();

}

